#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		llvm::Value* genStaticArrayPrimitiveMethodCall(Function& function,
		                                               const SEM::Type* const type,
		                                               const String& methodName,
		                                               PendingResultArray args,
		                                               llvm::Value* const hintResultValue) {
			auto& builder = function.getBuilder();
			auto& module = function.module();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			const auto elementType = type->templateArguments().front().typeRefType();
			const auto& elementCount = type->templateArguments().back();
			
			switch (methodID) {
				case METHOD_ALIGNMASK: {
					// Alignment of array is the same as alignment
					// of each element.
					return genAlignMask(function, elementType);
				}
				case METHOD_SIZEOF: {
					return builder.CreateMul(genSizeOf(function, elementType),
					                         genValue(function, elementCount));
				}
				case METHOD_UNINITIALIZED: {
					// TODO: set elements to dead state.
					if (isTypeSizeAlwaysKnown(module, type)) {
						return ConstantGenerator(module).getUndef(genType(module, type));
					} else {
						const auto result = genAlloca(function,
						                              type,
						                              hintResultValue);
						// TODO
						return result;
					}
				}
				case METHOD_COPY:
				case METHOD_IMPLICITCOPY: {
					// TODO: need to call implicit copy methods!
					return args[0].resolveWithoutBind(function);
				}
				case METHOD_ISVALID: {
					// TODO!
					(void) args[0].resolve(function);
					return ConstantGenerator(module).getI1(true);
				}
				case METHOD_SETINVALID: {
					// TODO!
					(void) args[0].resolve(function);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_ISLIVE: {
					// TODO!
					(void) args[0].resolve(function);
					return ConstantGenerator(module).getI1(true);
				}
				case METHOD_SETDEAD: {
					// TODO!
					(void) args[0].resolve(function);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_MOVETO: {
					// TODO!
					const auto methodOwner = args[0].resolveWithoutBind(function);
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
					const auto llvmType = genType(module, type);
					const auto castedDestPtr = builder.CreatePointerCast(destPtr, llvmType->getPointerTo());
					builder.CreateStore(methodOwner, castedDestPtr);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_INDEX: {
					const auto methodOwner = args[0].resolve(function);
					const auto operand = args[1].resolve(function);
					llvm::Value* const indexArray[] = {
							ConstantGenerator(module).getSizeTValue(0),
							operand
						};
					return builder.CreateInBoundsGEP(methodOwner, indexArray);
				}
				default:
					printf("%s\n", methodName.c_str());
					llvm_unreachable("Unknown static_array_t primitive method.");
			}
		}
		
	}
	
}

