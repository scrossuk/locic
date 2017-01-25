#include <locic/CodeGen/LivenessEmitter.hpp>

#include <locic/AST/Function.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/LivenessIndicator.hpp>
#include <locic/CodeGen/LivenessInfo.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>

namespace locic {
	
	namespace CodeGen {
		
		LivenessEmitter::LivenessEmitter(IREmitter& irEmitter)
		: irEmitter_(irEmitter) { }
		
		llvm::Value*
		LivenessEmitter::emitLivenessByteOffset(const AST::TypeInstance& typeInstance,
		                                        const LivenessIndicator livenessIndicator) {
			if (livenessIndicator.isSuffixByte()) {
				return genSuffixByteOffset(irEmitter_.function(), typeInstance);
			} else if (livenessIndicator.isGapByte()) {
				return ConstantGenerator(irEmitter_.module()).getI64(livenessIndicator.gapByteOffset());
			} else {
				llvm_unreachable("Cannot get byte offset of non-byte liveness indicator.");
			}
		}
		
		llvm::Value*
		LivenessEmitter::emitLivenessBytePtr(const AST::TypeInstance& typeInstance,
		                                     const LivenessIndicator livenessIndicator,
		                                     llvm::Value* const objectPointerValue) {
			const auto byteOffsetValue = emitLivenessByteOffset(typeInstance, livenessIndicator);
			return irEmitter_.emitInBoundsGEP(llvm_abi::Int8Ty,
			                                  objectPointerValue,
			                                  byteOffsetValue);
		}
		
		void
		LivenessEmitter::emitSetOuterLive(const AST::TypeInstance& typeInstance,
		                                  llvm::Value* const objectPointerValue) {
			auto& module = irEmitter_.module();
			
			const auto livenessIndicator =
			    LivenessInfo(module).getLivenessIndicator(typeInstance);
			
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
					const auto bytePtr = emitLivenessBytePtr(typeInstance, livenessIndicator, objectPointerValue);
					irEmitter_.emitRawStore(ConstantGenerator(module).getI8(1),
					                        bytePtr);
					break;
				}
			}
		}
		
		void
		LivenessEmitter::emitSetDeadCall(const AST::Type* const rawType, llvm::Value* const value) {
			const auto type = rawType->resolveAliases();
			auto& function = irEmitter_.function();
			auto& module = irEmitter_.module();
			
			// Call __setdead method.
			if (type->isObject()) {
				const auto methodName = module.getCString("__setdead");
				const auto functionType = type->getObjectType()->getFunction(methodName).type();
				
				MethodInfo methodInfo(type, methodName, functionType, {});
				const auto contextArg = RefPendingResult(value, type);
				genDynamicMethodCall(function, methodInfo, contextArg, {});
				return;
			} else if (type->isTemplateVar()) {
				// TODO!
				return;
			}
			
			llvm_unreachable("Unknown __setdead value type.");
		}
		
		void
		LivenessEmitter::emitSetInvalidCall(const AST::Type* const type, llvm::Value* const value) {
			auto& function = irEmitter_.function();
			auto& module = irEmitter_.module();
			
			// Call __setinvalid method.
			if (type->isObject()) {
				const auto methodName = module.getCString("__setinvalid");
				const auto functionType = type->getObjectType()->getFunction(methodName).type();
				
				MethodInfo methodInfo(type, methodName, functionType, {});
				const auto contextArg = RefPendingResult(value, type);
				genDynamicMethodCall(function, methodInfo, contextArg, {});
				return;
			} else if (type->isTemplateVar()) {
				// TODO!
				return;
			}
			
			llvm_unreachable("Unknown __setinvalid value type.");
		}
		
		llvm::Value*
		LivenessEmitter::emitIsLiveCall(const AST::Type* const type, llvm::Value* const value) {
			auto& function = irEmitter_.function();
			auto& module = irEmitter_.module();
			
			// Call __islive method.
			if (type->isObject()) {
				TypeInfo typeInfo(module);
				if (!typeInfo.objectHasLivenessIndicator(*(type->getObjectType()))) {
					// Assume value is always live.
					return ConstantGenerator(module).getBool(true);
				}
				
				// Call __islive method.
				const auto methodName = module.getCString("__islive");
				const auto functionType = type->getObjectType()->getFunction(methodName).type();
				
				MethodInfo methodInfo(type, methodName, functionType, {});
				const auto contextArg = RefPendingResult(value, type);
				return genDynamicMethodCall(function, methodInfo, contextArg, {});
			}
			
			llvm_unreachable("Unknown __islive value type.");
		}
		
	}
	
}
