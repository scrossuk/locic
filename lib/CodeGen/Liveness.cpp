#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/LivenessIndicator.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/SEMFunctionGenerator.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/SEM/TypeInstance.hpp>
#include <locic/Support/Utils.hpp>

namespace locic {
	
	namespace CodeGen {
		
		Optional<LivenessIndicator> getCustomLivenessIndicator(Module& module, const SEM::TypeInstance& typeInstance) {
			// Check if we have custom __islive and __setdead methods.
			const auto& isLiveMethod = typeInstance.functions().at(module.getCString("__islive"));
			const auto& setDeadMethod = typeInstance.functions().at(module.getCString("__setdead"));
			if (!isLiveMethod->isDefault() && !setDeadMethod->isDefault()) {
				// We can just call the __islive and __setdead methods.
				return make_optional(LivenessIndicator::CustomMethods());
			}
			
			// No custom liveness indicator.
			return None;
		}	
		
		Optional<LivenessIndicator> getMemberLivenessIndicator(Module& module, const SEM::TypeInstance& typeInstance) {
			// See if one of the member variables has an invalid state we can use.
			for (const auto& var: typeInstance.variables()) {
				const auto type = var->constructType();
				if (!type->isObject()) {
					continue;
				}
				
				const auto objectType = type->getObjectType();
				const auto& functions = objectType->functions();
				
				if (functions.find(module.getCString("__setinvalid")) != functions.end()
					&& functions.find(module.getCString("__isvalid")) != functions.end()) {
					// Member variable has invalid state, so just use
					// that to determine whether the object is live.
					return make_optional(LivenessIndicator::MemberInvalidState(*var));
				}
			}
			
			// No member variables have invalid states.
			return None;
		}
		
		Optional<LivenessIndicator> getGapByteLivenessIndicator(Module& module, const SEM::TypeInstance& typeInstance) {
			size_t currentOffset = 0;
			TypeInfo typeInfo(module);
			for (const auto& var: typeInstance.variables()) {
				if (!typeInfo.isSizeKnownInThisModule(var->type())) {
					// Reached an unknown-size member, so give up here.
					break;
				}
				
				const auto abiType = genABIType(module, var->type());
				const size_t nextOffset = roundUpToAlign(currentOffset, module.abi().typeAlign(abiType));
				if (currentOffset != nextOffset) {
					// Found a gap of one or more bytes, so let's
					// insert a byte field here to store the
					// invalid state.
					return make_optional(LivenessIndicator::GapByte(currentOffset));
				}
				
				currentOffset = nextOffset + module.abi().typeSize(abiType);
			}
			
			// No gaps available.
			return None;
		}
		
		LivenessIndicator getLivenessIndicator(Module& module, const SEM::TypeInstance& typeInstance) {
			TypeInfo typeInfo(module);
			if (!typeInfo.objectHasLivenessIndicator(typeInstance)) {
				// Only classes need liveness indicators because only they
				// have custom destructors and move operations.
				return LivenessIndicator::None();
			}
			
			// Prefer to use user-specified liveness indicator if available.
			const auto customLivenessIndicator = getCustomLivenessIndicator(module, typeInstance);
			if (customLivenessIndicator) {
				return *customLivenessIndicator;
			}
			
			// Try to find member variable with invalid state that can be used
			// to represent whether this object is live/dead.
			const auto memberLivenessIndicator = getMemberLivenessIndicator(module, typeInstance);
			if (memberLivenessIndicator) {
				return *memberLivenessIndicator;
			}
			
			const auto gapByteLivenessIndicator = getGapByteLivenessIndicator(module, typeInstance);
			if (gapByteLivenessIndicator) {
				return *gapByteLivenessIndicator;
			}
			
			// Worst case scenario; just put a byte at the end.
			return LivenessIndicator::SuffixByte();
		}
		
		llvm::Function* genSetDeadFunctionDecl(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto& function = typeInstance->functions().at(module.getCString("__setdead"));
			auto& semFunctionGenerator = module.semFunctionGenerator();
			return semFunctionGenerator.getDecl(typeInstance,
			                                    *function);
		}
		
		llvm::Value* getLivenessByteOffset(Function& function, const SEM::TypeInstance& typeInstance, const LivenessIndicator livenessIndicator) {
			if (livenessIndicator.isSuffixByte()) {
				return genSuffixByteOffset(function, typeInstance);
			} else if (livenessIndicator.isGapByte()) {
				return ConstantGenerator(function.module()).getI64(livenessIndicator.gapByteOffset());
			} else {
				llvm_unreachable("Cannot get byte offset of non-byte liveness indicator.");
			}
		}
		
