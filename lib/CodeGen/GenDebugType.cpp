#include <string>
#include <stdexcept>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>

#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/TemplateVar.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenDebugType.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Primitives/RefPrimitive.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>

namespace locic {
	
	namespace CodeGen {
		
		DISubroutineType genDebugFunctionType(Module& module, AST::FunctionType type) {
			std::vector<LLVMMetadataValue*> parameterTypes;
			parameterTypes.push_back(genDebugType(module, type.returnType()));
			
			for (const auto& paramType: type.parameterTypes()) {
				parameterTypes.push_back(genDebugType(module, paramType));
			}
			
			return module.debugBuilder().createFunctionType(parameterTypes);
		}
		
		DIType genObjectDebugType(Module& module, const AST::Type* const type) {
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
		
		DIType genPrimitiveDebugType(Module& module, const AST::Type* const type) {
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
					RefPrimitive refPrimitive(*(type->getObjectType()));
					const auto targetType = type->templateArguments()[0].typeRefType();
					if (refPrimitive.isAbstractnessKnown(targetType)) {
						const auto targetDebugType = genDebugType(module, targetType);
						if (refPrimitive.isAbstract(targetType)) {
							// TODO?
							return module.debugBuilder().createUnspecifiedType(module.getCString("ref_t"));
						} else {
							return module.debugBuilder().createReferenceType(targetDebugType);
						}
					} else {
						return module.debugBuilder().createUnspecifiedType(module.getCString("ref_t"));
					}
				}
				CASE_CALLABLE_ID(PrimitiveFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveMethodFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveTemplatedFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveTemplatedMethodFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveVarArgFunctionPtr): {
					const auto functionType = type->asFunctionType();
					return module.debugBuilder().createPointerType(genDebugFunctionType(module, functionType));
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
		
		DIType genDebugType(Module& module, const AST::Type* const type) {
			switch (type->kind()) {
				case AST::Type::OBJECT: {
					if (type->isPrimitive()) {
						return genPrimitiveDebugType(module, type);
					}
					
					return genObjectDebugType(module, type);
				}
				case AST::Type::TEMPLATEVAR: {
					const auto templateVar = type->getTemplateVar();
					return module.debugBuilder().createUnspecifiedType(templateVar->fullName().last());
				}
				case AST::Type::ALIAS: {
					return genDebugType(module, type->resolveAliases());
				}
				default: {
					llvm_unreachable("Unknown type enum for generating type.");
				}
			}
		}
		
	}
	
}

