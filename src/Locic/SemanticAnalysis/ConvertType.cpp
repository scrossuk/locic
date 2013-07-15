#include <cstdio>
#include <Locic/AST.hpp>
#include <Locic/Map.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>
#include <Locic/SemanticAnalysis/Exception.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		Map<SEM::TemplateVar*, SEM::Type*> GenerateTemplateVarMap(Context& context, const AST::Symbol& symbol) {
			const Name fullName = symbol.createName();
			assert(fullName.size() == symbol.size());
			
			Map<SEM::TemplateVar*, SEM::Type*> templateVarMap;
			
			for(size_t i = 0; i < symbol.size(); i++){
				const AST::SymbolElement& element = symbol.at(i);
				const std::vector<AST::Type*>& astTemplateArgs = element.templateArguments();
				const size_t numTemplateArguments = astTemplateArgs.size();
				
				const Name name = fullName.substr(i + 1);
				
				const Node objectNode = context.lookupName(name);
				if(objectNode.isTypeInstance()){
					SEM::TypeInstance* typeInstance = objectNode.getSEMTypeInstance();
					const size_t numTemplateVariables = typeInstance->templateVariables().size();
					if(numTemplateVariables != numTemplateArguments){
						throw TodoException(makeString("Incorrect number of template "
							"arguments provided for type '%s'; %llu were required, "
							"but %llu were provided.", name.toString().c_str(),
							(unsigned long long) numTemplateVariables,
							(unsigned long long) numTemplateArguments));
					}
					
					for(size_t j = 0; j < numTemplateArguments; j++){
						templateVarMap.insert(typeInstance->templateVariables().at(j),
							ConvertType(context, astTemplateArgs.at(j), SEM::Type::LVALUE));
					}
				}else{
					if(numTemplateArguments > 0){
						throw TodoException(makeString("%llu template "
							"arguments provided for non-type node '%s'; none should be provided.",
							(unsigned long long) numTemplateArguments,
							name.toString().c_str()));
					}
				}
			}
			
			return templateVarMap;
		}
		
		std::vector<SEM::Type*> GetTemplateValues(Context& context, const AST::Symbol& symbol) {
			std::vector<SEM::Type*> templateArguments;
			for(size_t i = 0; i < symbol.size(); i++){
				const AST::SymbolElement& element = symbol.at(i);
				const std::vector<AST::Type*>& astTemplateArgs = element.templateArguments();
				const size_t numTemplateArguments = astTemplateArgs.size();
				
				for(size_t j = 0; j < numTemplateArguments; j++){
					templateArguments.push_back(ConvertType(context, astTemplateArgs.at(j), SEM::Type::LVALUE));
				}
			}
			return templateArguments;
		}
		
		SEM::Type* ConvertType(Context& context, AST::Type* type, bool isLValue) {
			switch(type->typeEnum) {
				case AST::Type::UNDEFINED: {
					assert(false && "Cannot convert undefined type.");
					return NULL;
				}
				case AST::Type::VOID: {
					return SEM::Type::Void();
				}
				case AST::Type::LVAL: {
					assert(isLValue);
					return SEM::Type::Lval(ConvertType(context, type->lvalType.targetType, SEM::Type::LVALUE));
				}
				case AST::Type::OBJECT: {
					const AST::Symbol& symbol = type->objectType.symbol;
					assert(!symbol.empty());
					
					const Name name = symbol.createName();
					const Node objectNode = context.lookupName(name);
					
					const Map<SEM::TemplateVar*, SEM::Type*> templateVarMap = GenerateTemplateVarMap(context, symbol);
					
					if(objectNode.isTypeInstance()) {
						SEM::TypeInstance* typeInstance = objectNode.getSEMTypeInstance();
						
						assert(templateVarMap.size() == typeInstance->templateVariables().size());
						
						std::vector<SEM::Type*> templateArguments;
						for(size_t i = 0; i < typeInstance->templateVariables().size(); i++){
							templateArguments.push_back(templateVarMap.get(typeInstance->templateVariables().at(i)));
						}
						
						return SEM::Type::Object(type->isMutable, isLValue, typeInstance, templateArguments);
					}else if(objectNode.isTemplateVar()) {
						assert(templateVarMap.empty());
						
						SEM::TemplateVar* templateVar = objectNode.getSEMTemplateVar();
						
						return SEM::Type::TemplateVarRef(type->isMutable, isLValue, templateVar);
					}else{
						throw TodoException(makeString("Unknown type with name '%s'.", name.toString().c_str()));
					}
				}
				case AST::Type::POINTER: {
					// Pointed-to types are always l-values (otherwise they couldn't have their address taken).
					SEM::Type* pointerType = ConvertType(context, type->getPointerTarget(), SEM::Type::LVALUE);
					
					return SEM::Type::Pointer(type->isMutable, isLValue, pointerType);
				}
				case AST::Type::REFERENCE: {
					// Referred-to types are always l-values.
					SEM::Type* refType = ConvertType(context, type->getReferenceTarget(), SEM::Type::LVALUE);
					
					return SEM::Type::Reference(isLValue, refType);
				}
				case AST::Type::FUNCTION: {
					SEM::Type* returnType = ConvertType(context, type->functionType.returnType, SEM::Type::RVALUE);
					
					std::vector<SEM::Type*> parameterTypes;
					
					const std::vector<AST::Type*>& astParameterTypes = type->functionType.parameterTypes;
					
					for(std::size_t i = 0; i < astParameterTypes.size(); i++) {
						SEM::Type* paramType = ConvertType(context, astParameterTypes.at(i), SEM::Type::LVALUE);
						
						if(paramType->isVoid()) {
							throw TodoException("Parameter type (inside function type) cannot be void.");
						}
						
						parameterTypes.push_back(paramType);
					}
					
					return SEM::Type::Function(isLValue, type->functionType.isVarArg, returnType, parameterTypes);
				}
				default:
					assert(false && "Unknown AST::Type type enum.");
					return NULL;
			}
		}
		
	}
	
}

