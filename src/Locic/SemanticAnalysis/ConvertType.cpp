#include <cstdio>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Type* ConvertType(Context& context, AST::Type* type, bool isLValue) {
			switch(type->typeEnum) {
				case AST::Type::UNDEFINED: {
					printf("Internal Compiler Error: Cannot convert undefined type.\n");
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
					
					if(!objectNode.isTypeInstance()) {
						printf("Semantic Analysis Error: Unknown type with name '%s'.\n", name.toString().c_str());
						return NULL;
					}
					
					SEM::TypeInstance* typeInstance = objectNode.getSEMTypeInstance();
					
					// TODO: Handle template arguments.
					std::vector<SEM::Type*> templateArguments;
					const std::vector<AST::Type*>& astTemplateArgs = symbol.last().templateArguments();
					for(size_t i = 0; i < astTemplateArgs.size(); i++) {
						templateArguments.push_back(ConvertType(context, astTemplateArgs.at(i), SEM::Type::LVALUE));
					}
					
					return SEM::Type::Object(type->isMutable, isLValue, typeInstance, templateArguments);
				}
				case AST::Type::POINTER: {
					// Pointed-to types are always l-values (otherwise they couldn't have their address taken).
					SEM::Type* pointerType = ConvertType(context, type->getPointerTarget(), SEM::Type::LVALUE);
					
					if(pointerType == NULL) {
						return NULL;
					}
					
					return SEM::Type::Pointer(type->isMutable, isLValue, pointerType);
				}
				case AST::Type::REFERENCE: {
					// Referred-to types are always l-values.
					SEM::Type* refType = ConvertType(context, type->getReferenceTarget(), SEM::Type::LVALUE);
					
					if(refType == NULL) {
						return NULL;
					}
					
					return SEM::Type::Reference(isLValue, refType);
				}
				case AST::Type::FUNCTION: {
					SEM::Type* returnType = ConvertType(context, type->functionType.returnType, SEM::Type::RVALUE);
					
					if(returnType == NULL) {
						return NULL;
					}
					
					std::vector<SEM::Type*> parameterTypes;
					
					const std::vector<AST::Type*>& astParameterTypes = type->functionType.parameterTypes;
					
					for(std::size_t i = 0; i < astParameterTypes.size(); i++) {
						SEM::Type* paramType = ConvertType(context, astParameterTypes.at(i), SEM::Type::LVALUE);
						
						if(paramType == NULL) {
							return NULL;
						}
						
						if(paramType->isVoid()) {
							printf("Semantic Analysis Error: Parameter type (inside function type) cannot be void.\n");
							return NULL;
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

