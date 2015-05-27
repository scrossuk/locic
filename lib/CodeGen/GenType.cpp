#include <string>
#include <stdexcept>
#include <vector>

#include <locic/CodeGen/ArgInfo.hpp>
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

namespace locic {

	namespace CodeGen {
	
		llvm::Type* genArgType(Module& module, const SEM::Type* type) {
			if (canPassByValue(module, type)) {
				return genType(module, type);
			} else {
				return genPointerType(module, type);
			}
		}
		
		llvm::FunctionType* genFunctionType(Module& module, SEM::FunctionType type) {
			return getFunctionArgInfo(module, type).makeFunctionType();
		}
		
		llvm::Type* genObjectType(Module& module, const SEM::Type* type) {
			const auto typeInstance = type->getObjectType();
			if (typeInstance->isPrimitive()) {
				return getPrimitiveType(module, type);
			} else {
				assert(!typeInstance->isInterface() && "Interface types must always be converted by reference");
				return genTypeInstance(module, type->getObjectType());
			}
		}
		
		llvm::PointerType* genPointerType(Module& module, const SEM::Type* rawTargetType) {
			const auto targetType = rawTargetType->resolveAliases();
                        assert(!targetType->isInterface());
			if (targetType->isPrimitive()) {
				return getPrimitivePointerType(module, targetType);
			} else if (targetType->isTemplateVar()) {
				return TypeGenerator(module).getI8PtrType();
			} else {
				const auto llvmTargetType = genType(module, targetType);
				if (llvmTargetType->isVoidTy()) {
					// LLVM doesn't support 'void *' => use 'int8_t *' instead.
					return TypeGenerator(module).getI8PtrType();
				} else {
					return llvmTargetType->getPointerTo();
				}
			}
		}
		
		llvm::Type* genType(Module& module, const SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::OBJECT: {
					return genObjectType(module, type);
				}
				case SEM::Type::ALIAS: {
					return genType(module, type->resolveAliases());
				}
				default: {
					llvm_unreachable("Unknown type enum for generating type.");
				}
			}
		}
		
		llvm::DIType genDebugFunctionType(Module& module, SEM::FunctionType type) {
			// TODO!
			const auto file = module.debugBuilder().createFile("/object/dir/example_source_file.loci");
			
			std::vector<LLVMMetadataValue*> parameterTypes;
			parameterTypes.push_back(genDebugType(module, type.returnType()));
			
			for (const auto& paramType: type.parameterTypes()) {
				parameterTypes.push_back(genDebugType(module, paramType));
			}
			
			return module.debugBuilder().createFunctionType(file, parameterTypes);
		}
		
		llvm::DIType genObjectDebugType(Module& module, const SEM::Type* const type) {
			const auto objectType = type->getObjectType();
			const auto debugInfo = objectType->debugInfo();
			
			if (debugInfo) {
				const auto& typeInstanceInfo = *debugInfo;
				const auto& location = typeInstanceInfo.location;
				const auto file = module.debugBuilder().createFile(location.fileName());
				const auto lineNumber = location.range().start().lineNumber();
				return module.debugBuilder().createObjectType(file, lineNumber, objectType->name());
			} else {
				return module.debugBuilder().createUnspecifiedType(objectType->name().last());
			}
		}
		
		llvm::DIType genPrimitiveDebugType(Module& module, const SEM::Type* const type) {
			const auto objectType = type->getObjectType();
			
			const auto primitiveKind = module.primitiveKind(objectType->name().last());
			
			switch (primitiveKind) {
				case PrimitiveVoid:
					return module.debugBuilder().createVoidType();
				case PrimitiveNull:
					return module.debugBuilder().createNullType();
				case PrimitivePtr:
					return module.debugBuilder().createPointerType(genDebugType(module, type->templateArguments().front().typeRefType()));
				case PrimitiveInt:
					return module.debugBuilder().createIntType(module.getCString("int_t"));
				case PrimitiveRef:
					return module.debugBuilder().createReferenceType(genDebugType(module, type->templateArguments().front().typeRefType()));
				case PrimitiveFunctionPtr:
				case PrimitiveMethodFunctionPtr:
				case PrimitiveTemplatedFunctionPtr:
				case PrimitiveTemplatedMethodFunctionPtr:
				case PrimitiveVarArgFunctionPtr: {
					const auto functionType = type->asFunctionType();
					// TODO!
					const auto file = module.debugBuilder().createFile("/object/dir/example_source_file.loci");
					
					std::vector<LLVMMetadataValue*> parameterTypes;
					parameterTypes.push_back(genDebugType(module, functionType.returnType()));
					
					for (const auto paramType: functionType.parameterTypes()) {
						parameterTypes.push_back(genDebugType(module, paramType));
					}
					
					return module.debugBuilder().createFunctionType(file, parameterTypes);
				}
				case PrimitiveMethod:
				case PrimitiveTemplatedMethod: {
					// TODO!
					return module.debugBuilder().createUnspecifiedType(module.getCString("method"));
				}
				case PrimitiveInterfaceMethod: {
					// TODO!
					return module.debugBuilder().createUnspecifiedType(module.getCString("interfacemethod"));
				}
				case PrimitiveStaticInterfaceMethod: {
					// TODO!
					return module.debugBuilder().createUnspecifiedType(module.getCString("staticinterfacemethod"));
				}
				default: {
					return genObjectDebugType(module, type);
				}
			}
		}
		
		llvm::DIType genDebugType(Module& module, const SEM::Type* const type) {
			switch (type->kind()) {
				case SEM::Type::OBJECT: {
					if (type->isPrimitive()) {
						return genPrimitiveDebugType(module, type);
					}
					
					return genObjectDebugType(module, type);
				}
				case SEM::Type::TEMPLATEVAR: {
					const auto templateVar = type->getTemplateVar();
					const auto debugInfo = templateVar->debugInfo();
					
					if (debugInfo) {
						const auto& templateVarInfo = *debugInfo;
						const auto file = module.debugBuilder().createFile(templateVarInfo.declLocation.fileName());
						const auto lineNumber = templateVarInfo.declLocation.range().start().lineNumber();
						return module.debugBuilder().createObjectType(file, lineNumber, templateVar->name());
					} else {
						return module.debugBuilder().createUnspecifiedType(templateVar->name().last());
					}
				}
				case SEM::Type::ALIAS: {
					return genDebugType(module, type->resolveAliases());
				}
				default: {
					llvm_unreachable("Unknown type enum for generating type.");
				}
			}
		}
		
	}
	
}

