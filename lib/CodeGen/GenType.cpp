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
			return getFunctionArgInfo(module, type).makeFunctionType();
		}
		
		llvm::Type* genObjectType(Module& module, SEM::Type* type) {
			const auto typeInstance = type->getObjectType();
			if (typeInstance->isPrimitive()) {
				return getPrimitiveType(module, type);
			} else {
				assert(!typeInstance->isInterface() && "Interface types must always be converted by reference");
				return genTypeInstance(module, type->getObjectType());
			}
		}
		
		llvm::PointerType* genPointerType(Module& module, SEM::Type* targetType) {
			if (targetType->isObject()) {
				assert(!targetType->isInterface());
				return genObjectType(module, targetType)->getPointerTo();
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
		
		llvm::Type* genType(Module& module, SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::OBJECT: {
					return genObjectType(module, type);
				}
				
				case SEM::Type::FUNCTION: {
					// Generate struct of function pointer and template
					// generator if function type is templated.
					const auto functionPtrType = genFunctionType(module, type)->getPointerTo();
					if (type->isFunctionTemplated()) {
						llvm::Type* const memberTypes[] = { functionPtrType, templateGeneratorType(module).second };
						return TypeGenerator(module).getStructType(memberTypes);
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
					llvm::SmallVector<llvm::Type*, 2> types;
					types.push_back(TypeGenerator(module).getI8PtrType());
					types.push_back(genType(module, type->getMethodFunctionType()));
					return TypeGenerator(module).getStructType(types);
				}
				
				case SEM::Type::INTERFACEMETHOD: {
					return interfaceMethodType(module).second;
				}
				
				default: {
					llvm_unreachable("Unknown type enum for generating type.");
				}
			}
		}
		
		llvm::DIType genDebugType(Module& module, SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::OBJECT: {
					const auto objectType = type->getObjectType();
					if (objectType->isPrimitive()) {
						if (objectType->name() == (Name::Absolute() + "void")) {
							return module.debugBuilder().createVoidType();
						}
						
						if (objectType->name() == (Name::Absolute() + "null_t")) {
							return module.debugBuilder().createNullType();
						}
						
						if (objectType->name() == (Name::Absolute() + "ptr")) {
							return module.debugBuilder().createPointerType(genDebugType(module, type->templateArguments().front()));
						}
						
						if (objectType->name() == (Name::Absolute() + "int_t")) {
							return module.debugBuilder().createIntType("int_t");
						}
						
						if (objectType->name() == (Name::Absolute() + "__ref")) {
							return module.debugBuilder().createReferenceType(genDebugType(module, type->templateArguments().front()));
						}
					}
					
					// TODO!
					const auto file = module.debugBuilder().createFile("/object/dir/example_source_file.loci");
					const auto lineNumber = 12;
					
					return module.debugBuilder().createObjectType(file, lineNumber, objectType->name());
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
				
				case SEM::Type::TEMPLATEVAR: {
					// TODO!
					const auto file = module.debugBuilder().createFile("/object/dir/example_source_file.loci");
					const auto lineNumber = 12;
					const auto name = Name::Absolute() + "T";
					
					return module.debugBuilder().createObjectType(file, lineNumber, name);
				}
				
				default: {
					throw std::runtime_error("Unknown type enum for generating type.");
				}
			}
		}
		
	}
	
}

