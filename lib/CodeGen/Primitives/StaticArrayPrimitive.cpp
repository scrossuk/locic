#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/Constant.hpp>

#include <locic/AST/Type.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
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
#include <locic/CodeGen/ValueEmitter.hpp>
#include <locic/CodeGen/VTable.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		StaticArrayPrimitive::StaticArrayPrimitive(const AST::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) { }
		
		bool StaticArrayPrimitive::isSizeAlwaysKnown(const TypeInfo& typeInfo,
		                                             llvm::ArrayRef<AST::Value> templateArguments) const {
			assert(templateArguments.size() == 2);
			const auto targetType = templateArguments.front().typeRefType();
			const auto& elementCountValue = templateArguments.back();
			return typeInfo.isSizeAlwaysKnown(targetType) &&
			       elementCountValue.isConstant();
		}
		
		bool StaticArrayPrimitive::isSizeKnownInThisModule(const TypeInfo& typeInfo,
		                                                   llvm::ArrayRef<AST::Value> templateArguments) const {
			assert(templateArguments.size() == 2);
			const auto targetType = templateArguments.front().typeRefType();
			const auto& elementCountValue = templateArguments.back();
			return typeInfo.isSizeKnownInThisModule(targetType) &&
			       elementCountValue.isConstant();
		}
		
		bool StaticArrayPrimitive::hasCustomDestructor(const TypeInfo& typeInfo,
		                                               llvm::ArrayRef<AST::Value> templateArguments) const {
			return typeInfo.hasCustomDestructor(templateArguments.front().typeRefType());
		}
		
		bool StaticArrayPrimitive::hasCustomMove(const TypeInfo& typeInfo,
		                                         llvm::ArrayRef<AST::Value> templateArguments) const {
			return typeInfo.hasCustomMove(templateArguments.front().typeRefType());
		}
		
		llvm_abi::Type StaticArrayPrimitive::getABIType(Module& module,
		                                                 const llvm_abi::TypeBuilder& abiTypeBuilder,
		                                                 llvm::ArrayRef<AST::Value> templateArguments) const {
			const auto elementType = genABIType(module, templateArguments.front().typeRefType());
			const auto& elementCount = templateArguments.back().constant().integerValue();
			return abiTypeBuilder.getArrayTy(elementCount.asUint64(),
			                                 elementType);
		}
		
		namespace {
			
			llvm::Value* getArrayIndex(IREmitter& irEmitter,
			                           const AST::Type* const elementType,
			                           llvm::Value* const arrayPtr,
			                           llvm::Value* const elementIndex) {
				auto& builder = irEmitter.builder();
				auto& module = irEmitter.module();
				
				TypeInfo typeInfo(module);
				if (typeInfo.isSizeAlwaysKnown(elementType)) {
					return irEmitter.emitInBoundsGEP(genType(module, elementType),
					                                 arrayPtr,
					                                 elementIndex);
				} else {
					const auto elementSize = genSizeOf(irEmitter.function(),
					                                   elementType);
					const auto indexPos = builder.CreateMul(elementSize,
					                                        elementIndex);
					return irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
					                                 arrayPtr,
					                                 indexPos);
				}
			}
			
		}
		
		llvm::Value* StaticArrayPrimitive::emitMethod(IREmitter& irEmitter,
		                                              const MethodID methodID,
		                                              llvm::ArrayRef<AST::Value> typeTemplateArguments,
		                                              llvm::ArrayRef<AST::Value> /*functionTemplateArguments*/,
		                                              PendingResultArray args,
		                                              llvm::Value* const resultPtr) const {
			auto& builder = irEmitter.builder();
			auto& function = irEmitter.function();
			auto& module = irEmitter.module();
			
			AST::ValueArray valueArray;
			for (const auto& value: typeTemplateArguments) {
				valueArray.push_back(value.copy());
			}
			const auto type = AST::Type::Object(&typeInstance_,
			                                    std::move(valueArray));
			const auto elementType = typeTemplateArguments.front().typeRefType();
			const auto& elementCount = typeTemplateArguments.back();
			
			ValueEmitter valueEmitter(irEmitter);
			
			switch (methodID) {
				case METHOD_ALIGNMASK: {
					// Alignment of array is the same as alignment
					// of each element.
					return genAlignMask(function, elementType);
				}
				case METHOD_SIZEOF: {
					return builder.CreateMul(genSizeOf(function, elementType),
					                         valueEmitter.emitValue(elementCount));
				}
				case METHOD_UNINITIALIZED: {
					// TODO: set elements to dead state.
					TypeInfo typeInfo(module);
					if (typeInfo.isSizeAlwaysKnown(type)) {
						return ConstantGenerator(module).getUndef(genType(module, type));
					} else {
						const auto result = irEmitter.emitAlloca(type, resultPtr);
						// TODO
						return result;
					}
				}
				case METHOD_DESTROY: {
					const auto arraySize = valueEmitter.emitValue(elementCount);
					const auto arrayPtr = args[0].resolve(function);
					
					const auto beforeLoopBB = builder.GetInsertBlock();
					const auto loopBB = irEmitter.createBasicBlock("");
					const auto afterLoopBB = irEmitter.createBasicBlock("");
					
					irEmitter.emitBranch(loopBB);
					
					irEmitter.selectBasicBlock(loopBB);
					
					const auto sizeTType = TypeGenerator(module).getSizeTType();
					
					const auto phiNode = builder.CreatePHI(sizeTType, 2);
					phiNode->addIncoming(ConstantGenerator(module).getSizeTValue(0),
					                     beforeLoopBB);
					
					const auto reverseIndexPlusOne = builder.CreateSub(arraySize, phiNode);
					const auto oneValue = ConstantGenerator(module).getSizeTValue(1);
					const auto reverseIndex = builder.CreateSub(reverseIndexPlusOne, oneValue);
					
					const auto memberPtr = getArrayIndex(irEmitter,
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
					
					irEmitter.emitCondBranch(isEnd, afterLoopBB,
					                         loopBB);
					
					irEmitter.selectBasicBlock(afterLoopBB);
					
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_COPY:
				case METHOD_IMPLICITCOPY: {
					const auto arraySize = valueEmitter.emitValue(elementCount);
					const auto arrayPtr = args[0].resolve(function);
					
					const auto result = irEmitter.emitAlloca(type, resultPtr);
					
					const auto beforeLoopBB = builder.GetInsertBlock();
					const auto loopBB = irEmitter.createBasicBlock("");
					const auto afterLoopBB = irEmitter.createBasicBlock("");
					
					irEmitter.emitBranch(loopBB);
					
					irEmitter.selectBasicBlock(loopBB);
					
					const auto sizeTType = TypeGenerator(module).getSizeTType();
					
					const auto phiNode = builder.CreatePHI(sizeTType, 2);
					phiNode->addIncoming(ConstantGenerator(module).getSizeTValue(0),
					                     beforeLoopBB);
					
					const auto memberSourcePtr = getArrayIndex(irEmitter,
					                                           elementType,
					                                           arrayPtr,
					                                           phiNode);
					
					const auto memberDestPtr = getArrayIndex(irEmitter,
					                                         elementType,
					                                         result,
					                                         phiNode);
					
					const auto copyResult = irEmitter.emitCopyCall(methodID,
					                                               memberSourcePtr,
					                                               elementType,
					                                               memberDestPtr);
					irEmitter.emitStore(copyResult, memberDestPtr, elementType);
					
					const auto indexIncremented = builder.CreateAdd(phiNode,
					                                                ConstantGenerator(module).getSizeTValue(1));
					
					phiNode->addIncoming(indexIncremented,
					                     loopBB);
					
					const auto isEnd = builder.CreateICmpEQ(indexIncremented,
					                                        arraySize);
					
					irEmitter.emitCondBranch(isEnd, afterLoopBB,
					                         loopBB);
					
					irEmitter.selectBasicBlock(afterLoopBB);
					
					return irEmitter.emitLoad(result, type);
				}
				case METHOD_ISLIVE: {
					(void) args[0].resolve(function);
					return ConstantGenerator(module).getBool(true);
				}
				case METHOD_SETDEAD: {
					(void) args[0].resolve(function);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_MOVE: {
					const auto arraySize = valueEmitter.emitValue(elementCount);
					const auto sourcePtr = args[0].resolve(function);
					const auto destPtr = irEmitter.emitAlloca(type, resultPtr);
					
					const auto beforeLoopBB = builder.GetInsertBlock();
					const auto loopBB = irEmitter.createBasicBlock("");
					const auto afterLoopBB = irEmitter.createBasicBlock("");
					
					irEmitter.emitBranch(loopBB);
					
					irEmitter.selectBasicBlock(loopBB);
					
					const auto sizeTType = TypeGenerator(module).getSizeTType();
					
					const auto phiNode = builder.CreatePHI(sizeTType, 2);
					phiNode->addIncoming(ConstantGenerator(module).getSizeTValue(0),
					                     beforeLoopBB);
					
					const auto sourceMemberPtr = getArrayIndex(irEmitter,
					                                     elementType,
					                                     sourcePtr,
					                                     phiNode);
					
					const auto destMemberPtr = getArrayIndex(irEmitter,
					                                     elementType,
					                                     destPtr,
					                                     phiNode);
					
					irEmitter.emitMove(sourceMemberPtr, destMemberPtr, elementType);
					
					const auto indexIncremented = builder.CreateAdd(phiNode,
					                                                ConstantGenerator(module).getSizeTValue(1));
					
					phiNode->addIncoming(indexIncremented,
					                     irEmitter.getBasicBlock());
					
					const auto isEnd = builder.CreateICmpEQ(indexIncremented,
					                                        arraySize);
					
					irEmitter.emitCondBranch(isEnd, afterLoopBB,
					                         loopBB);
					
					irEmitter.selectBasicBlock(afterLoopBB);
					
					return irEmitter.emitLoad(destPtr, type);
				}
				case METHOD_INDEX: {
					const auto methodOwner = args[0].resolve(function);
					const auto operand = args[1].resolve(function);
					TypeInfo typeInfo(module);
					if (typeInfo.isSizeAlwaysKnown(elementType)) {
						return irEmitter.emitInBoundsGEP(genType(module, elementType),
						                                 methodOwner,
						                                 operand);
					} else {
						const auto elementSize = genSizeOf(function,
						                                   elementType);
						const auto indexPos = builder.CreateMul(elementSize,
						                                        operand);
						const auto elementPtr = irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
						                                                  methodOwner,
						                                                  indexPos);
						return elementPtr;
					}
				}
				default:
					llvm_unreachable("Unknown static_array_t primitive method.");
			}
		}
		
	}
	
}

