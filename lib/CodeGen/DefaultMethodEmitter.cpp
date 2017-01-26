#include <locic/CodeGen/DefaultMethodEmitter.hpp>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>

#include <locic/AST/FunctionType.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Var.hpp>

#include <locic/CodeGen/CallEmitter.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/LivenessEmitter.hpp>
#include <locic/CodeGen/LivenessIndicator.hpp>
#include <locic/CodeGen/LivenessInfo.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/ValueEmitter.hpp>

#include <locic/AST/Predicate.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		DefaultMethodEmitter::DefaultMethodEmitter(Function& functionGenerator)
		: functionGenerator_(functionGenerator) { }
		
		llvm::Value*
		DefaultMethodEmitter::emitMethod(const MethodID methodID,
		                                 const bool isInnerMethod,
		                                 const AST::Type* const type,
		                                 const AST::FunctionType functionType,
		                                 PendingResultArray args,
		                                 llvm::Value* const resultPtr) {
			if (isInnerMethod) {
				if (methodID == METHOD_DESTROY) {
					return emitInnerDestroy(type,
					                        functionType,
					                        std::move(args));
				} else {
					assert(methodID == METHOD_MOVE);
					return emitInnerMove(type, functionType, std::move(args),
					                     resultPtr);
				}
			}
			
			switch (methodID) {
				case METHOD_CREATE:
					return emitConstructor(type, functionType, std::move(args),
					                       resultPtr);
				case METHOD_DESTROY:
					return emitOuterDestroy(type,
					                        functionType,
					                        std::move(args));
				case METHOD_MOVE:
					return emitOuterMove(type, functionType, std::move(args),
					                     resultPtr);
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
					                        resultPtr);
				case METHOD_COPY:
					return emitExplicitCopy(type,
					                        functionType,
					                        std::move(args),
					                        resultPtr);
				case METHOD_COMPARE:
					return emitCompare(type,
					                   functionType,
					                   std::move(args));
				default:
					llvm_unreachable("Unknown default function.");
			}
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitConstructor(const AST::Type* const type,
		                                      const AST::FunctionType /*functionType*/,
		                                      PendingResultArray args,
		                                      llvm::Value* const resultPtr) {
			const auto& typeInstance = *(type->getObjectType());
			assert(!typeInstance.isEnum() && !typeInstance.isVariant());
			
			auto& module = functionGenerator_.module();
			
			if (typeInstance.isUnion()) {
				assert(resultPtr == nullptr);
				return ConstantGenerator(module).getNull(genABIType(module, type));
			} else if (typeInstance.isException()) {
				return emitExceptionConstructor(type, std::move(args), resultPtr);
			} else {
				return emitTrivialConstructor(type, std::move(args), resultPtr);
			}
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitExceptionConstructor(const AST::Type* const type,
		                                               PendingResultArray args,
		                                               llvm::Value* const resultPtr) {
			const auto& typeInstance = *(type->getObjectType());
			assert(typeInstance.isException());
			
			if (typeInstance.parentType() == nullptr) {
				// No parent, so just create a normal default constructor.
				return emitTrivialConstructor(type, std::move(args), resultPtr);
			}
			
			IREmitter irEmitter(functionGenerator_);
			
			const auto resultValue = irEmitter.emitAlloca(type, resultPtr);
			
			// Start from 1 to skip over parent exception type variable.
			for (size_t i = 1; i < typeInstance.variables().size(); i++) {
				const auto& memberVar = typeInstance.variables()[i];
				
				const auto memberType = memberVar->type()->resolveAliases();
				
				const auto memberPtr = genMemberPtr(functionGenerator_, resultValue, type, memberVar->index());
				
				irEmitter.emitMoveStore(args[i-1].resolve(functionGenerator_),
				                        memberPtr, memberType);
				
				// Add variable to mapping so it can be referred to in the
				// initialisation of the exception parent object.
				functionGenerator_.setVarAddress(*memberVar, memberPtr);
			}
			
			// Need an array to store all the pending results being referred to.
			Array<ValuePendingResult, 10> pendingResultArgs;
			
			PendingResultArray parentArguments;
			parentArguments.reserve(typeInstance.initializerValues().size());
			
			ValueEmitter valueEmitter(irEmitter);
			for (const auto& initializerValue: typeInstance.initializerValues()) {
				const auto irValue = valueEmitter.emitValue(initializerValue);
				pendingResultArgs.push_back(ValuePendingResult(irValue, initializerValue.type()));
				parentArguments.push_back(pendingResultArgs.back());
			}
			
			// Parent exception type variable is the first member variable.
			const auto parentMemberPtr = genMemberPtr(functionGenerator_, resultValue, type, 0);
			
			// Call the constructor of the parent exception type.
			const auto parentValue = irEmitter.emitConstructorCall(typeInstance.parentType(),
			                                                       std::move(parentArguments), parentMemberPtr);
			irEmitter.emitStore(parentValue, parentMemberPtr, typeInstance.parentType());
			
			// Set object into live state (e.g. set gap byte to 1).
			LivenessEmitter(irEmitter).emitSetOuterLive(typeInstance, resultValue);
			
			return irEmitter.emitLoad(resultValue, type);
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitTrivialConstructor(const AST::Type* const type,
		                                             PendingResultArray args,
		                                             llvm::Value* const resultPtr) {
			const auto& typeInstance = *(type->getObjectType());
			IREmitter irEmitter(functionGenerator_);
			
			const auto resultValue = irEmitter.emitAlloca(type, resultPtr);
			
			for (size_t i = 0; i < typeInstance.variables().size(); i++) {
				const auto& memberVar = typeInstance.variables()[i];
				
				const auto memberType = memberVar->type()->resolveAliases();
				
				const auto memberPtr = genMemberPtr(functionGenerator_, resultValue, type, memberVar->index());
				
				const auto argValue = args[i].resolve(functionGenerator_);
				irEmitter.emitMoveStore(argValue, memberPtr, memberType);
			}
			
			// Set object into live state (e.g. set gap byte to 1).
			LivenessEmitter(irEmitter).emitSetOuterLive(typeInstance, resultValue);
			
			return irEmitter.emitLoad(resultValue, type);
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitOuterDestroy(const AST::Type* const type,
		                                       const AST::FunctionType /*functionType*/,
		                                       PendingResultArray args) {
			const auto& typeInstance = *(type->getObjectType());
			auto& module = functionGenerator_.module();
			auto& builder = functionGenerator_.getBuilder();
			
			IREmitter irEmitter(functionGenerator_);
			LivenessEmitter livenessEmitter(irEmitter);
			
			const auto thisValue = args[0].resolve(functionGenerator_);
			
			if (typeInstance.isVariant()) {
				const auto loadedTag = irEmitter.emitLoadVariantTag(thisValue);
				
				const auto endBB = irEmitter.createBasicBlock("end");
				const auto switchInstruction =
					builder.CreateSwitch(loadedTag, endBB,
					                     typeInstance.variantTypes().size());
				
				// Start from 1 so that 0 can represent 'empty'.
				uint8_t tag = 1;
				
				for (const auto variantType: typeInstance.variantTypes()) {
					const auto matchBB = irEmitter.createBasicBlock("tagMatch");
					const auto tagValue = ConstantGenerator(module).getI8(tag++);
					
					switchInstruction->addCase(tagValue, matchBB);
					
					irEmitter.selectBasicBlock(matchBB);
					
					const auto variantPtr =
						irEmitter.emitGetVariantValuePtr(thisValue, type);
					
					irEmitter.emitDestructorCall(variantPtr, variantType);
					
					irEmitter.emitBranch(endBB);
				}
				
				irEmitter.selectBasicBlock(endBB);
			} else {
				const auto isLiveBB = irEmitter.createBasicBlock("is_live");
				const auto endBB = irEmitter.createBasicBlock("");
				
				// Check whether this object is in a 'live' state and only
				// run the destructor if it is.
				const auto isLiveBool = livenessEmitter.emitIsLiveCall(typeInstance.selfType(), thisValue);
				const auto isLive = irEmitter.emitBoolToI1(isLiveBool);
				irEmitter.emitCondBranch(isLive, isLiveBB, endBB);
				
				irEmitter.selectBasicBlock(isLiveBB);
				
				// Call the custom destructor function, if one exists.
				irEmitter.emitInnerDestructorCall(thisValue, type);
				
				const auto& memberVars = typeInstance.variables();
				
				// Call destructors for all objects within the
				// parent object, in *REVERSE* order.
				for (size_t i = 0; i < memberVars.size(); i++) {
					const auto memberVar = memberVars.at((memberVars.size() - 1) - i);
					const auto memberOffsetValue = genMemberOffset(functionGenerator_, typeInstance.selfType(),
					                                               memberVar->index());
					const auto ptrToMember = irEmitter.emitInBoundsGEP(llvm_abi::Int8Ty,
					                                                   thisValue,
					                                                   memberOffsetValue);
					irEmitter.emitDestructorCall(ptrToMember, memberVar->type());
				}
				
				// Put the object into a dead state.
				livenessEmitter.emitSetDeadCall(typeInstance.selfType(), thisValue);
				
				irEmitter.emitBranch(endBB);
				
				irEmitter.selectBasicBlock(endBB);
			}
			
			return ConstantGenerator(module).getVoidUndef();
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitInnerDestroy(const AST::Type* const /*type*/,
		                                       const AST::FunctionType /*functionType*/,
		                                       PendingResultArray args) {
			// Default destroy code doesn't do anything.
			auto& module = functionGenerator_.module();
			(void) args[0].resolve(functionGenerator_);
			return ConstantGenerator(module).getVoidUndef();
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitOuterMove(const AST::Type* const type,
		                                    const AST::FunctionType /*functionType*/,
		                                    PendingResultArray args,
		                                    llvm::Value* const resultPtr) {
			const auto& typeInstance = *(type->getObjectType());
			auto& module = functionGenerator_.module();
			
			IREmitter irEmitter(functionGenerator_);
			LivenessEmitter livenessEmitter(irEmitter);
			
			const auto destPtr = irEmitter.emitAlloca(type, resultPtr);
			const auto sourcePtr = args[0].resolve(functionGenerator_);
			
			const auto livenessIndicator =
			    LivenessInfo(module).getLivenessIndicator(typeInstance);
			
			if (livenessIndicator.isNone()) {
				// No liveness indicator so just move the member values.
				const auto loadedValue =
					irEmitter.emitInnerMoveCall(sourcePtr, type,
					                            destPtr);
				irEmitter.emitStore(loadedValue, destPtr, type);
			} else {
				TypeGenerator typeGenerator(module);
				
				const auto isLiveBB = irEmitter.createBasicBlock("is_live");
				const auto isNotLiveBB = irEmitter.createBasicBlock("is_not_live");
				const auto mergeBB = irEmitter.createBasicBlock("");
				
				// Check whether the source object is in a 'live' state and
				// only perform the move if it is.
				const auto isLiveBool = livenessEmitter.emitIsLiveCall(type, sourcePtr);
				const auto isLive = irEmitter.emitBoolToI1(isLiveBool);
				irEmitter.emitCondBranch(isLive, isLiveBB, isNotLiveBB);
				
				irEmitter.selectBasicBlock(isLiveBB);
				
				// Move member values.
				const auto loadedValue =
					irEmitter.emitInnerMoveCall(sourcePtr, type,
					                            destPtr);
				irEmitter.emitStore(loadedValue, destPtr, type);
				
				// Set dest object to be valid (e.g. may need to set gap byte to 1).
				livenessEmitter.emitSetOuterLive(typeInstance, destPtr);
				
				// Set the source object to dead state.
				livenessEmitter.emitSetDeadCall(type, sourcePtr);
				
				irEmitter.emitBranch(mergeBB);
				
				irEmitter.selectBasicBlock(isNotLiveBB);
				
				// If the source object is dead, set destination to be dead.
				livenessEmitter.emitSetDeadCall(type, destPtr);
				
				irEmitter.emitBranch(mergeBB);
				
				irEmitter.selectBasicBlock(mergeBB);
			}
			
			return irEmitter.emitLoad(destPtr, type);
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitInnerMove(const AST::Type* const type,
		                                    const AST::FunctionType /*functionType*/,
		                                    PendingResultArray args,
		                                    llvm::Value* const resultPtr) {
			auto& module = functionGenerator_.module();
			auto& builder = functionGenerator_.getBuilder();
			const auto& typeInstance = *(type->getObjectType());
			
			IREmitter irEmitter(functionGenerator_);
			
			if (typeInstance.isEnum() || typeInstance.isUnion()) {
				return args[0].resolveWithoutBind(functionGenerator_);
			}
			
			const auto destPtr = irEmitter.emitAlloca(type, resultPtr);
			const auto sourcePtr = args[0].resolve(functionGenerator_);
			
			if (typeInstance.isVariant()) {
				const auto sourcePointers = getVariantPointers(functionGenerator_, type,
				                                               sourcePtr);
				const auto destPointers = getVariantPointers(functionGenerator_, type,
				                                             destPtr);
				TypeGenerator typeGenerator(module);
				const auto loadedTag = irEmitter.emitRawLoad(sourcePointers.first,
				                                             llvm_abi::Int8Ty);
				
				// Store tag.
				irEmitter.emitRawStore(loadedTag, destPointers.first);
				
				// Set previous tag to zero.
				irEmitter.emitRawStore(ConstantGenerator(module).getI8(0),
				                       sourcePointers.first);
				
				// Offset of union datatype data is equivalent to its alignment size.
				
				const auto endBB = irEmitter.createBasicBlock("end");
				const auto switchInstruction =
					builder.CreateSwitch(loadedTag, endBB,
					                     typeInstance.variantTypes().size());
				
				// Start from 1 so that 0 can represent 'empty'.
				uint8_t tag = 1;
				
				for (const auto& variantType: typeInstance.variantTypes()) {
					const auto matchBB = irEmitter.createBasicBlock("tagMatch");
					const auto tagValue = ConstantGenerator(module).getI8(tag++);
					
					switchInstruction->addCase(tagValue, matchBB);
					
					irEmitter.selectBasicBlock(matchBB);
					
					irEmitter.emitMove(sourcePointers.second,
					                   destPointers.second,
					                   variantType);
					
					irEmitter.emitBranch(endBB);
				}
				
				irEmitter.selectBasicBlock(endBB);
			} else {
				// Move member variables.
				for (const auto& memberVar: typeInstance.variables()) {
					const auto memberIndex = memberVar->index();
					const auto sourceMemberPtr = genMemberPtr(functionGenerator_, sourcePtr, type, memberIndex);
					const auto destMemberPtr = genMemberPtr(functionGenerator_, destPtr, type, memberIndex);
					irEmitter.emitMove(sourceMemberPtr, destMemberPtr,
					                   memberVar->type());
				}
			}
			
			return irEmitter.emitLoad(destPtr, type);
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitAlignMask(const AST::Type* const type) {
			auto& module = functionGenerator_.module();
			const auto& typeInstance = *(type->getObjectType());
			
			IREmitter irEmitter(functionGenerator_);
			
			const auto zero = ConstantGenerator(module).getSizeTValue(0);
			
			if (typeInstance.isEnum()) {
				const auto intAlign = module.abi().typeInfo().getTypeRequiredAlign(llvm_abi::IntTy).asBytes();
				return ConstantGenerator(module).getSizeTValue(intAlign - 1);
			} else if (typeInstance.isVariant()) {
				// Calculate maximum alignment mask of all variants,
				// which is just a matter of OR-ing them together
				// (the tag byte has an alignment of 1 and hence an
				// alignment mask of 0).
				llvm::Value* maxVariantAlignMask = zero;
				
				for (const auto variantType: typeInstance.variantTypes()) {
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
		DefaultMethodEmitter::emitSizeOf(const AST::Type* const type) {
			auto& module = functionGenerator_.module();
			const auto& typeInstance = *(type->getObjectType());
			
			IREmitter irEmitter(functionGenerator_);
			
			const auto zero = ConstantGenerator(module).getSizeTValue(0);
			const auto one = ConstantGenerator(module).getSizeTValue(1);
			
			if (typeInstance.isEnum()) {
				const auto intSize = module.abi().typeInfo().getTypeAllocSize(llvm_abi::IntTy).asBytes();
				return ConstantGenerator(module).getSizeTValue(intSize);
			} else if (typeInstance.isUnion()) {
				// Calculate maximum alignment and size of all variants.
				llvm::Value* maxVariantAlignMask = zero;
				llvm::Value* maxVariantSize = zero;
				
				for (const auto& var: typeInstance.variables()) {
					const auto variantAlignMask = irEmitter.emitAlignMask(var->type());
					const auto variantSize = irEmitter.emitSizeOf(var->type());
					
					maxVariantAlignMask = functionGenerator_.getBuilder().CreateOr(maxVariantAlignMask, variantAlignMask);
					
					const auto compareResult = functionGenerator_.getBuilder().CreateICmpUGT(variantSize, maxVariantSize);
					maxVariantSize = functionGenerator_.getBuilder().CreateSelect(compareResult, variantSize, maxVariantSize);
				}
				
				return makeAligned(functionGenerator_, maxVariantSize, maxVariantAlignMask);
			} else if (typeInstance.isVariant()) {
				// Calculate maximum alignment and size of all variants.
				llvm::Value* maxVariantAlignMask = zero;
				llvm::Value* maxVariantSize = zero;
				
				for (const auto variantType: typeInstance.variantTypes()) {
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
				const auto livenessIndicator =
				    LivenessInfo(module).getLivenessIndicator(typeInstance);
				
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
				
				if (!typeInstance.isStruct()) {
					// Class sizes must be at least one byte; empty structs
					// are zero size for compatibility with the GCC extension.
					const auto isZero = functionGenerator_.getBuilder().CreateICmpEQ(classSize, zero);
					classSize = functionGenerator_.getBuilder().CreateSelect(isZero, one, classSize);
				}
				
				return makeAligned(functionGenerator_, classSize, classAlignMask);
			}
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitSetDead(const AST::Type* const type,
		                                  const AST::FunctionType /*functionType*/,
		                                  PendingResultArray args) {
			auto& module = functionGenerator_.module();
			
			const auto& typeInstance = *(type->getObjectType());
			
			const auto contextValue = args[0].resolve(functionGenerator_);
			
			const auto livenessIndicator =
			    LivenessInfo(module).getLivenessIndicator(typeInstance);
			
			IREmitter irEmitter(functionGenerator_);
			LivenessEmitter livenessEmitter(irEmitter);
			
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
					const auto memberPtr = genMemberPtr(functionGenerator_, contextValue, type, memberVar->index());
					livenessEmitter.emitSetInvalidCall(memberVar->type(), memberPtr);
					break;
				}
				case LivenessIndicator::CUSTOM_METHODS: {
					llvm_unreachable("Shouldn't reach custom __setdead method invocation inside auto-generated method.");
					break;
				}
				case LivenessIndicator::SUFFIX_BYTE:
				case LivenessIndicator::GAP_BYTE: {
					// Store zero into suffix/gap byte to represent dead state.
					const auto bytePtr =
					    livenessEmitter.emitLivenessBytePtr(typeInstance,
					                                        livenessIndicator,
					                                        contextValue);
					irEmitter.emitRawStore(ConstantGenerator(module).getI8(0),
					                       bytePtr);
					break;
				}
			}
			
			return ConstantGenerator(module).getVoidUndef();
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitIsLive(const AST::Type* const type,
		                                 const AST::FunctionType functionType,
		                                 PendingResultArray args) {
			auto& module = functionGenerator_.module();
			auto& builder = functionGenerator_.getBuilder();
			
			const auto& typeInstance = *(type->getObjectType());
			
			const auto contextValue = args[0].resolve(functionGenerator_);
			
			const auto livenessIndicator =
			    LivenessInfo(module).getLivenessIndicator(typeInstance);
			
			IREmitter irEmitter(functionGenerator_);
			
			switch (livenessIndicator.kind()) {
				case LivenessIndicator::NONE: {
					// Always consider object to be live.
					return ConstantGenerator(module).getBool(true);
				}
				case LivenessIndicator::MEMBER_INVALID_STATE: {
					// Query whether member has invalid state.
					const auto& memberVar = livenessIndicator.memberVar();
					const auto memberPtr = genMemberPtr(functionGenerator_,
					                                    contextValue,
					                                    type,
					                                    memberVar.index());
					const auto memberType = memberVar.type();
					const MethodInfo methodInfo(memberType, module.getCString("__isvalid"), functionType, {});
					const auto contextArg = RefPendingResult(memberPtr, memberType);
					return CallEmitter(irEmitter).emitDynamicMethodCall(methodInfo, contextArg, {});
				}
				case LivenessIndicator::CUSTOM_METHODS: {
					llvm_unreachable("No custom __islive method exists for liveness indicator that references custom methods!");
				}
				case LivenessIndicator::SUFFIX_BYTE:
				case LivenessIndicator::GAP_BYTE: {
					LivenessEmitter livenessEmitter(irEmitter);
					const auto bytePtr =
					    livenessEmitter.emitLivenessBytePtr(typeInstance,
					                                        livenessIndicator,
					                                        contextValue);
					const auto oneValue = ConstantGenerator(module).getI8(1);
					const auto byteValue = irEmitter.emitRawLoad(bytePtr, llvm_abi::Int8Ty);
					// Live if suffix/gap byte == 1.
					return irEmitter.emitI1ToBool(builder.CreateICmpEQ(byteValue, oneValue));
				}
			}
			
			llvm_unreachable("Unknown liveness indicator kind.");
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitImplicitCopy(const AST::Type* const type,
		                                       const AST::FunctionType functionType,
		                                       PendingResultArray args,
		                                       llvm::Value* const resultPtr) {
			return emitCopyMethod(METHOD_IMPLICITCOPY,
			                      type,
			                      functionType,
			                      std::move(args),
			                      resultPtr);
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitExplicitCopy(const AST::Type* const type,
		                                       const AST::FunctionType functionType,
		                                       PendingResultArray args,
		                                       llvm::Value* const resultPtr) {
			return emitCopyMethod(METHOD_COPY,
			                      type,
			                      functionType,
			                      std::move(args),
			                      resultPtr);
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitCopyMethod(const MethodID methodID,
		                                     const AST::Type* const type,
		                                     const AST::FunctionType /*functionType*/,
		                                     PendingResultArray args,
		                                     llvm::Value* const resultPtr) {
			assert(methodID == METHOD_IMPLICITCOPY ||
			       methodID == METHOD_COPY);
			
			const auto& typeInstance = *(type->getObjectType());
			
			IREmitter irEmitter(functionGenerator_);
			
			if (typeInstance.isEnum() || typeInstance.isUnion()) {
				return args[0].resolveWithoutBind(functionGenerator_);
			}
			
			const auto thisPointer = args[0].resolve(functionGenerator_);
			
			auto& module = functionGenerator_.module();
			
			const auto resultValue = irEmitter.emitAlloca(type, resultPtr);
			
			if (typeInstance.isVariant()) {
				const auto loadedTag = irEmitter.emitLoadVariantTag(thisPointer);
				irEmitter.emitStoreVariantTag(loadedTag, resultValue);
				
				const auto endBB = irEmitter.createBasicBlock("end");
				const auto switchInstruction =
					functionGenerator_.getBuilder().CreateSwitch(loadedTag, endBB,
				                                                     typeInstance.variantTypes().size());
				
				// Start from 1 so that 0 can represent 'empty'.
				uint8_t tag = 1;
				
				for (const auto variantType: typeInstance.variantTypes()) {
					const auto matchBB = irEmitter.createBasicBlock("tagMatch");
					const auto tagValue = ConstantGenerator(module).getI8(tag++);
					
					switchInstruction->addCase(tagValue, matchBB);
					
					irEmitter.selectBasicBlock(matchBB);
					
					const auto variantPtr =
						irEmitter.emitGetVariantValuePtr(thisPointer,
						                                 type);
					
					const auto variantDestPtr =
						irEmitter.emitGetVariantValuePtr(resultValue,
						                                 type);
					
					const auto copyResult = irEmitter.emitCopyCall(methodID,
					                                               variantPtr,
					                                               variantType,
					                                               variantDestPtr);
					irEmitter.emitStore(copyResult, variantDestPtr,
					                    variantType);
					
					irEmitter.emitBranch(endBB);
				}
				
				irEmitter.selectBasicBlock(endBB);
			} else {
				for (const auto& memberVar: typeInstance.variables()) {
					const auto memberIndex = memberVar->index();
					const auto ptrToMember = genMemberPtr(functionGenerator_,
					                                      thisPointer,
					                                      type,
					                                      memberIndex);
					
					const auto memberType = memberVar->type()->resolveAliases();
					
					const auto memberPtr = genMemberPtr(functionGenerator_, resultValue, type, memberIndex);
					
					const auto copyResult = irEmitter.emitCopyCall(methodID,
					                                               ptrToMember,
					                                               memberType,
					                                               memberPtr);
					
					irEmitter.emitStore(copyResult, memberPtr,
					                    memberType);
				}
				
				// Set object into live state (e.g. set gap byte to 1).
				LivenessEmitter(irEmitter).emitSetOuterLive(typeInstance, resultValue);
			}
			
			return irEmitter.emitLoad(resultValue, type);
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitCompare(const AST::Type* const type,
		                                  const AST::FunctionType /*functionType*/,
		                                  PendingResultArray args) {
			const auto& typeInstance = *(type->getObjectType());
			assert(!typeInstance.isUnion() &&
			       "Unions don't support default compare");
			
			const auto otherPointer = args[1].resolve(functionGenerator_);
			const auto thisPointer = args[0].resolve(functionGenerator_);
			
			IREmitter irEmitter(functionGenerator_);
			auto& builder = functionGenerator_.getBuilder();
			
			auto& module = functionGenerator_.module();
			const auto i8Type = TypeGenerator(module).getI8Type();
			
			if (typeInstance.isEnum()) {
				const auto intType = genABIType(module, type);
				const auto thisValue = irEmitter.emitRawLoad(thisPointer, intType);
				const auto otherValue = irEmitter.emitRawLoad(otherPointer, intType);
				
				const auto isNotEqual = builder.CreateICmpNE(thisValue, otherValue);
				const auto isLessThan = builder.CreateICmpSLT(thisValue, otherValue);
				const auto result = builder.CreateSelect(isLessThan,
				                                         ConstantGenerator(module).getI8(-1),
				                                         ConstantGenerator(module).getI8(1));
				return builder.CreateSelect(isNotEqual, result,
				                            ConstantGenerator(module).getI8(0));
			} else if (typeInstance.isVariant()) {
				const auto thisTag = irEmitter.emitLoadVariantTag(thisPointer);
				const auto otherTag = irEmitter.emitLoadVariantTag(otherPointer);
				
				const auto isTagNotEqual = functionGenerator_.getBuilder().CreateICmpNE(thisTag,
				                                                                        otherTag);
				
				const auto isTagLessThan = functionGenerator_.getBuilder().CreateICmpSLT(thisTag,
				                                                                         otherTag);
				
				const auto tagCompareResult = functionGenerator_.getBuilder().CreateSelect(isTagLessThan,
				                                                                           ConstantGenerator(module).getI8(-1),
				                                                                           ConstantGenerator(module).getI8(1));
				
				const auto startCompareBB = irEmitter.createBasicBlock("startCompare");
				
				const auto endBB = irEmitter.createBasicBlock("end");
				
				const auto phiNode = llvm::PHINode::Create(i8Type,
				                                           typeInstance.variantTypes().size(),
				                                           "compare_result",
				                                           endBB);
				
				phiNode->addIncoming(tagCompareResult,
				                     functionGenerator_.getBuilder().GetInsertBlock());
				
				irEmitter.emitCondBranch(isTagNotEqual, endBB,
				                         startCompareBB);
				
				irEmitter.selectBasicBlock(startCompareBB);
				
				const auto unreachableBB = irEmitter.createBasicBlock("");
				
				const auto switchInstruction =
					functionGenerator_.getBuilder().CreateSwitch(thisTag, unreachableBB,
					                                             typeInstance.variantTypes().size());
				
				irEmitter.selectBasicBlock(unreachableBB);
				irEmitter.emitUnreachable();
				
				// Start from 1 so that 0 can represent 'empty'.
				uint8_t tag = 1;
				
				for (const auto variantType: typeInstance.variantTypes()) {
					const auto matchBB = irEmitter.createBasicBlock("tagMatch");
					const auto tagValue = ConstantGenerator(module).getI8(tag++);
					
					switchInstruction->addCase(tagValue, matchBB);
					
					irEmitter.selectBasicBlock(matchBB);
					
					const auto thisValuePtr =
						irEmitter.emitGetVariantValuePtr(thisPointer,
						                                 type);
					
					const auto otherValuePtr =
						irEmitter.emitGetVariantValuePtr(otherPointer,
						                                 type);
					
					const auto compareResult = irEmitter.emitCompareCall(thisValuePtr,
					                                                     otherValuePtr,
					                                                     variantType);
					
					phiNode->addIncoming(compareResult,
					                     matchBB);
					
					irEmitter.emitBranch(endBB);
				}
				
				irEmitter.selectBasicBlock(endBB);
				return phiNode;
			} else {
				if (typeInstance.variables().empty()) {
					// Return equal result for empty objects.
					return ConstantGenerator(module).getI8(0);
				}
				
				llvm::BasicBlock* endBB = nullptr;
				llvm::PHINode* phiNode = nullptr;
				
				if (typeInstance.variables().size() > 1) {
					endBB = irEmitter.createBasicBlock("end");
					
					phiNode = llvm::PHINode::Create(i8Type,
					                                typeInstance.variables().size(),
					                                "compare_result",
					                                endBB);
				}
				
				for (size_t i = 0; i < typeInstance.variables().size(); i++) {
					const auto& memberVar = typeInstance.variables()[i];
					const auto memberIndex = memberVar->index();
					const auto thisMemberPtr = genMemberPtr(functionGenerator_,
					                                        thisPointer,
					                                        type, memberIndex);
					const auto otherMemberPtr = genMemberPtr(functionGenerator_,
					                                         otherPointer,
					                                         type,
					                                         memberIndex);
					
					const auto memberType = memberVar->type()->resolveAliases();
					
					const auto compareResult = irEmitter.emitCompareCall(thisMemberPtr,
					                                                     otherMemberPtr,
					                                                     memberType);
					
					if (typeInstance.variables().size() == 1) {
						return compareResult;
					}
					
					phiNode->addIncoming(compareResult,
					                     functionGenerator_.getBuilder().GetInsertBlock());
					
					if (i != (typeInstance.variables().size() - 1)) {
						const auto nextCompareBB = irEmitter.createBasicBlock("nextCompare");
						
						const auto zeroValue = ConstantGenerator(module).getI8(0);
						
						const auto isEqualResult = functionGenerator_.getBuilder().CreateICmpEQ(compareResult,
						                                                                        zeroValue);
						
						irEmitter.emitCondBranch(isEqualResult,
						                         nextCompareBB, endBB);
						
						irEmitter.selectBasicBlock(nextCompareBB);
					} else {
						irEmitter.emitBranch(endBB);
					}
				}
				
				irEmitter.selectBasicBlock(endBB);
				return phiNode;
			}
		}
		
	}
	
}

