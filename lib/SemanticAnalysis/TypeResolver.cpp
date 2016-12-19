#include <stdio.h>

#include <stdexcept>

#include <locic/AST.hpp>
#include <locic/AST/Type.hpp>
#include <locic/Support/Map.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/AliasTypeResolver.hpp>
#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>
#include <locic/SemanticAnalysis/Template.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>
#include <locic/SemanticAnalysis/TypeResolver.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		TypeResolver::TypeResolver(Context& context)
		: context_(context) { }
		
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
		
		const AST::Type*
		TypeResolver::resolveObjectType(const AST::Node<AST::Symbol>& symbol) {
			assert(!symbol->empty());
			
			const Name name = symbol->createName();
			
			const auto searchResult = performSearch(context_, name);
			
			const auto templateVarMap = GenerateSymbolTemplateVarMap(context_, symbol);
			
			if (searchResult.isTypeInstance()) {
				auto& typeInstance = searchResult.typeInstance();
				
				assert(templateVarMap.size() == typeInstance.templateVariables().size());
				
				return AST::Type::Object(&typeInstance, GetTemplateValues(templateVarMap, typeInstance.templateVariables()));
			} else if (searchResult.isTemplateVar()) {
				assert(templateVarMap.empty());
				
				return AST::Type::TemplateVarRef(&(searchResult.templateVar()));
			} else if (searchResult.isAlias()) {
				auto& alias = searchResult.alias();
				
				assert(templateVarMap.size() == alias.templateVariables().size());
				
				auto templateValues = GetTemplateValues(templateVarMap, alias.templateVariables());
				assert(templateValues.size() == alias.templateVariables().size());
				
				return AST::Type::Alias(alias, std::move(templateValues));
			} else {
				context_.issueDiag(UnknownTypeNameDiag(name.copy()),
				                   symbol.location());
				return context_.typeBuilder().getIntType();
			}
		}
		
		const AST::Type*
		TypeResolver::resolveIntegerType(AST::TypeDecl::SignedModifier signedModifier,
		                                 const String& nameString) {
			// Unsigned types have 'u' prefix and all integer types
			// have '_t' suffix (e.g. uint_t, short_t etc.).
			const auto fullNameString = (signedModifier == AST::TypeDecl::UNSIGNED) ?
				(context_.getCString("u") + nameString + "_t") :
				(nameString + "_t");
			return getBuiltInType(context_, fullNameString, {});
		}
		
		const AST::Type*
		TypeResolver::resolveFloatType(const String& nameString) {
			// All floating point types have '_t' suffix (e.g. float_t, double_t etc.).
			const auto fullNameString = nameString + "_t";
			return getBuiltInType(context_, fullNameString, {});
		}
		
		class FunctionTypeParameterCannotBeVoidDiag: public Error {
		public:
			FunctionTypeParameterCannotBeVoidDiag() { }
			
			std::string toString() const {
				return "parameter type in function pointer type cannot be void";
			}
			
		};
		
		const AST::Type*
		TypeResolver::convertType(AST::Node<AST::TypeDecl>& type) {
			TypeBuilder builder(context_);
			switch (type->typeEnum) {
				case AST::TypeDecl::AUTO: {
					return AST::Type::Auto(context_.semContext());
				}
				case AST::TypeDecl::CONST: {
					return resolveType(type->getConstTarget())->createTransitiveConstType(SEM::Predicate::True());
				}
				case AST::TypeDecl::CONSTPREDICATE: {
					auto constPredicate = ConvertPredicate(context_, type->getConstPredicate());
					const auto constTarget = resolveType(type->getConstPredicateTarget());
					return constTarget->createTransitiveConstType(std::move(constPredicate));
				}
				case AST::TypeDecl::NOTAG: {
					return resolveType(type->getNoTagTarget())->createNoTagType();
				}
				case AST::TypeDecl::LVAL: {
					return resolveType(type->getLvalType())->createLvalType();
				}
				case AST::TypeDecl::REF: {
					auto targetType = resolveType(type->getRefTarget());
					return resolveType(type->getRefType())->createRefType(targetType);
				}
				case AST::TypeDecl::STATICREF: {
					auto targetType = resolveType(type->getStaticRefTarget());
					return resolveType(type->getStaticRefType())->createStaticRefType(targetType);
				}
				case AST::TypeDecl::VOID: {
					return context_.typeBuilder().getVoidType();
				}
				case AST::TypeDecl::BOOL: {
					return context_.typeBuilder().getBoolType();
				}
				case AST::TypeDecl::PRIMITIVE: {
					return context_.typeBuilder().getPrimitiveType(type->primitiveID());
				}
				case AST::TypeDecl::INTEGER: {
					return resolveIntegerType(type->integerType.signedModifier,
					                          type->integerType.name);
				}
				case AST::TypeDecl::FLOAT: {
					return resolveFloatType(type->floatType.name);
				}
				case AST::TypeDecl::OBJECT: {
					return resolveObjectType(type->objectType.symbol);
				}
				case AST::TypeDecl::REFERENCE: {
					const auto targetType = resolveType(type->getReferenceTarget());
					return createReferenceType(context_, targetType);
				}
				case AST::TypeDecl::POINTER: {
					const auto targetType = resolveType(type->getPointerTarget());
					return builder.getPointerType(targetType);
				}
				case AST::TypeDecl::STATICARRAY: {
					const auto targetType = resolveType(type->getStaticArrayTarget());
					auto arraySize = ConvertValue(context_, type->getArraySize());
					return builder.getStaticArrayType(targetType,
					                                  std::move(arraySize),
					                                  type.location());
				}
				case AST::TypeDecl::FUNCTION: {
					const auto returnType = resolveType(type->functionType.returnType);
					
					const auto& astParameterTypes = type->functionType.parameterTypes;
					
					AST::TypeArray parameterTypes;
					parameterTypes.reserve(astParameterTypes->size());
					
					for (auto& astParamType: *astParameterTypes) {
						const auto paramType = resolveType(astParamType);
						
						if (paramType->isBuiltInVoid()) {
							context_.issueDiag(FunctionTypeParameterCannotBeVoidDiag(),
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
					
					AST::FunctionAttributes attributes(isVarArg, isDynamicMethod, isTemplated, std::move(noexceptPredicate));
					const AST::FunctionType builtInFunctionType(std::move(attributes),  returnType, std::move(parameterTypes));
					
					return builder.getFunctionPointerType(builtInFunctionType);
				}
			}
			
			locic_unreachable("Unknown AST::TypeDecl kind.");
		}
		
		const AST::Type*
		TypeResolver::resolveType(AST::Node<AST::TypeDecl>& type) {
			if (type->resolvedType() != nullptr) {
				return type->resolvedType();
			}
			const auto resolvedType = convertType(type);
			assert(resolvedType != nullptr);
			type->setResolvedType(resolvedType);
			return resolvedType;
		}
		
		class PredicateAliasNotBoolDiag: public Error {
		public:
			PredicateAliasNotBoolDiag(const Name& name, const AST::Type* const type)
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
		
		AST::Alias*
		TypeResolver::getTemplateVarTypeAlias(const AST::Node<AST::TypeDecl>& type) {
			if (!type->isObjectType()) return nullptr;
			
			const Name name = type->symbol()->createName();
			const auto searchResult = performSearch(context_, name);
			if (!searchResult.isAlias()) return nullptr;
			
			auto& alias = searchResult.alias();
			if (alias.templateVariables().size() != 1) return nullptr;
			
			for (size_t i = 0; i < type->symbol()->size(); i++) {
				if (type->symbol()->at(i)->templateArguments()->size() != 0) return nullptr;
			}
			
			return &alias;
		}
		
		SEM::Predicate
		TypeResolver::getTemplateVarTypePredicate(const AST::Node<AST::TypeDecl>& type,
		                                          const AST::TemplateVar& templateVar) {
			const auto alias = getTemplateVarTypeAlias(type);
			if (alias == nullptr) {
				return SEM::Predicate::True();
			}
			
			(void) context_.aliasTypeResolver().resolveAliasType(*alias);
			assert(alias->templateVariables().size() == 1);
			
			AST::ValueArray values;
			values.push_back(templateVar.selfRefValue());
			auto templateVarMap = GenerateTemplateVarMap(context_, *alias,
			                                             std::move(values),
			                                             type.location());
			const auto aliasValue = alias->value().substitute(templateVarMap);
			if (!aliasValue.type()->isBuiltInBool()) {
				context_.issueDiag(PredicateAliasNotBoolDiag(type->symbol()->createName(),
				                                             aliasValue.type()),
				                   type.location());
				return SEM::Predicate::True();
			}
			
			return aliasValue.makePredicate();
		}
		
		const AST::Type*
		TypeResolver::resolveTemplateVarType(AST::Node<AST::TypeDecl>& type) {
			if (getTemplateVarTypeAlias(type) != nullptr) {
				// If the template variable type is actually an
				// alias, this likely means it is a predicate
				// (e.g. <movable T>), hence it actually has
				// type 'typename'.
				return TypeBuilder(context_).getTypenameType();
			} else {
				return resolveType(type);
			}
		}
		
	}
	
}

