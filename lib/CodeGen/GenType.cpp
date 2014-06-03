#include <string>
#include <stdexcept>
#include <vector>

#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenTypeInstance.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Type* genArgType(Module& module, SEM::Type* type) {
			if (isTypeSizeAlwaysKnown(module, type)) {
				return genType(module, type);
			} else {
				return genPointerType(module, type);
			}
		}
		
		llvm::FunctionType* genFunctionType(Module& module, SEM::Type* type) {
			assert(type != nullptr && "Generating a function type requires a non-NULL SEM Type object");
			assert(type->isFunction() && "Type must be a function type for it to be generated as such");
			
			const auto semReturnType = type->getFunctionReturnType();
			assert(semReturnType != nullptr && "Generating function return type requires a non-NULL SEM return type");
			
			llvm::Type* returnType = nullptr;
			std::vector<llvm::Type*> paramTypes;
			
			if (isTypeSizeAlwaysKnown(module, semReturnType)) {
				returnType = genType(module, semReturnType);
			} else {
				// Unknown size return values are constructed on the caller's
				// stack, and given to the callee as a pointer.
				paramTypes.push_back(genPointerType(module, semReturnType));
				returnType = TypeGenerator(module).getVoidType();
			}
			
			if (type->isFunctionTemplatedMethod()) {
				// Add template generator arguments for methods of
				// templated types.
				paramTypes.push_back(templateGeneratorType(module));
			}
			
			if (type->isFunctionMethod()) {
				// If there's a context pointer (for non-static methods),
				// add it before the other (normal) arguments.
				paramTypes.push_back(TypeGenerator(module).getI8PtrType());
			}
			
			for (const auto paramType: type->getFunctionParameterTypes()) {
				paramTypes.push_back(genArgType(module, paramType));
			}
			
			const auto genericFunctionType = TypeGenerator(module).getFunctionType(returnType, paramTypes, type->isFunctionVarArg());
			return module.abi().rewriteFunctionType(genericFunctionType, genABIFunctionType(module, type));
		}
		
		llvm::Type* genObjectType(Module& module, SEM::TypeInstance* typeInstance) {
			if (typeInstance->isPrimitive()) {
				return getPrimitiveType(module, typeInstance->name().last());
			} else {
				assert(!typeInstance->isInterface() && "Interface types must always be converted by reference");
				return genTypeInstance(module, typeInstance);
			}
		}
		
		llvm::Type* genPointerType(Module& module, SEM::Type* targetType) {
			if (targetType->isObject()) {
				return getTypeInstancePointer(module, targetType->getObjectType());
			} else if (targetType->isTemplateVar()) {
				return TypeGenerator(module).getI8PtrType();
			} else {
				const auto pointerType = genType(module, targetType);
				
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
				return interfaceStructType(module);
			} else {
				return genObjectType(module, typeInstance)->getPointerTo();
			}
		}
		
		llvm::Type* genType(Module& module, SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::VOID: {
					return TypeGenerator(module).getVoidType();
				}
				
				case SEM::Type::OBJECT: {
					return genObjectType(module, type->getObjectType());
				}
				
				case SEM::Type::REFERENCE: {
					return genPointerType(module, type->getReferenceTarget());
				}
				
				case SEM::Type::FUNCTION: {
					// Generate struct of function pointer and template
					// generator if function type is templated method.
					const auto functionPtrType = genFunctionType(module, type)->getPointerTo();
					if (type->isFunctionTemplatedMethod()) {
						return TypeGenerator(module).getStructType({ functionPtrType, templateGeneratorType(module) });
					} else {
						return functionPtrType;
					}
				}
				
				case SEM::Type::METHOD: {
					/* Method type is (roughly):
						struct {
							i8* context;
							// If is templated method:
							struct {
								RetType (*func)(i8*, ArgTypes);
								struct {
									void* rootFn;
									uint32_t path;
								} templateGenerator;
							};
							// Otherwise:
							RetType (*func)(i8*, ArgTypes);
						};
					*/
					std::vector<llvm::Type*> types;
					types.push_back(TypeGenerator(module).getI8PtrType());
					types.push_back(genType(module, type->getMethodFunctionType()));
					return TypeGenerator(module).getStructType(types);
				}
				
				case SEM::Type::INTERFACEMETHOD: {
					return interfaceMethodType(module);
				}
				
				default: {
					llvm_unreachable("Unknown type enum for generating type.");
				}
			}
		}
		
		llvm::DIType genDebugType(Module& module, SEM::Type* type) {
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
						
						if (objectType->name() == (Name::Absolute() + "int_t")) {
							return module.debugBuilder().createIntType("int_t");
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

