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
#include <locic/CodeGen/TypeInfo.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Type* genArgType(Module& module, const SEM::Type* type) {
			if (canPassByValue(module, type)) {
				return genType(module, type);
			} else {
				return TypeGenerator(module).getPtrType();
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
		
		DISubroutineType genDebugFunctionType(Module& module, SEM::FunctionType type) {
			// TODO!
			const auto file = module.debugBuilder().createFile("/object/dir/example_source_file.loci");
			
			std::vector<LLVMMetadataValue*> parameterTypes;
			parameterTypes.push_back(genDebugType(module, type.returnType()));
			
			for (const auto& paramType: type.parameterTypes()) {
				parameterTypes.push_back(genDebugType(module, paramType));
			}
			
			return module.debugBuilder().createFunctionType(file, parameterTypes);
		}
		
		DIType genObjectDebugType(Module& module, const SEM::Type* const type) {
			const auto objectType = type->getObjectType();
			const auto debugInfo = objectType->debugInfo();
			
			if (!debugInfo) {
				return module.debugBuilder().createUnspecifiedType(objectType->name().last());
			}
			
			const auto& typeInstanceInfo = *debugInfo;
			const auto& location = typeInstanceInfo.location;
			const auto file = module.debugBuilder().createFile(location.fileName());
			const auto lineNumber = location.range().start().lineNumber();
			
			TypeInfo typeInfo(module);
			if (typeInfo.isSizeKnownInThisModule(type)) {
				const auto abiType = genABIType(module, type);
				const auto typeSizeBytes = module.abi().typeSize(abiType);
				const auto typeAlignBytes = module.abi().typeAlign(abiType);
				return module.debugBuilder().createObjectType(file,
				                                              lineNumber,
				                                              objectType->name(),
				                                              typeSizeBytes*8,
				                                              typeAlignBytes*8);
			} else {
				return module.debugBuilder().createUnspecifiedType(objectType->name().last());
			}
		}
		
		DIType genPrimitiveDebugType(Module& module, const SEM::Type* const type) {
			switch (type->primitiveID()) {
				case PrimitiveVoid:
					return module.debugBuilder().createVoidType();
				case PrimitiveNull:
					return module.debugBuilder().createNullType();
				case PrimitivePtr:
					return module.debugBuilder().createPointerType(genDebugType(module, type->templateArguments().front().typeRefType()));
				case PrimitiveInt:
					// TODO: Add other integer types.
					return module.debugBuilder().createIntType(PrimitiveInt);
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
		
		DIType genDebugType(Module& module, const SEM::Type* const type) {
			switch (type->kind()) {
				case SEM::Type::OBJECT: {
					if (type->isPrimitive()) {
						return genPrimitiveDebugType(module, type);
					}
					
					return genObjectDebugType(module, type);
				}
				case SEM::Type::TEMPLATEVAR: {
					const auto templateVar = type->getTemplateVar();
					return module.debugBuilder().createUnspecifiedType(templateVar->name().last());
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