		llvm::Value* getLivenessBytePtr(Function& function, const SEM::TypeInstance& typeInstance,
				const LivenessIndicator livenessIndicator, llvm::Value* const objectPointerValue) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			IREmitter irEmitter(function);
			const auto byteOffsetValue = getLivenessByteOffset(function, typeInstance, livenessIndicator);
			const auto startPtr = builder.CreatePointerCast(objectPointerValue, TypeGenerator(module).getPtrType());
			return irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
			                                 startPtr,
			                                 byteOffsetValue);
		}
		
		void setOuterLiveState(Function& functionGenerator, const SEM::TypeInstance& typeInstance, llvm::Value* const objectPointerValue) {
			auto& module = functionGenerator.module();
			
			const auto livenessIndicator = getLivenessIndicator(module, typeInstance);
			
			switch (livenessIndicator.kind()) {
				case LivenessIndicator::NONE: {
					// Nothing to do.
					break;
				}
				case LivenessIndicator::MEMBER_INVALID_STATE: {
					// Nothing to do; the expectation is that the member is already in a valid state.
					break;
				}
				case LivenessIndicator::CUSTOM_METHODS: {
					// Nothing to do; the expectation is that the object is already in a live state
					// (or that it is in a notionally 'dead' state but it doesn't matter).
					break;
				}
				case LivenessIndicator::SUFFIX_BYTE:
				case LivenessIndicator::GAP_BYTE: {
					// Store one into gap/suffix byte to represent live state.
					const auto bytePtr = getLivenessBytePtr(functionGenerator, typeInstance, livenessIndicator, objectPointerValue);
					IREmitter irEmitter(functionGenerator);
					irEmitter.emitRawStore(ConstantGenerator(module).getI8(1),
					                       bytePtr);
					break;
				}
			}
		}
		
		void genSetDeadState(Function& functionGenerator, const SEM::Type* const rawType, llvm::Value* const value) {
			const auto type = rawType->resolveAliases();
			auto& module = functionGenerator.module();
			
			// Call __setdead method.
			if (type->isObject()) {
				const auto methodName = module.getCString("__setdead");
				const auto functionType = type->getObjectType()->functions().at(methodName)->type();
				
				MethodInfo methodInfo(type, methodName, functionType, {});
				const auto contextArg = RefPendingResult(value, type);
				genDynamicMethodCall(functionGenerator, methodInfo, contextArg, {});
				return;
			} else if (type->isTemplateVar()) {
				// TODO!
				return;
			}
			
			llvm_unreachable("Unknown __setdead value type.");
		}
		
		void genSetInvalidState(Function& functionGenerator, const SEM::Type* const type, llvm::Value* const value) {
			auto& module = functionGenerator.module();
			
			// Call __setinvalid method.
			if (type->isObject()) {
				const auto methodName = module.getCString("__setinvalid");
				const auto functionType = type->getObjectType()->functions().at(methodName)->type();
				
				MethodInfo methodInfo(type, methodName, functionType, {});
				const auto contextArg = RefPendingResult(value, type);
				genDynamicMethodCall(functionGenerator, methodInfo, contextArg, {});
				return;
			} else if (type->isTemplateVar()) {
				// TODO!
				return;
			}
			
			llvm_unreachable("Unknown __setinvalid value type.");
		}
		
		llvm::Function* genIsLiveFunctionDecl(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto& function = typeInstance->functions().at(module.getCString("__islive"));
			auto& semFunctionGenerator = module.semFunctionGenerator();
			return semFunctionGenerator.getDecl(typeInstance,
			                                    *function);
		}
		
		llvm::Value* genIsLive(Function& function, const SEM::Type* const type, llvm::Value* const value) {
			auto& module = function.module();
			
			// Call __islive method.
			if (type->isObject()) {
				TypeInfo typeInfo(module);
				if (!typeInfo.objectHasLivenessIndicator(*(type->getObjectType()))) {
					// Assume value is always live.
					return ConstantGenerator(module).getI1(true);
				}
				
				// Call __islive method.
				const auto methodName = module.getCString("__islive");
				const auto functionType = type->getObjectType()->functions().at(methodName)->type();
				
				MethodInfo methodInfo(type, methodName, functionType, {});
				const auto contextArg = RefPendingResult(value, type);
				return genDynamicMethodCall(function, methodInfo, contextArg, {});
			}
			
			llvm_unreachable("Unknown __islive value type.");
		}
		
	}
	
}
