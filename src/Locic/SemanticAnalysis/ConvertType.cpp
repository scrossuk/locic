#include <cstdio>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>
#include <Locic/SemanticAnalysis/Exception.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Type* ConvertType(Context& context, AST::Type* type, bool isLValue) {
			switch(type->typeEnum) {
				case AST::Type::UNDEFINED: {
					assert(false && "Cannot convert undefined type.");
					return NULL;
				}
				case AST::Type::VOID: {
					return SEM::Type::Void();
				}
				case AST::Type::OBJECT: {
					const AST::Symbol& symbol = type->objectType.symbol;
					assert(!symbol.empty());
					
					const Name name = symbol.createName();
					const Node objectNode = context.lookupName(name);
					
					if(objectNode.isTypeInstance()) {
						SEM::TypeInstance* typeInstance = objectNode.getSEMTypeInstance();
						
						std::vector<SEM::Type*> templateArguments;
						const std::vector<AST::Type*>& astTemplateArgs = symbol.last().templateArguments();
						for(size_t i = 0; i < astTemplateArgs.size(); i++) {
							templateArguments.push_back(ConvertType(context, astTemplateArgs.at(i), SEM::Type::LVALUE));
						}
						
						const size_t numTemplateVariables = typeInstance->templateVariables().size();
						const size_t numTemplateArguments = templateArguments.size();
						if(numTemplateVariables != numTemplateArguments){
							throw TodoException(makeString("Incorrect number of template "
								"arguments provided for type '%s'; %llu were required, "
								"but %llu were provided.", name.toString().c_str(),
								(unsigned long long) numTemplateVariables,
								(unsigned long long) numTemplateArguments));
						}
						
						return SEM::Type::Object(type->isMutable, isLValue, typeInstance, templateArguments);
					}else if(objectNode.isTemplateVar()) {
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

