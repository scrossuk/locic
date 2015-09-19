#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/DefaultMethodEmitter.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/LivenessIndicator.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

#include <locic/SEM/Function.hpp>
#include <locic/SEM/FunctionType.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		DefaultMethodEmitter::DefaultMethodEmitter(Function& functionGenerator)
		: functionGenerator_(functionGenerator) { }
		
		llvm::Value*
		DefaultMethodEmitter::emitMethod(const MethodID methodID,
		                                 const bool isInnerMethod,
		                                 const SEM::Type* const type,
		                                 const SEM::FunctionType functionType,
		                                 PendingResultArray args,
		                                 llvm::Value* const hintResultValue) {
			if (isInnerMethod) {
				if (methodID == METHOD_DESTROY) {
					return emitInnerDestroy(type,
					                        functionType,
					                        std::move(args));
				} else {
					assert(methodID == METHOD_MOVETO);
					return emitInnerMoveTo(type,
					                       functionType,
					                       std::move(args));
				}
			}
			
			switch (methodID) {
				case METHOD_CREATE:
					return emitCreateConstructor(type,
					                             functionType,
					                             std::move(args),
					                             hintResultValue);
				case METHOD_DESTROY:
					return emitOuterDestroy(type,
					                        functionType,
					                        std::move(args));
				case METHOD_MOVETO:
					return emitOuterMoveTo(type,
					                       functionType,
					                       std::move(args));
				case METHOD_ALIGNMASK:
					return emitAlignMask(type);
				case METHOD_SIZEOF:
					return emitSizeOf(type);
				case METHOD_ISLIVE:
					return emitIsLive(type,
					                  functionType,
					                  std::move(args));
				case METHOD_SETDEAD:
					return emitSetDead(type,
					                   functionType,
					                   std::move(args));
				case METHOD_IMPLICITCOPY:
					return emitImplicitCopy(type,
					                        functionType,
					                        std::move(args),
					                        hintResultValue);
				case METHOD_COPY:
					return emitExplicitCopy(type,
					                        functionType,
					                        std::move(args),
					                        hintResultValue);
				case METHOD_COMPARE:
					return emitCompare(type,
					                   functionType,
					                   std::move(args));
				default:
					llvm_unreachable("Unknown default function.");
			}
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitCreateConstructor(const SEM::Type* const type,
		                                            const SEM::FunctionType /*functionType*/,
		                                            PendingResultArray args,
		                                            llvm::Value* const hintResultValue) {
			const auto& typeInstance = *(type->getObjectType());
			assert(!typeInstance.isUnionDatatype());
			
			auto& module = functionGenerator_.module();
			
			if (typeInstance.isUnion()) {
				assert(hintResultValue == nullptr);
				return ConstantGenerator(module).getNull(genType(module, type));
			}
			
			IREmitter irEmitter(functionGenerator_, hintResultValue);
			
			const auto resultValue = irEmitter.emitReturnAlloca(type);
			
			for (size_t i = 0; i < typeInstance.variables().size(); i++) {
				const auto& memberVar = typeInstance.variables()[i];
				const size_t memberIndex = module.getMemberVarMap().at(memberVar);
				
				const auto memberType = memberVar->constructType()->resolveAliases();
				
				const auto resultPtr = genMemberPtr(functionGenerator_, resultValue, type, memberIndex);
				
				irEmitter.emitMoveStore(args[i].resolve(functionGenerator_),
				                        resultPtr,
				                        memberType);
			}
			
			// Set object into live state (e.g. set gap byte to 1).
			setOuterLiveState(functionGenerator_,
			                  typeInstance,
			                  resultValue);
			
			return irEmitter.emitMoveLoad(resultValue, type);
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitOuterDestroy(const SEM::Type* const type,
		                                       const SEM::FunctionType /*functionType*/,
		                                       PendingResultArray args) {
			const auto& typeInstance = *(type->getObjectType());
			auto& module = functionGenerator_.module();
			auto& builder = functionGenerator_.getBuilder();
			
			IREmitter irEmitter(functionGenerator_);
			
			const auto thisValue = args[0].resolve(functionGenerator_);
			
			if (typeInstance.isUnionDatatype()) {
				const auto loadedTag = irEmitter.emitLoadDatatypeTag(thisValue);
				
				const auto endBB = functionGenerator_.createBasicBlock("end");
				const auto switchInstruction = builder.CreateSwitch(loadedTag, endBB, typeInstance.variants().size());
				
				// Start from 1 so that 0 can represent 'empty'.
				uint8_t tag = 1;
				
				for (const auto variantTypeInstance: typeInstance.variants()) {
					const auto matchBB = functionGenerator_.createBasicBlock("tagMatch");
					const auto tagValue = ConstantGenerator(module).getI8(tag++);
					
					switchInstruction->addCase(tagValue, matchBB);
					
					functionGenerator_.selectBasicBlock(matchBB);
					
					const auto variantType = variantTypeInstance->selfType();
					
					const auto unionValuePtr = irEmitter.emitGetDatatypeVariantPtr(thisValue,
					                                                               type,
					                                                               variantType);
					
					irEmitter.emitDestructorCall(unionValuePtr, variantType);
					
					builder.CreateBr(endBB);
				}
				
				functionGenerator_.selectBasicBlock(endBB);
			} else {
				const auto isLiveBB = functionGenerator_.createBasicBlock("is_live");
				const auto endBB = functionGenerator_.createBasicBlock("");
				
				// Check whether this object is in a 'live' state and only
				// run the destructor if it is.
				const auto isLive = genIsLive(functionGenerator_, typeInstance.selfType(), thisValue);
				builder.CreateCondBr(isLive, isLiveBB, endBB);
				
				functionGenerator_.selectBasicBlock(isLiveBB);
				
				// Call the custom destructor function, if one exists.
				const auto& function = *(typeInstance.functions().at(module.getCString("__destroy")));
				
				auto& semFunctionGenerator = module.semFunctionGenerator();
				
				const auto customDestructor = semFunctionGenerator.genDef(&typeInstance,
				                                                          function,
				                                                          /*isInnerMethod=*/true);
				
				const auto i8PtrType = TypeGenerator(module).getI8PtrType();
				const auto castThisValue = builder.CreatePointerCast(thisValue, i8PtrType);
				
				const auto argInfo = destructorArgInfo(module, typeInstance);
				const auto callArgs = argInfo.hasTemplateGeneratorArgument() ?
							std::vector<llvm::Value*> { functionGenerator_.getTemplateGenerator(), castThisValue } :
							std::vector<llvm::Value*> { castThisValue };
				(void) genRawFunctionCall(functionGenerator_, argInfo, customDestructor, callArgs);
				
				const auto& memberVars = typeInstance.variables();
				
				// Call destructors for all objects within the
				// parent object, in *REVERSE* order.
				for (size_t i = 0; i < memberVars.size(); i++) {
					const auto memberVar = memberVars.at((memberVars.size() - 1) - i);
					const size_t memberIndex = module.getMemberVarMap().at(memberVar);
					const auto memberOffsetValue = genMemberOffset(functionGenerator_, typeInstance.selfType(), memberIndex);
					const auto ptrToMember = builder.CreateInBoundsGEP(castThisValue, memberOffsetValue);
					irEmitter.emitDestructorCall(ptrToMember, memberVar->type());
				}
				
				// Put the object into a dead state.
				genSetDeadState(functionGenerator_, typeInstance.selfType(), thisValue);
				
				builder.CreateBr(endBB);
				
				functionGenerator_.selectBasicBlock(endBB);
			}
			
			return ConstantGenerator(module).getVoidUndef();
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitInnerDestroy(const SEM::Type* const /*type*/,
		                                       const SEM::FunctionType /*functionType*/,
		                                       PendingResultArray args) {
			// Default destroy code doesn't do anything.
			auto& module = functionGenerator_.module();
			(void) args[0].resolve(functionGenerator_);
			return ConstantGenerator(module).getVoidUndef();
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitOuterMoveTo(const SEM::Type* const type,
		                                      const SEM::FunctionType /*functionType*/,
		                                      PendingResultArray args) {
			const auto& typeInstance = *(type->getObjectType());
			auto& module = functionGenerator_.module();
			
			const auto destValue = args[1].resolve(functionGenerator_);
			const auto positionValue = args[2].resolve(functionGenerator_);
			const auto sourceValue = args[0].resolve(functionGenerator_);
			
			const auto livenessIndicator = getLivenessIndicator(module,
			                                                    typeInstance);
			
			if (livenessIndicator.isNone()) {
				// No liveness indicator so just move the member values.
				genCallUserMoveFunction(functionGenerator_,
				                        typeInstance,
				                        sourceValue,
				                        destValue,
				                        positionValue);
			} else {
				auto& builder = functionGenerator_.getBuilder();
				
				const auto destPtr = builder.CreateInBoundsGEP(destValue,
				                                               positionValue);
				const auto castedDestPtr = builder.CreatePointerCast(destPtr,
				                                                     genPointerType(module, type));
				
				const auto isLiveBB = functionGenerator_.createBasicBlock("is_live");
				const auto isNotLiveBB = functionGenerator_.createBasicBlock("is_not_live");
				const auto mergeBB = functionGenerator_.createBasicBlock("");
				
				// Check whether the source object is in a 'live' state and
				// only perform the move if it is.
				const auto isLive = genIsLive(functionGenerator_,
				                              type,
				                              sourceValue);
				builder.CreateCondBr(isLive,
				                     isLiveBB,
				                     isNotLiveBB);
				
				functionGenerator_.selectBasicBlock(isLiveBB);
				
				// Move member values.
				genCallUserMoveFunction(functionGenerator_,
				                        typeInstance,
				                        sourceValue,
				                        destValue,
				                        positionValue);
				
				// Set dest object to be valid (e.g. may need to set gap byte to 1).
				setOuterLiveState(functionGenerator_,
				                  typeInstance,
				                  castedDestPtr);
				
				// Set the source object to dead state.
				genSetDeadState(functionGenerator_,
				                type,
				                sourceValue);
				
				builder.CreateBr(mergeBB);
				
				functionGenerator_.selectBasicBlock(isNotLiveBB);
				
				// If the source object is dead, set destination to be dead.
				genSetDeadState(functionGenerator_,
				                type,
				                castedDestPtr);
				
				builder.CreateBr(mergeBB);
				
				functionGenerator_.selectBasicBlock(mergeBB);
			}
			
			return ConstantGenerator(module).getVoidUndef();
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitInnerMoveTo(const SEM::Type* const type,
		                                      const SEM::FunctionType /*functionType*/,
		                                      PendingResultArray args) {
			auto& module = functionGenerator_.module();
			auto& builder = functionGenerator_.getBuilder();
			const auto& typeInstance = *(type->getObjectType());
			
			const auto destValue = args[1].resolve(functionGenerator_);
			const auto positionValue = args[2].resolve(functionGenerator_);
			const auto sourceValue = args[0].resolve(functionGenerator_);
			
			if (typeInstance.isUnion()) {
				// Basically just do a memcpy.
				genBasicMove(functionGenerator_, type, sourceValue, destValue, positionValue);
			} else if (typeInstance.isUnionDatatype()) {
				const auto unionDatatypePointers = getUnionDatatypePointers(functionGenerator_,
				                                                            type,
				                                                            sourceValue);
				const auto loadedTag = builder.CreateLoad(unionDatatypePointers.first);
				
				// Store tag.
				builder.CreateStore(loadedTag, makeRawMoveDest(functionGenerator_, destValue, positionValue));
				
				// Set previous tag to zero.
				builder.CreateStore(ConstantGenerator(module).getI8(0), unionDatatypePointers.first);
				
				// Offset of union datatype data is equivalent to its alignment size.
				const auto unionDataOffset = genAlignOf(functionGenerator_, type);
				const auto adjustedPositionValue = builder.CreateAdd(positionValue, unionDataOffset);
				
				const auto endBB = functionGenerator_.createBasicBlock("end");
				const auto switchInstruction = builder.CreateSwitch(loadedTag, endBB, typeInstance.variants().size());
				
				// Start from 1 so that 0 can represent 'empty'.
				uint8_t tag = 1;
				
				for (const auto& variantTypeInstance: typeInstance.variants()) {
					const auto matchBB = functionGenerator_.createBasicBlock("tagMatch");
					const auto tagValue = ConstantGenerator(module).getI8(tag++);
					
					switchInstruction->addCase(tagValue, matchBB);
					
					functionGenerator_.selectBasicBlock(matchBB);
					
					const auto variantType = variantTypeInstance->selfType();
					const auto unionValueType = genType(module, variantType);
					const auto castedUnionValuePtr = builder.CreatePointerCast(unionDatatypePointers.second,
					                                                           unionValueType->getPointerTo());
					
					genMoveCall(functionGenerator_,
					            variantType,
					            castedUnionValuePtr,
					            destValue,
					            adjustedPositionValue);
					
					builder.CreateBr(endBB);
				}
				
				functionGenerator_.selectBasicBlock(endBB);
			} else {
				// Move member variables.
				for (const auto& memberVar: typeInstance.variables()) {
					const size_t memberIndex = module.getMemberVarMap().at(memberVar);
					const auto ptrToMember = genMemberPtr(functionGenerator_, sourceValue, type, memberIndex);
					const auto memberOffsetValue = genMemberOffset(functionGenerator_, type, memberIndex);
					const auto adjustedPositionValue = builder.CreateAdd(positionValue, memberOffsetValue);
					genMoveCall(functionGenerator_,
					            memberVar->type(),
					            ptrToMember,
					            destValue,
					            adjustedPositionValue);
				}
			}
			
			return ConstantGenerator(module).getVoidUndef();
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitAlignMask(const SEM::Type* const type) {
			auto& module = functionGenerator_.module();
			const auto& typeInstance = *(type->getObjectType());
			
			IREmitter irEmitter(functionGenerator_);
			
			const auto zero = ConstantGenerator(module).getSizeTValue(0);
			
			if (typeInstance.isUnionDatatype()) {
				// Calculate maximum alignment mask of all variants,
				// which is just a matter of OR-ing them together
				// (the tag byte has an alignment of 1 and hence an
				// alignment mask of 0).
				llvm::Value* maxVariantAlignMask = zero;
				
				for (const auto variantTypeInstance: typeInstance.variants()) {
					const auto variantType = SEM::Type::Object(variantTypeInstance,
					                                           type->templateArguments().copy());
					const auto variantAlignMask = irEmitter.emitAlignMask(variantType);
					maxVariantAlignMask = functionGenerator_.getBuilder().CreateOr(maxVariantAlignMask, variantAlignMask);
				}
				
				return maxVariantAlignMask;
			} else {
				// Calculate maximum alignment mask of all variables,
				// which is just a matter of OR-ing them together.
				llvm::Value* classAlignMask = zero;
				
				for (const auto& var: typeInstance.variables()) {
					const auto varType = var->type()->substitute(type->generateTemplateVarMap());
					const auto varAlignMask = irEmitter.emitAlignMask(varType);
					classAlignMask = functionGenerator_.getBuilder().CreateOr(classAlignMask, varAlignMask);
				}
				
				return classAlignMask;
			}
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitSizeOf(const SEM::Type* const type) {
			auto& module = functionGenerator_.module();
			const auto& typeInstance = *(type->getObjectType());
			
			IREmitter irEmitter(functionGenerator_);
			
			const auto zero = ConstantGenerator(module).getSizeTValue(0);
			const auto one = ConstantGenerator(module).getSizeTValue(1);
			
			if (typeInstance.isUnionDatatype()) {
				// Calculate maximum alignment and size of all variants.
				llvm::Value* maxVariantAlignMask = zero;
				llvm::Value* maxVariantSize = zero;
				
				for (const auto variantTypeInstance: typeInstance.variants()) {
					const auto variantType = SEM::Type::Object(variantTypeInstance,
					                                           type->templateArguments().copy());
					const auto variantAlignMask = irEmitter.emitAlignMask(variantType);
					const auto variantSize = irEmitter.emitSizeOf(variantType);
					
					maxVariantAlignMask = functionGenerator_.getBuilder().CreateOr(maxVariantAlignMask, variantAlignMask);
					
					const auto compareResult = functionGenerator_.getBuilder().CreateICmpUGT(variantSize, maxVariantSize);
					maxVariantSize = functionGenerator_.getBuilder().CreateSelect(compareResult, variantSize, maxVariantSize);
				}
				
				// Add one byte for the tag.
				llvm::Value* classSize = one;
				
				// Align for most alignment variant type.
				classSize = makeAligned(functionGenerator_, classSize, maxVariantAlignMask);
				
				// Add can't overflow.
				const bool hasNoUnsignedWrap = true;
				const bool hasNoSignedWrap = false;
				classSize = functionGenerator_.getBuilder().CreateAdd(classSize,
				                                                      maxVariantSize,
				                                                      "",
				                                                      hasNoUnsignedWrap,
				                                                      hasNoSignedWrap);
				
				return makeAligned(functionGenerator_, classSize, maxVariantAlignMask);
			} else {
				const auto livenessIndicator = getLivenessIndicator(module, typeInstance);
				
				// Add up all member variable sizes.
				llvm::Value* classSize = livenessIndicator.isSuffixByte() ? one : zero;
				
				// Also need to calculate class alignment so the
				// correct amount of padding is added at the end.
				llvm::Value* classAlignMask = zero;
				
				for (const auto& var: typeInstance.variables()) {
					const auto varType = var->type()->substitute(type->generateTemplateVarMap());
					const auto memberAlignMask = irEmitter.emitAlignMask(varType);
					const auto memberSize = irEmitter.emitSizeOf(varType);
					
					classAlignMask = functionGenerator_.getBuilder().CreateOr(classAlignMask,
					                                                          memberAlignMask);
					
					classSize = makeAligned(functionGenerator_, classSize, memberAlignMask);
					
					// Add can't overflow.
					const bool hasNoUnsignedWrap = true;
					const bool hasNoSignedWrap = false;
					classSize = functionGenerator_.getBuilder().CreateAdd(classSize,
					                                                      memberSize,
					                                                      "",
					                                                      hasNoUnsignedWrap,
					                                                      hasNoSignedWrap);
				}
				
				// Class sizes must be at least one byte.
				const auto isZero = functionGenerator_.getBuilder().CreateICmpEQ(classSize, zero);
				classSize = functionGenerator_.getBuilder().CreateSelect(isZero, one, classSize);
				
				return makeAligned(functionGenerator_, classSize, classAlignMask);
			}
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitSetDead(const SEM::Type* const type,
		                                  const SEM::FunctionType /*functionType*/,
		                                  PendingResultArray args) {
			auto& module = functionGenerator_.module();
			auto& builder = functionGenerator_.getBuilder();
			
			const auto& typeInstance = *(type->getObjectType());
			
			const auto contextValue = args[0].resolve(functionGenerator_);
			
			const auto livenessIndicator = getLivenessIndicator(module, typeInstance);
			
			IREmitter irEmitter(functionGenerator_);
			
			switch (livenessIndicator.kind()) {
				case LivenessIndicator::NONE: {
					// Nothing to be done. Note that we don't need to set the
					// member variables into a dead state, since this is done
					// as part of a move operation or destructor call. In fact
					// it's important NOT to do this otherwise it'd mean we
					// would be recursing through the 'subtree' of objects twice
					// for these operations; once for the operation and once at
					// the end when calling __setdead (and since the operation
					// calls __setdead, we'd be calling __setdead more than once
					// for many objects).
					break;
				}
				case LivenessIndicator::MEMBER_INVALID_STATE: {
					// Set the relevant member into an invalid state.
					const auto memberVar = &(livenessIndicator.memberVar());
					const auto memberIndex = module.getMemberVarMap().at(memberVar);
					const auto memberPtr = genMemberPtr(functionGenerator_, contextValue, type, memberIndex);
					genSetInvalidState(functionGenerator_, memberVar->constructType(), memberPtr);
					break;
				}
				case LivenessIndicator::CUSTOM_METHODS: {
					llvm_unreachable("Shouldn't reach custom __setdead method invocation inside auto-generated method.");
					break;
				}
				case LivenessIndicator::SUFFIX_BYTE:
				case LivenessIndicator::GAP_BYTE: {
					// Store zero into suffix/gap byte to represent dead state.
					const auto bytePtr = getLivenessBytePtr(functionGenerator_,
					                                        typeInstance,
					                                        livenessIndicator,
					                                        contextValue);
					builder.CreateStore(ConstantGenerator(module).getI8(0), bytePtr);
					break;
				}
			}
			
			return ConstantGenerator(module).getVoidUndef();
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitIsLive(const SEM::Type* const type,
		                                 const SEM::FunctionType functionType,
		                                 PendingResultArray args) {
			auto& module = functionGenerator_.module();
			auto& builder = functionGenerator_.getBuilder();
			
			const auto& typeInstance = *(type->getObjectType());
			
			const auto contextValue = args[0].resolve(functionGenerator_);
			
			const auto livenessIndicator = getLivenessIndicator(module, typeInstance);
			
			IREmitter irEmitter(functionGenerator_);
			
			switch (livenessIndicator.kind()) {
				case LivenessIndicator::NONE: {
					// Always consider object to be live.
					return ConstantGenerator(module).getI1(true);
				}
				case LivenessIndicator::MEMBER_INVALID_STATE: {
					// Query whether member has invalid state.
					const auto& memberVar = livenessIndicator.memberVar();
					const auto memberIndex = module.getMemberVarMap().at(&memberVar);
					const auto memberPtr = genMemberPtr(functionGenerator_,
					                                    contextValue,
					                                    type,
					                                    memberIndex);
					const auto memberType = memberVar.constructType();
					const MethodInfo methodInfo(memberType, module.getCString("__isvalid"), functionType, {});
					const auto contextArg = RefPendingResult(memberPtr, memberType);
					return genDynamicMethodCall(functionGenerator_, methodInfo, contextArg, {});
				}
				case LivenessIndicator::CUSTOM_METHODS: {
					llvm_unreachable("No custom __islive method exists for liveness indicator that references custom methods!");
				}
				case LivenessIndicator::SUFFIX_BYTE:
				case LivenessIndicator::GAP_BYTE: {
					const auto bytePtr = getLivenessBytePtr(functionGenerator_,
					                                        typeInstance,
					                                        livenessIndicator,
					                                        contextValue);
					const auto byteValue = builder.CreateLoad(bytePtr);
					// Live if suffix/gap byte == 1.
					return builder.CreateICmpEQ(byteValue, ConstantGenerator(module).getI8(1));
				}
			}
			
			llvm_unreachable("Unknown liveness indicator kind.");
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitImplicitCopy(const SEM::Type* const type,
		                                       const SEM::FunctionType functionType,
		                                       PendingResultArray args,
		                                       llvm::Value* const hintResultValue) {
			return emitCopyMethod(METHOD_IMPLICITCOPY,
			                      type,
			                      functionType,
			                      std::move(args),
			                      hintResultValue);
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitExplicitCopy(const SEM::Type* const type,
		                                       const SEM::FunctionType functionType,
		                                       PendingResultArray args,
		                                       llvm::Value* const hintResultValue) {
			return emitCopyMethod(METHOD_COPY,
			                      type,
			                      functionType,
			                      std::move(args),
			                      hintResultValue);
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitCopyMethod(const MethodID methodID,
		                                     const SEM::Type* const type,
		                                     const SEM::FunctionType /*functionType*/,
		                                     PendingResultArray args,
		                                     llvm::Value* const hintResultValue) {
			assert(methodID == METHOD_IMPLICITCOPY ||
			       methodID == METHOD_COPY);
			
			const auto& typeInstance = *(type->getObjectType());
			
			const auto thisPointer = args[0].resolve(functionGenerator_);
			
			IREmitter irEmitter(functionGenerator_, hintResultValue);
			
			if (typeInstance.isUnion()) {
				return irEmitter.emitMoveLoad(thisPointer, type);
			}
			
			auto& module = functionGenerator_.module();
			
			const auto resultValue = irEmitter.emitReturnAlloca(type);
			
			if (typeInstance.isUnionDatatype()) {
				const auto loadedTag = irEmitter.emitLoadDatatypeTag(thisPointer);
				irEmitter.emitStoreDatatypeTag(loadedTag, resultValue);
				
				const auto endBB = functionGenerator_.createBasicBlock("end");
				const auto switchInstruction = functionGenerator_.getBuilder().CreateSwitch(loadedTag,
				                                                                            endBB,
				                                                                            typeInstance.variants().size());
				
				// Start from 1 so that 0 can represent 'empty'.
				uint8_t tag = 1;
				
				for (const auto variantTypeInstance : typeInstance.variants()) {
					const auto matchBB = functionGenerator_.createBasicBlock("tagMatch");
					const auto tagValue = ConstantGenerator(module).getI8(tag++);
					
					switchInstruction->addCase(tagValue, matchBB);
					
					functionGenerator_.selectBasicBlock(matchBB);
					
					const auto variantType = SEM::Type::Object(variantTypeInstance, type->templateArguments().copy());
					
					const auto unionValuePtr = irEmitter.emitGetDatatypeVariantPtr(thisPointer,
					                                                               type,
					                                                               variantType);
					
					const auto copyResult = irEmitter.emitCopyCall(methodID,
					                                               unionValuePtr,
					                                               variantType);
					
					const auto unionValueDestPtr = irEmitter.emitGetDatatypeVariantPtr(resultValue,
					                                                                   type,
					                                                                   variantType);
					
					irEmitter.emitMoveStore(copyResult,
					                        unionValueDestPtr,
					                        variantType);
					
					functionGenerator_.getBuilder().CreateBr(endBB);
				}
				
				functionGenerator_.selectBasicBlock(endBB);
			} else {
				for (const auto& memberVar: typeInstance.variables()) {
					const size_t memberIndex = module.getMemberVarMap().at(memberVar);
					const auto ptrToMember = genMemberPtr(functionGenerator_,
					                                      thisPointer,
					                                      type,
					                                      memberIndex);
					
					const auto memberType = memberVar->constructType()->resolveAliases();
					
					const auto copyResult = irEmitter.emitCopyCall(methodID,
					                                               ptrToMember,
					                                               memberType);
					
					const auto resultPtr = genMemberPtr(functionGenerator_, resultValue, type, memberIndex);
					
					irEmitter.emitMoveStore(copyResult,
					                        resultPtr,
					                        memberType);
				}
				
				// Set object into live state (e.g. set gap byte to 1).
				setOuterLiveState(functionGenerator_,
				                  typeInstance,
				                  resultValue);
			}
			
			return irEmitter.emitMoveLoad(resultValue, type);
		}
		
		static const SEM::Type*
		createRefType(const SEM::Type* const refTargetType,
		              const SEM::TypeInstance& refTypeInstance,
		              const SEM::Type* const typenameType) {
			auto typeRef = SEM::Value::TypeRef(refTargetType,
			                                   typenameType->createStaticRefType(refTargetType));
			SEM::ValueArray templateArguments;
			templateArguments.push_back(std::move(typeRef));
			return SEM::Type::Object(&refTypeInstance,
			                         std::move(templateArguments))->createRefType(refTargetType);
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitCompare(const SEM::Type* const type,
		                                  const SEM::FunctionType functionType,
		                                  PendingResultArray args) {
			const auto& typeInstance = *(type->getObjectType());
			assert(!typeInstance.isUnion() &&
			       "Unions don't support default compare");
			
			const auto otherPointer = args[1].resolve(functionGenerator_);
			const auto thisPointer = args[0].resolve(functionGenerator_);
			
			IREmitter irEmitter(functionGenerator_);
			
			auto& module = functionGenerator_.module();
			const auto i8Type = TypeGenerator(module).getI8Type();
			
			const auto compareResultType = functionType.returnType();
			const auto& refTypeInstance = *(functionType.parameterTypes()[0]->getObjectType());
			const auto typenameType = refTypeInstance.templateVariables()[0]->type();
			
			if (typeInstance.isUnionDatatype()) {
				const auto thisTag = irEmitter.emitLoadDatatypeTag(thisPointer);
				const auto otherTag = irEmitter.emitLoadDatatypeTag(otherPointer);
				
				const auto isTagNotEqual = functionGenerator_.getBuilder().CreateICmpNE(thisTag,
				                                                                        otherTag);
				
				const auto isTagLessThan = functionGenerator_.getBuilder().CreateICmpSLT(thisTag,
				                                                                         otherTag);
				
				const auto tagCompareResult = functionGenerator_.getBuilder().CreateSelect(isTagLessThan,
				                                                                           ConstantGenerator(module).getI8(-1),
				                                                                           ConstantGenerator(module).getI8(1));
				
				const auto startCompareBB = functionGenerator_.createBasicBlock("startCompare");
				
				const auto endBB = functionGenerator_.createBasicBlock("end");
				
				const auto phiNode = llvm::PHINode::Create(i8Type,
				                                           typeInstance.variants().size(),
				                                           "compare_result",
				                                           endBB);
				
				phiNode->addIncoming(tagCompareResult,
				                     functionGenerator_.getBuilder().GetInsertBlock());
				
				functionGenerator_.getBuilder().CreateCondBr(isTagNotEqual,
				                                             endBB,
				                                             startCompareBB);
				
				functionGenerator_.selectBasicBlock(startCompareBB);
				
				const auto unreachableBB = functionGenerator_.createBasicBlock("");
				
				const auto switchInstruction = functionGenerator_.getBuilder().CreateSwitch(thisTag,
				                                                                            unreachableBB,
				                                                                            typeInstance.variants().size());
				
				functionGenerator_.selectBasicBlock(unreachableBB);
				
				functionGenerator_.getBuilder().CreateUnreachable();
				
				// Start from 1 so that 0 can represent 'empty'.
				uint8_t tag = 1;
				
				for (const auto variantTypeInstance : typeInstance.variants()) {
					const auto matchBB = functionGenerator_.createBasicBlock("tagMatch");
					const auto tagValue = ConstantGenerator(module).getI8(tag++);
					
					switchInstruction->addCase(tagValue, matchBB);
					
					functionGenerator_.selectBasicBlock(matchBB);
					
					const auto variantType = SEM::Type::Object(variantTypeInstance, type->templateArguments().copy());
					const auto variantRefType = createRefType(variantType,
					                                          refTypeInstance,
					                                          typenameType);
					
					const auto thisValuePtr = irEmitter.emitGetDatatypeVariantPtr(thisPointer,
					                                                              type,
					                                                              variantType);
					
					const auto otherValuePtr = irEmitter.emitGetDatatypeVariantPtr(otherPointer,
					                                                               type,
					                                                               variantType);
					
					const auto compareResult = irEmitter.emitCompareCall(thisValuePtr,
					                                                     otherValuePtr,
					                                                     compareResultType,
					                                                     variantType,
					                                                     variantRefType);
					
					phiNode->addIncoming(compareResult,
					                     matchBB);
					
					functionGenerator_.getBuilder().CreateBr(endBB);
				}
				
				functionGenerator_.selectBasicBlock(endBB);
				return phiNode;
			} else {
				if (typeInstance.variables().empty()) {
					// Return equal result for empty objects.
					return ConstantGenerator(module).getI8(0);
				}
				
				const auto endBB = functionGenerator_.createBasicBlock("end");
				
				const auto phiNode = llvm::PHINode::Create(i8Type,
				                                           typeInstance.variables().size(),
				                                           "compare_result",
				                                           endBB);
				
				for (size_t i = 0; i < typeInstance.variables().size(); i++) {
					const auto& memberVar = typeInstance.variables()[i];
					const size_t memberIndex = module.getMemberVarMap().at(memberVar);
					const auto thisMemberPtr = genMemberPtr(functionGenerator_,
					                                        thisPointer,
					                                        type,
					                                        memberIndex);
					const auto otherMemberPtr = genMemberPtr(functionGenerator_,
					                                         otherPointer,
					                                         type,
					                                         memberIndex);
					
					const auto memberType = memberVar->constructType()->resolveAliases();
					const auto memberRefType = createRefType(memberType,
					                                         refTypeInstance,
					                                         typenameType);
					
					const auto compareResult = irEmitter.emitCompareCall(thisMemberPtr,
					                                                     otherMemberPtr,
					                                                     compareResultType,
					                                                     memberType,
					                                                     memberRefType);
					
					phiNode->addIncoming(compareResult,
					                     functionGenerator_.getBuilder().GetInsertBlock());
					
					if (i != (typeInstance.variables().size() - 1)) {
						const auto nextCompareBB = functionGenerator_.createBasicBlock("nextCompare");
						
						const auto zeroValue = ConstantGenerator(module).getI8(0);
						
						const auto isEqualResult = functionGenerator_.getBuilder().CreateICmpEQ(compareResult,
						                                                                        zeroValue);
						
						functionGenerator_.getBuilder().CreateCondBr(isEqualResult,
						                                             nextCompareBB,
						                                             endBB);
						
						functionGenerator_.selectBasicBlock(nextCompareBB);
					} else {
						functionGenerator_.getBuilder().CreateBr(endBB);
					}
				}
				
				functionGenerator_.selectBasicBlock(endBB);
				return phiNode;
			}
		}
		
	}
	
}

