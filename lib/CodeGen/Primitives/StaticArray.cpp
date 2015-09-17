#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		llvm::Value* getArrayIndex(Function& function,
		                           const SEM::Type* const type,
		                           const SEM::Type* const elementType,
		                           llvm::Value* const arrayPtr,
		                           llvm::Value* const elementIndex) {
			auto& builder = function.getBuilder();
			auto& module = function.module();
			
			if (isTypeSizeAlwaysKnown(module, type)) {
				llvm::Value* const indexArray[] = {
						ConstantGenerator(module).getSizeTValue(0),
						elementIndex
					};
				return builder.CreateInBoundsGEP(arrayPtr, indexArray);
			} else {
				const auto castMethodOwner = builder.CreatePointerCast(arrayPtr,
				                                                       TypeGenerator(module).getI8PtrType());
				const auto elementSize = genSizeOf(function,
				                                   elementType);
				const auto indexPos = builder.CreateMul(elementSize,
				                                        elementIndex);
				const auto elementPtr = builder.CreateInBoundsGEP(castMethodOwner,
				                                                  indexPos);
				return builder.CreatePointerCast(elementPtr,
				                                 genPointerType(module, elementType));
			}
		}
		
		llvm::Value* genStaticArrayPrimitiveMethodCall(Function& function,
		                                               const SEM::Type* const type,
		                                               const MethodID methodID,
		                                               PendingResultArray args,
		                                               llvm::Value* const hintResultValue) {
			auto& builder = function.getBuilder();
			auto& module = function.module();
			
			IREmitter irEmitter(function);
			
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
					const auto arraySize = genValue(function, elementCount);
					const auto arrayPtr = args[0].resolve(function);
					
					const auto result = irEmitter.emitAlloca(type,
					                                         hintResultValue);
					
					const auto beforeLoopBB = builder.GetInsertBlock();
					const auto loopBB = function.createBasicBlock("");
					const auto afterLoopBB = function.createBasicBlock("");
					
					builder.CreateBr(loopBB);
					
					function.selectBasicBlock(loopBB);
					
					const auto sizeTType = TypeGenerator(module).getSizeTType();
					
					const auto phiNode = builder.CreatePHI(sizeTType, 2);
					phiNode->addIncoming(ConstantGenerator(module).getSizeTValue(0),
					                     beforeLoopBB);
					
					const auto memberPtr = getArrayIndex(function,
					                                     type,
					                                     elementType,
					                                     arrayPtr,
					                                     phiNode);
					
					const auto resultPtr = getArrayIndex(function,
					                                     type,
					                                     elementType,
					                                     result,
					                                     phiNode);
					
					const auto copyResult = irEmitter.emitCopyCall(methodID,
					                                               memberPtr,
					                                               elementType,
					                                               resultPtr);
					
					irEmitter.emitMoveStore(copyResult,
					                        resultPtr,
					                        elementType);
					
					const auto indexIncremented = builder.CreateAdd(phiNode,
					                                                ConstantGenerator(module).getSizeTValue(1));
					
					phiNode->addIncoming(indexIncremented,
					                     loopBB);
					
					const auto isEnd = builder.CreateICmpEQ(indexIncremented,
					                                        arraySize);
					
					builder.CreateCondBr(isEnd,
					                     afterLoopBB,
					                     loopBB);
					
					function.selectBasicBlock(afterLoopBB);
					
					return irEmitter.emitMoveLoad(result,
					                              type);
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
					const auto arraySize = genValue(function, elementCount);
					const auto arrayPtr = args[0].resolve(function);
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					
					const auto result = builder.CreateInBoundsGEP(moveToPtr,
					                                              moveToPosition);
					
					const auto beforeLoopBB = builder.GetInsertBlock();
					const auto loopBB = function.createBasicBlock("");
					const auto afterLoopBB = function.createBasicBlock("");
					
					builder.CreateBr(loopBB);
					
					function.selectBasicBlock(loopBB);
					
					const auto sizeTType = TypeGenerator(module).getSizeTType();
					
					const auto phiNode = builder.CreatePHI(sizeTType, 2);
					phiNode->addIncoming(ConstantGenerator(module).getSizeTValue(0),
					                     beforeLoopBB);
					
					const auto memberPtr = getArrayIndex(function,
					                                     type,
					                                     elementType,
					                                     arrayPtr,
					                                     phiNode);
					
					const auto resultPtr = getArrayIndex(function,
					                                     type,
					                                     elementType,
					                                     result,
					                                     phiNode);
					
					const auto memberValue = irEmitter.emitMoveLoad(memberPtr,
					                                                elementType);
					
					irEmitter.emitMoveStore(memberValue,
					                        resultPtr,
					                        elementType);
					
					const auto indexIncremented = builder.CreateAdd(phiNode,
					                                                ConstantGenerator(module).getSizeTValue(1));
					
					phiNode->addIncoming(indexIncremented,
					                     loopBB);
					
					const auto isEnd = builder.CreateICmpEQ(indexIncremented,
					                                        arraySize);
					
					builder.CreateCondBr(isEnd,
					                     afterLoopBB,
					                     loopBB);
					
					function.selectBasicBlock(afterLoopBB);
					
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_INDEX: {
					const auto methodOwner = args[0].resolve(function);
					const auto operand = args[1].resolve(function);
					if (isTypeSizeAlwaysKnown(module, type)) {
						llvm::Value* const indexArray[] = {
								ConstantGenerator(module).getSizeTValue(0),
								operand
							};
						return builder.CreateInBoundsGEP(methodOwner, indexArray);
					} else {
						const auto castMethodOwner = builder.CreatePointerCast(methodOwner,
						                                                       TypeGenerator(module).getI8PtrType());
						const auto elementSize = genSizeOf(function,
						                                   elementType);
						const auto indexPos = builder.CreateMul(elementSize,
						                                        operand);
						const auto elementPtr = builder.CreateInBoundsGEP(castMethodOwner,
						                                                  indexPos);
						return builder.CreatePointerCast(elementPtr,
						                                 genPointerType(module, elementType));
					}
				}
				default:
					llvm_unreachable("Unknown static_array_t primitive method.");
			}
		}
		
	}
	
}

