#include <stdio.h>

#include <stdexcept>

#include <locic/AST.hpp>
#include <locic/Support/Map.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/AliasTypeResolver.hpp>
#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>
#include <locic/SemanticAnalysis/Template.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		static const SEM::Type* ConvertIntegerType(Context& context, AST::TypeDecl::SignedModifier signedModifier,
		                                           const String& nameString) {
			// Unsigned types have 'u' prefix and all integer types
			// have '_t' suffix (e.g. uint_t, short_t etc.).
			const auto fullNameString = (signedModifier == AST::TypeDecl::UNSIGNED) ? (context.getCString("u") + nameString + "_t") : (nameString + "_t");
			return getBuiltInType(context, fullNameString, {});
		}
		
		static const SEM::Type* ConvertFloatType(Context& context, const String& nameString) {
			// All floating point types have '_t' suffix (e.g. float_t, double_t etc.).
			const auto fullNameString = nameString + "_t";
			return getBuiltInType(context, fullNameString, {});
		}
		
		class UnknownTypeNameDiag: public Error {
		public:
			UnknownTypeNameDiag(Name name)
			: name_(std::move(name)) { }
			
			std::string toString() const {
				return makeString("unknown type name '%s'",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		const SEM::Type* ConvertObjectType(Context& context, const AST::Node<AST::Symbol>& symbol) {
			assert(!symbol->empty());
			
			const Name name = symbol->createName();
			
			const auto searchResult = performSearch(context, name);
			
			const auto templateVarMap = GenerateSymbolTemplateVarMap(context, symbol);
			
			if (searchResult.isTypeInstance()) {
				auto& typeInstance = searchResult.typeInstance();
				
				assert(templateVarMap.size() == typeInstance.templateVariables().size());
				
				return SEM::Type::Object(&typeInstance, GetTemplateValues(templateVarMap, typeInstance.templateVariables()));
			} else if (searchResult.isTemplateVar()) {
				assert(templateVarMap.empty());
				
				return SEM::Type::TemplateVarRef(&(searchResult.templateVar()));
			} else if (searchResult.isAlias()) {
				auto& alias = searchResult.alias();
				
				assert(templateVarMap.size() == alias.templateVariables().size());
				
				auto templateValues = GetTemplateValues(templateVarMap, alias.templateVariables());
				assert(templateValues.size() == alias.templateVariables().size());
				
				return SEM::Type::Alias(alias, std::move(templateValues));
			} else {
				context.issueDiag(UnknownTypeNameDiag(name.copy()),
				                  symbol.location());
				return context.typeBuilder().getIntType();
			}
		}
		
		class FunctionTypeParameterCannotBeVoidDiag: public Error {
		public:
			FunctionTypeParameterCannotBeVoidDiag() { }
			
			std::string toString() const {
				return "parameter type in function pointer type cannot be void";
			}
			
		};
		
		const SEM::Type* ConvertType(Context& context, const AST::Node<AST::TypeDecl>& type) {
			TypeBuilder builder(context);
			switch (type->typeEnum) {
				case AST::TypeDecl::AUTO: {
					return SEM::Type::Auto(context.semContext());
				}
				case AST::TypeDecl::CONST: {
					return ConvertType(context, type->getConstTarget())->createTransitiveConstType(SEM::Predicate::True());
				}
				case AST::TypeDecl::CONSTPREDICATE: {
					auto constPredicate = ConvertPredicate(context, type->getConstPredicate());
					const auto constTarget = ConvertType(context, type->getConstPredicateTarget());
					return constTarget->createTransitiveConstType(std::move(constPredicate));
				}
				case AST::TypeDecl::NOTAG: {
					return ConvertType(context, type->getNoTagTarget())->createNoTagType();
				}
				case AST::TypeDecl::LVAL: {
					auto targetType = ConvertType(context, type->getLvalTarget());
					return ConvertType(context, type->getLvalType())->createLvalType(targetType);
				}
				case AST::TypeDecl::REF: {
					auto targetType = ConvertType(context, type->getRefTarget());
					return ConvertType(context, type->getRefType())->createRefType(targetType);
				}
				case AST::TypeDecl::STATICREF: {
					auto targetType = ConvertType(context, type->getStaticRefTarget());
					return ConvertType(context, type->getStaticRefType())->createStaticRefType(targetType);
				}
				case AST::TypeDecl::VOID: {
					return context.typeBuilder().getVoidType();
				}
				case AST::TypeDecl::BOOL: {
					return context.typeBuilder().getBoolType();
				}
				case AST::TypeDecl::PRIMITIVE: {
					return context.typeBuilder().getPrimitiveType(type->primitiveID());
				}
				case AST::TypeDecl::INTEGER: {
					return ConvertIntegerType(context, type->integerType.signedModifier, type->integerType.name);
				}
				case AST::TypeDecl::FLOAT: {
					return ConvertFloatType(context, type->floatType.name);
				}
				case AST::TypeDecl::OBJECT: {
					return ConvertObjectType(context, type->objectType.symbol);
				}
				case AST::TypeDecl::REFERENCE: {
					const auto targetType = ConvertType(context, type->getReferenceTarget());
					return createReferenceType(context, targetType);
				}
				case AST::TypeDecl::POINTER: {
					const auto targetType = ConvertType(context, type->getPointerTarget());
					return builder.getPointerType(targetType);
				}
				case AST::TypeDecl::STATICARRAY: {
					const auto targetType = ConvertType(context, type->getStaticArrayTarget());
					auto arraySize = ConvertValue(context, type->getArraySize());
					return builder.getStaticArrayType(targetType,
					                                  std::move(arraySize),
					                                  type.location());
				}
				case AST::TypeDecl::FUNCTION: {
					const auto returnType = ConvertType(context, type->functionType.returnType);
					
					const auto& astParameterTypes = type->functionType.parameterTypes;
					
					SEM::TypeArray parameterTypes;
					parameterTypes.reserve(astParameterTypes->size());
					
					for (const auto& astParamType: *astParameterTypes) {
						const auto paramType = ConvertType(context, astParamType);
						
						if (paramType->isBuiltInVoid()) {
							context.issueDiag(FunctionTypeParameterCannotBeVoidDiag(),
							                  astParamType.location());
						}
						
						parameterTypes.push_back(paramType);
					}
					
					// Currently no syntax exists to express a method function type.
					const bool isDynamicMethod = false;
					
					// Currently no syntax exists to express a templated function type.
					const bool isTemplated = false;
					
					// Currently no syntax exists to express a type with 'noexcept'.
					auto noexceptPredicate = SEM::Predicate::False();
					
					const bool isVarArg = type->functionType.isVarArg;
					
					SEM::FunctionAttributes attributes(isVarArg, isDynamicMethod, isTemplated, std::move(noexceptPredicate));
					const SEM::FunctionType builtInFunctionType(std::move(attributes),  returnType, std::move(parameterTypes));
					
					return builder.getFunctionPointerType(builtInFunctionType);
				}
			}
			
			std::terminate();
		}
		
		class PredicateAliasNotBoolDiag: public Error {
		public:
			PredicateAliasNotBoolDiag(const Name& name, const SEM::Type* const type)
			: name_(name.copy()), typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("alias '%s' has non-boolean type '%s' and "
				                  "therefore cannot be used in predicate",
				                  name_.toString(/*addPrefix=*/false).c_str(),
				                  typeString_.c_str());
			}
			
		private:
			Name name_;
			std::string typeString_;
			
		};
		
		static SEM::Alias*
		getTemplateVarTypeAlias(Context& context, const AST::Node<AST::TypeDecl>& type) {
			if (!type->isObjectType()) return nullptr;
			
			const Name name = type->symbol()->createName();
			const auto searchResult = performSearch(context, name);
			if (!searchResult.isAlias()) return nullptr;
			
			auto& alias = searchResult.alias();
			if (alias.templateVariables().size() != 1) return nullptr;
			
			for (size_t i = 0; i < type->symbol()->size(); i++) {
				if (type->symbol()->at(i)->templateArguments()->size() != 0) return nullptr;
			}
			
			return &alias;
		}
		
		SEM::Predicate
		getTemplateVarTypePredicate(Context& context, const AST::Node<AST::TypeDecl>& type,
		                            const SEM::TemplateVar& templateVar) {
			const auto alias = getTemplateVarTypeAlias(context, type);
			if (alias == nullptr) {
				return SEM::Predicate::True();
			}
			
			(void) context.aliasTypeResolver().resolveAliasType(*alias);
			assert(alias->templateVariables().size() == 1);
			
			SEM::ValueArray values;
			values.push_back(templateVar.selfRefValue());
			auto templateVarMap = GenerateTemplateVarMap(context, *alias,
			                                             std::move(values),
			                                             type.location());
			const auto aliasValue = alias->value().substitute(templateVarMap);
			if (!aliasValue.type()->isBuiltInBool()) {
				context.issueDiag(PredicateAliasNotBoolDiag(type->symbol()->createName(),
				                                            aliasValue.type()),
				                  type.location());
				return SEM::Predicate::True();
			}
			
			return aliasValue.makePredicate();
		}
		
		const SEM::Type* ConvertTemplateVarType(Context& context, const AST::Node<AST::TypeDecl>& type) {
			if (getTemplateVarTypeAlias(context, type) != nullptr) {
				return TypeBuilder(context).getTypenameType();
			} else {
				return ConvertType(context, type);
			}
		}
		
	}
	
}

