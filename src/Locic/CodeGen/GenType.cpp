#include <string>
#include <vector>

#include <Locic/CodeGen/GenType.hpp>
#include <Locic/CodeGen/Mangling.hpp>
#include <Locic/CodeGen/Module.hpp>
#include <Locic/CodeGen/Primitives.hpp>
#include <Locic/CodeGen/Support.hpp>
#include <Locic/CodeGen/TypeGenerator.hpp>

namespace Locic {

	namespace CodeGen {
	
		llvm::FunctionType* genFunctionType(Module& module, SEM::Type* type, llvm::Type* contextPointerType) {
			assert(type != NULL && "Generating a function type requires a non-NULL SEM Type object");
			assert(type->isFunction() && "Type must be a function type for it to be generated as such");
			
			SEM::Type* semReturnType = type->getFunctionReturnType();
			assert(semReturnType != NULL && "Generating function return type requires a non-NULL SEM return type");
			
			llvm::Type* returnType = genType(module, semReturnType);
			std::vector<llvm::Type*> paramTypes;
			
			if (semReturnType->isClassOrTemplateVar()) {
				// Class return values are constructed on the caller's
				// stack, and given to the callee as a pointer.
				paramTypes.push_back(returnType->getPointerTo());
				returnType = TypeGenerator(module).getVoidType();
			}
			
			if (contextPointerType != NULL) {
				// If there's a context pointer (for non-static methods),
				// add it before the other (normal) arguments.
				paramTypes.push_back(contextPointerType);
			}
			
			const std::vector<SEM::Type*>& params = type->getFunctionParameterTypes();
			
			for (std::size_t i = 0; i < params.size(); i++) {
				SEM::Type* paramType = params.at(i);
				llvm::Type* rawType = genType(module, paramType);
				
				if (paramType->isObject()) {
					SEM::TypeInstance* typeInstance = paramType->getObjectType();
					
					if (typeInstance->isClass()) {
						rawType = rawType->getPointerTo();
					}
				}
				
				paramTypes.push_back(rawType);
			}
			
			return TypeGenerator(module).getFunctionType(returnType, paramTypes, type->isFunctionVarArg());
		}
		
		llvm::Type* genObjectType(Module& module, SEM::TypeInstance* typeInstance) {
			if (typeInstance->isPrimitive()) {
				return getPrimitiveType(module, typeInstance->name().last());
			} else {
				assert(!typeInstance->isInterface() && "Interface types must always be converted by pointer");
				return module.getTypeMap().get(typeInstance);
			}
		}
		
		llvm::Type* genPointerType(Module& module, SEM::Type* targetType) {
			if (targetType->isObject()) {
				return getTypeInstancePointer(module, targetType->getObjectType());
			} else {
				llvm::Type* pointerType = genType(module, targetType);
				
				if (pointerType->isVoidTy()) {
					// LLVM doesn't support 'void *' => use 'int8_t *' instead.
					return TypeGenerator(module).getI8PtrType();
				} else {
					return pointerType->getPointerTo();
				}
			}
		}
		
		llvm::Type* getTypeInstancePointer(Module& module, SEM::TypeInstance* typeInstance) {
			if (typeInstance->isInterface()) {
				// Interface pointers/references are actually two pointers:
				// one to the class, and one to the class vtable.
				std::vector<llvm::Type*> types;
				// Class pointer.
				types.push_back(module.getTypeMap().get(typeInstance)->getPointerTo());
				// Vtable pointer.
				types.push_back(getVTableType(module.getTargetInfo())->getPointerTo());
				return TypeGenerator(module).getStructType(types);
			} else {
				return genObjectType(module, typeInstance)->getPointerTo();
			}
		}
		
		llvm::Type* genType(Module& module, SEM::Type* type) {
			LOG(LOG_INFO, "genType(type: %s, mangledType: %s)",
				type->toString().c_str(), mangleType(type).c_str());
			
			switch (type->kind()) {
				case SEM::Type::VOID: {
					return TypeGenerator(module).getVoidType();
				}
				
				case SEM::Type::NULLT: {
					return TypeGenerator(module).getI8PtrType();
				}
				
				case SEM::Type::OBJECT: {
					return genObjectType(module, type->getObjectType());
				}
				
				case SEM::Type::POINTER: {
					return genPointerType(module, type->getPointerTarget());
				}
				
				case SEM::Type::REFERENCE: {
					return genPointerType(module, type->getReferenceTarget());
				}
				
				case SEM::Type::FUNCTION: {
					return genFunctionType(module, type)->getPointerTo();
				}
				
				case SEM::Type::METHOD: {
					std::vector<llvm::Type*> types;
					llvm::Type* contextPtrType = TypeGenerator(module).getI8PtrType();
					types.push_back(genFunctionType(module, type->getMethodFunctionType(),
							contextPtrType)->getPointerTo());
					types.push_back(contextPtrType);
					return TypeGenerator(module).getStructType(types);
				}
				
				case SEM::Type::TEMPLATEVAR: {
					assert(false && "Cannot generate template variable type.");
					return NULL;
				}
				
				default: {
					assert(false && "Unknown type enum for generating type");
					return TypeGenerator(module).getVoidType();
				}
			}
		}
		
	}
	
}

