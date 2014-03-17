#include <string>
#include <stdexcept>
#include <vector>

#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenTypeInstance.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::FunctionType* genFunctionType(Module& module, SEM::Type* type, llvm::Type* contextPointerType) {
			assert(type != NULL && "Generating a function type requires a non-NULL SEM Type object");
			assert(type->isFunction() && "Type must be a function type for it to be generated as such");
			
			SEM::Type* semReturnType = type->getFunctionReturnType();
			assert(semReturnType != NULL && "Generating function return type requires a non-NULL SEM return type");
			
			llvm::Type* returnType = genType(module, semReturnType);
			std::vector<llvm::Type*> paramTypes;
			
			if (!isTypeSizeAlwaysKnown(module, semReturnType)) {
				// Unknown size return values are constructed on the caller's
				// stack, and given to the callee as a pointer.
				paramTypes.push_back(returnType->getPointerTo());
				returnType = TypeGenerator(module).getVoidType();
			}
			
			if (contextPointerType != nullptr) {
				// If there's a context pointer (for non-static methods),
				// add it before the other (normal) arguments.
				paramTypes.push_back(contextPointerType);
			}
			
			for (const auto paramType: type->getFunctionParameterTypes()) {
				llvm::Type* rawType = genType(module, paramType);
				
				if (!isTypeSizeAlwaysKnown(module, paramType)) {
					rawType = rawType->getPointerTo();
				}
				
				paramTypes.push_back(rawType);
			}
			
			const auto genericFunctionType = TypeGenerator(module).getFunctionType(returnType, paramTypes, type->isFunctionVarArg());
			return module.abi().rewriteFunctionType(genericFunctionType, genABIFunctionType(module, type, contextPointerType));
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
							i64 methodHash
						};
					*/
					std::vector<llvm::Type*> interfaceMethodTypes;
					interfaceMethodTypes.push_back(
						getInterfaceStruct(module));
					interfaceMethodTypes.push_back(
						TypeGenerator(module).getI64Type());
					
					return TypeGenerator(module).getStructType(interfaceMethodTypes);
				}
				
				default: {
					assert(false && "Unknown type enum for generating type");
					return TypeGenerator(module).getVoidType();
				}
			}
		}
		
		llvm::DIType genDebugType(Module& module, SEM::Type* unresolvedType) {
			assert(unresolvedType != NULL);
			
			const auto type = module.resolveType(unresolvedType);
			
			LOG(LOG_INFO, "genDebugType(type: %s, mangledType: %s)",
				type->toString().c_str(), mangleType(module, type).c_str());
			
			switch (type->kind()) {
				case SEM::Type::VOID: {
					return module.debugBuilder().createVoidType();
				}
				
				case SEM::Type::OBJECT: {
					const auto objectType = type->getObjectType();
					if (objectType->isPrimitive()) {
						if (objectType->name() == (Name::Absolute() + "null_t")) {
							return module.debugBuilder().createNullType();
						}
						
						if (objectType->name() == (Name::Absolute() + "ptr")) {
							return module.debugBuilder().createPointerType(genDebugType(module, type->templateArguments().front()));
						}
						
						if (objectType->name() == (Name::Absolute() + "int")) {
							return module.debugBuilder().createIntType("int");
						}
					}
					
					// TODO!
					const auto file = module.debugBuilder().createFile("/object/dir/example_source_file.loci");
					const auto lineNumber = 12;
					
					return module.debugBuilder().createObjectType(file, lineNumber, objectType->name());
				}
				
				case SEM::Type::REFERENCE: {
					return module.debugBuilder().createReferenceType(genDebugType(module, type->getReferenceTarget()));
				}
				
				case SEM::Type::FUNCTION: {
					// TODO!
					const auto file = module.debugBuilder().createFile("/object/dir/example_source_file.loci");
					
					std::vector<llvm::Value*> parameterTypes;
					parameterTypes.push_back(genDebugType(module, type->getFunctionReturnType()));
					
					for (const auto paramType: type->getFunctionParameterTypes()) {
						parameterTypes.push_back(genDebugType(module, paramType));
					}
					
					return module.debugBuilder().createFunctionType(file, parameterTypes);
				}
				
				case SEM::Type::METHOD: {
					// TODO!
					return module.debugBuilder().createNullType();
				}
				
				case SEM::Type::INTERFACEMETHOD: {
					// TODO!
					return module.debugBuilder().createNullType();
				}
				
				default: {
					throw std::runtime_error("Unknown type enum for generating type.");
				}
			}
		}
		
	}
	
}

