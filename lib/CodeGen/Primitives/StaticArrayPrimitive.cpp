#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Primitives/StaticArrayPrimitive.hpp>
#include <locic/CodeGen/Routines.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/VTable.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		StaticArrayPrimitive::StaticArrayPrimitive(const SEM::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) { }
		
		bool StaticArrayPrimitive::isSizeAlwaysKnown(const TypeInfo& typeInfo,
		                                             llvm::ArrayRef<SEM::Value> templateArguments) const {
			assert(templateArguments.size() == 2);
			const auto targetType = templateArguments.front().typeRefType();
			const auto& elementCountValue = templateArguments.back();
			return typeInfo.isSizeAlwaysKnown(targetType) &&
			       elementCountValue.isConstant();
		}
		
		bool StaticArrayPrimitive::isSizeKnownInThisModule(const TypeInfo& typeInfo,
		                                                   llvm::ArrayRef<SEM::Value> templateArguments) const {
			assert(templateArguments.size() == 2);
			const auto targetType = templateArguments.front().typeRefType();
			const auto& elementCountValue = templateArguments.back();
			return typeInfo.isSizeKnownInThisModule(targetType) &&
			       elementCountValue.isConstant();
		}
		
		bool StaticArrayPrimitive::hasCustomDestructor(const TypeInfo& typeInfo,
		                                               llvm::ArrayRef<SEM::Value> templateArguments) const {
			return typeInfo.hasCustomDestructor(templateArguments.front().typeRefType());
		}
		
		bool StaticArrayPrimitive::hasCustomMove(const TypeInfo& typeInfo,
		                                         llvm::ArrayRef<SEM::Value> templateArguments) const {
			return typeInfo.hasCustomMove(templateArguments.front().typeRefType());
		}
		
		llvm_abi::Type* StaticArrayPrimitive::getABIType(Module& module,
		                                                 llvm_abi::Context& abiContext,
		                                                 llvm::ArrayRef<SEM::Value> templateArguments) const {
			const auto elementType = genABIType(module, templateArguments.front().typeRefType());
			const auto elementCount = templateArguments.back().constant().integerValue();
			return llvm_abi::Type::Array(abiContext,
			                             elementCount,
			                             elementType);
		}
		
		llvm::Type* StaticArrayPrimitive::getIRType(Module& module,
		                                            const TypeGenerator& typeGenerator,
		                                            llvm::ArrayRef<SEM::Value> templateArguments) const {
			const auto elementType = genType(module, templateArguments.front().typeRefType());
			const auto elementCount = templateArguments.back().constant().integerValue();
			return typeGenerator.getArrayType(elementType,
			                                  elementCount);
		}
		
		namespace {
			
			llvm::Value* getArrayIndex(Function& function,
			                           const SEM::Type* const type,
			                           const SEM::Type* const elementType,
			                           llvm::Value* const arrayPtr,
			                           llvm::Value* const elementIndex) {
				auto& builder = function.getBuilder();
				auto& module = function.module();
				
				TypeInfo typeInfo(module);
				if (typeInfo.isSizeAlwaysKnown(type)) {
					llvm::Value* const indexArray[] = {
							ConstantGenerator(module).getSizeTValue(0),
							elementIndex
						};
					return builder.CreateInBoundsGEP(arrayPtr, indexArray);
				} else {
					const auto castMethodOwner = builder.CreatePointerCast(arrayPtr,
					                                                       TypeGenerator(module).getPtrType());
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
			
		}
		
		llvm::Value* StaticArrayPrimitive::emitMethod(IREmitter& irEmitter,
		                                              const MethodID methodID,
		                                              llvm::ArrayRef<SEM::Value> typeTemplateArguments,
		                                              llvm::ArrayRef<SEM::Value> /*functionTemplateArguments*/,
		                                              PendingResultArray args) const {
			auto& builder = irEmitter.builder();
			auto& function = irEmitter.function();
			auto& module = irEmitter.module();
			
			SEM::ValueArray valueArray;
			for (const auto& value: typeTemplateArguments) {
				valueArray.push_back(value.copy());
			}
			const auto type = SEM::Type::Object(&typeInstance_,
			                                    std::move(valueArray));
			const auto elementType = typeTemplateArguments.front().typeRefType();
			const auto& elementCount = typeTemplateArguments.back();
			
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
					TypeInfo typeInfo(module);
					if (typeInfo.isSizeAlwaysKnown(type)) {
						return ConstantGenerator(module).getUndef(genType(module, type));
					} else {
						const auto result = irEmitter.emitReturnAlloca(type);
						// TODO
						return result;
					}
				}
				case METHOD_DESTROY: {
					const auto arraySize = genValue(function, elementCount);
					const auto arrayPtr = args[0].resolve(function);
					
					const auto beforeLoopBB = builder.GetInsertBlock();
					const auto loopBB = function.createBasicBlock("");
					const auto afterLoopBB = function.createBasicBlock("");
					
					builder.CreateBr(loopBB);
					
					function.selectBasicBlock(loopBB);
					
					const auto sizeTType = TypeGenerator(module).getSizeTType();
					
					const auto phiNode = builder.CreatePHI(sizeTType, 2);
					phiNode->addIncoming(ConstantGenerator(module).getSizeTValue(0),
					                     beforeLoopBB);
					
					const auto reverseIndexPlusOne = builder.CreateSub(arraySize, phiNode);
					const auto oneValue = ConstantGenerator(module).getSizeTValue(1);
					const auto reverseIndex = builder.CreateSub(reverseIndexPlusOne, oneValue);
					
					const auto memberPtr = getArrayIndex(function,
					                                     type,
					                                     elementType,
					                                     arrayPtr,
					                                     reverseIndex);
					
					irEmitter.emitDestructorCall(memberPtr,
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
				case METHOD_COPY:
				case METHOD_IMPLICITCOPY: {
					const auto arraySize = genValue(function, elementCount);
					const auto arrayPtr = args[0].resolve(function);
					
					const auto result = irEmitter.emitReturnAlloca(type);
					
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
				case METHOD_ISLIVE: {
					(void) args[0].resolve(function);
					return ConstantGenerator(module).getI1(true);
				}
				case METHOD_SETDEAD: {
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
					TypeInfo typeInfo(module);
					if (typeInfo.isSizeAlwaysKnown(type)) {
						llvm::Value* const indexArray[] = {
								ConstantGenerator(module).getSizeTValue(0),
								operand
							};
						return builder.CreateInBoundsGEP(methodOwner, indexArray);
					} else {
						const auto castMethodOwner = builder.CreatePointerCast(methodOwner,
						                                                       TypeGenerator(module).getPtrType());
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

