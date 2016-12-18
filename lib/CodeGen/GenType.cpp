#include <string>
#include <stdexcept>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>

#include <locic/AST/Value.hpp>
#include <locic/AST/TemplateVar.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenType.hpp>
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
		
		llvm::Type* genType(Module& module, const SEM::Type* type) {
			const auto abiType = genABIType(module, type);
			return module.abi().typeInfo().getLLVMType(abiType);
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
				return module.debugBuilder().createUnspecifiedType(objectType->fullName().last());
			}
			
			const auto& typeInstanceInfo = *debugInfo;
			const auto& location = typeInstanceInfo.location;
			const auto file = module.debugBuilder().createFile(location.fileName().asStdString());
			const auto lineNumber = location.range().start().lineNumber();
			
			TypeInfo typeInfo(module);
			if (typeInfo.isSizeKnownInThisModule(type)) {
				const auto abiType = genABIType(module, type);
				const auto typeSize = module.abi().typeInfo().getTypeAllocSize(abiType);
				const auto typeAlign = module.abi().typeInfo().getTypeAllocSize(abiType);
				return module.debugBuilder().createObjectType(file,
				                                              lineNumber,
				                                              objectType->fullName(),
				                                              typeSize.asBits(),
				                                              typeAlign.asBits());
			} else {
				return module.debugBuilder().createUnspecifiedType(objectType->fullName().last());
			}
		}
		
		namespace {
			
			bool isVirtualnessKnown(const SEM::Type* const type) {
				// Virtual template variables may or may not be
				// instantiated with virtual types.
				return !type->isTemplateVar() ||
					!type->getTemplateVar()->isVirtual();
			}
			
			const SEM::Type* getRefTarget(const SEM::Type* const type) {
				const auto refTarget = type->templateArguments().at(0).typeRefType();
				return refTarget->resolveAliases();
			}
			
			bool isRefVirtualnessKnown(const SEM::Type* const type) {
				return isVirtualnessKnown(getRefTarget(type));
			}
			
			bool isRefVirtual(const SEM::Type* const type) {
				assert(isRefVirtualnessKnown(type));
				return getRefTarget(type)->isInterface();
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
				case PrimitiveRef: {
					if (isRefVirtualnessKnown(type)) {
						const auto targetDebugType = genDebugType(module,
						                                          type->templateArguments().front().typeRefType());
						if (isRefVirtual(type)) {
							// TODO?
							return module.debugBuilder().createUnspecifiedType(module.getCString("ref_t"));
						} else {
							return module.debugBuilder().createReferenceType(targetDebugType);
						}
					} else {
						return module.debugBuilder().createUnspecifiedType(module.getCString("ref_t"));
					}
				}
					return module.debugBuilder().createReferenceType(genDebugType(module, type->templateArguments().front().typeRefType()));
				CASE_CALLABLE_ID(PrimitiveFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveMethodFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveTemplatedFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveTemplatedMethodFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveVarArgFunctionPtr): {
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
				CASE_CALLABLE_ID(PrimitiveMethod):
				CASE_CALLABLE_ID(PrimitiveTemplatedMethod): {
					// TODO!
					return module.debugBuilder().createUnspecifiedType(module.getCString("method"));
				}
				CASE_CALLABLE_ID(PrimitiveInterfaceMethod): {
					// TODO!
					return module.debugBuilder().createUnspecifiedType(module.getCString("interfacemethod"));
				}
				CASE_CALLABLE_ID(PrimitiveStaticInterfaceMethod): {
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
					return module.debugBuilder().createUnspecifiedType(templateVar->fullName().last());
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

