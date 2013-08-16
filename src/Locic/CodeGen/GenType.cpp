#include <string>
#include <vector>

#include <Locic/CodeGen/GenType.hpp>
#include <Locic/CodeGen/GenTypeInstance.hpp>
#include <Locic/CodeGen/Mangling.hpp>
#include <Locic/CodeGen/Module.hpp>
#include <Locic/CodeGen/Primitives.hpp>
#include <Locic/CodeGen/Support.hpp>
#include <Locic/CodeGen/TypeGenerator.hpp>

namespace Locic {

	namespace CodeGen {
	
		bool resolvesToClassType(Module& module, SEM::Type* type) {
			assert(type != NULL);
			return module.resolveType(type)->isClass();
		}
		
		llvm::FunctionType* genFunctionType(Module& module, SEM::Type* type, llvm::Type* contextPointerType) {
			assert(type != NULL && "Generating a function type requires a non-NULL SEM Type object");
			assert(type->isFunction() && "Type must be a function type for it to be generated as such");
			
			SEM::Type* semReturnType = type->getFunctionReturnType();
			assert(semReturnType != NULL && "Generating function return type requires a non-NULL SEM return type");
			
			llvm::Type* returnType = genType(module, semReturnType);
			std::vector<llvm::Type*> paramTypes;
			
			if (resolvesToClassType(module, semReturnType)) {
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
				
				if (resolvesToClassType(module, paramType)) {
					rawType = rawType->getPointerTo();
				}
				
				paramTypes.push_back(rawType);
			}
			
			return TypeGenerator(module).getFunctionType(returnType, paramTypes, type->isFunctionVarArg());
		}
		
		llvm::Type* genObjectType(Module& module, SEM::TypeInstance* typeInstance,
			const std::vector<SEM::Type*>& templateArguments) {
			if (typeInstance->isPrimitive()) {
				std::vector<llvm::Type*> generatedArguments;
				for (size_t i = 0; i < templateArguments.size(); i++) {
					generatedArguments.push_back(genType(module, templateArguments.at(i)));
				}
				return getPrimitiveType(module, typeInstance->name().last(), generatedArguments);
			} else {
				assert(!typeInstance->isInterface() && "Interface types must always be converted by reference");
				return genTypeInstance(module, typeInstance, templateArguments);
			}
		}
		
		llvm::Type* genPointerType(Module& module, SEM::Type* targetType) {
			if (targetType->isObject()) {
				return getTypeInstancePointer(module, targetType->getObjectType(), targetType->templateArguments());
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
		
		llvm::StructType* getInterfaceStruct(Module& module) {
			// Interface references are actually two pointers:
			// one to the class, and one to the class vtable.
			std::vector<llvm::Type*> types;
			// Class pointer.
			types.push_back(TypeGenerator(module).getI8PtrType());
			// Vtable pointer.
			types.push_back(getVTableType(module.getTargetInfo())->getPointerTo());
			return TypeGenerator(module).getStructType(types);
		}
		
		llvm::Type* getTypeInstancePointer(Module& module, SEM::TypeInstance* typeInstance, const std::vector<SEM::Type*>& templateArguments) {
			if (typeInstance->isInterface()) {
				return getInterfaceStruct(module);
			} else {
				return genObjectType(module, typeInstance, templateArguments)->getPointerTo();
			}
		}
		
		llvm::Type* genType(Module& module, SEM::Type* unresolvedType) {
			assert(unresolvedType != NULL);
			
			SEM::Type* type = module.resolveType(unresolvedType);
			
			LOG(LOG_INFO, "genType(type: %s, mangledType: %s)",
				type->toString().c_str(), mangleType(module, type).c_str());
			
			switch (type->kind()) {
				case SEM::Type::VOID: {
					return TypeGenerator(module).getVoidType();
				}
				
				case SEM::Type::NULLT: {
					return TypeGenerator(module).getI8PtrType();
				}
				
				case SEM::Type::OBJECT: {
					return genObjectType(module, type->getObjectType(),
						type->templateArguments());
				}
				
				case SEM::Type::REFERENCE: {
					return genPointerType(module, type->getReferenceTarget());
				}
				
				case SEM::Type::FUNCTION: {
					return genFunctionType(module, type)->getPointerTo();
				}
				
				case SEM::Type::METHOD: {
					/* Method type is:
						struct {
							i8* context,
							RetType (*func)(i8*, ArgTypes)
						};
					*/
					std::vector<llvm::Type*> types;
					llvm::Type* contextPtrType = TypeGenerator(module).getI8PtrType();
					types.push_back(genFunctionType(module, type->getMethodFunctionType(),
							contextPtrType)->getPointerTo());
					types.push_back(contextPtrType);
					return TypeGenerator(module).getStructType(types);
				}
				
				case SEM::Type::INTERFACEMETHOD: {
					/* Interface method type is:
						struct {
							struct {
								i8* context,
								__vtable_type* vtable
							},
							i32 methodHash
						};
					*/
					std::vector<llvm::Type*> interfaceMethodTypes;
					interfaceMethodTypes.push_back(
						getInterfaceStruct(module));
					interfaceMethodTypes.push_back(
						TypeGenerator(module).getI32Type());
					
					return TypeGenerator(module).getStructType(interfaceMethodTypes);
				}
				
				default: {
					assert(false && "Unknown type enum for generating type");
					return TypeGenerator(module).getVoidType();
				}
			}
		}
		
	}
	
}

