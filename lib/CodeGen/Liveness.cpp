#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/LivenessIndicator.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/SEM/TypeInstance.hpp>
#include <locic/Support/Utils.hpp>

namespace locic {
	
	namespace CodeGen {
		
		bool typeInstanceHasLivenessIndicator(Module& module, const SEM::TypeInstance& typeInstance) {
			// A liveness indicator is only required if the object has a custom destructor or move,
			// since the indicator is used to determine whether the destructor/move is run.
			return typeInstance.isClassDef() && (typeInstanceHasCustomMoveMethod(module, typeInstance) ||
				typeInstanceHasCustomDestructor(module, typeInstance));
		}
		
		bool typeHasLivenessIndicator(Module& module, const SEM::Type* const type) {
			return type->isObject() && typeInstanceHasLivenessIndicator(module, *(type->getObjectType()));
		}
		
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
				
				// TODO: check these methods have the right types!
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
			for (const auto& var: typeInstance.variables()) {
				if (!isTypeSizeKnownInThisModule(module, var->type())) {
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
			if (!typeInstanceHasLivenessIndicator(module, typeInstance)) {
				// Only classes need liveness indicators because only they
				// have custom destructors and move operations.
				//printf("%s: NONE\n", typeInstance.name().toString().c_str());
				return LivenessIndicator::None();
			}
			
			// Prefer to use user-specified liveness indicator if available.
			const auto customLivenessIndicator = getCustomLivenessIndicator(module, typeInstance);
			if (customLivenessIndicator) {
				//printf("%s: CUSTOM\n", typeInstance.name().toString().c_str());
				return *customLivenessIndicator;
			}
			
			// Try to find member variable with invalid state that can be used
			// to represent whether this object is live/dead.
			const auto memberLivenessIndicator = getMemberLivenessIndicator(module, typeInstance);
			if (memberLivenessIndicator) {
				//printf("%s: MEMBER\n", typeInstance.name().toString().c_str());
				return *memberLivenessIndicator;
			}
			
			const auto gapByteLivenessIndicator = getGapByteLivenessIndicator(module, typeInstance);
			if (gapByteLivenessIndicator) {
				//printf("%s: GAP BYTE\n", typeInstance.name().toString().c_str());
				return *gapByteLivenessIndicator;
			}
			
			// Worst case scenario; just put a byte at the end.
			//printf("%s: SUFFIX BYTE\n", typeInstance.name().toString().c_str());
			return LivenessIndicator::SuffixByte();
		}
		
		llvm::Function* genSetDeadFunctionDecl(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto semFunction = typeInstance->functions().at(module.getCString("__setdead")).get();
			return genFunctionDecl(module, typeInstance, semFunction);
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
			const auto byteOffsetValue = getLivenessByteOffset(function, typeInstance, livenessIndicator);
			const auto startPtr = builder.CreatePointerCast(objectPointerValue, TypeGenerator(module).getI8PtrType());
			return builder.CreateInBoundsGEP(startPtr, byteOffsetValue);
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
					auto& builder = functionGenerator.getBuilder();
					// Store one into gap/suffix byte to represent live state.
					const auto bytePtr = getLivenessBytePtr(functionGenerator, typeInstance, livenessIndicator, objectPointerValue);
					builder.CreateStore(ConstantGenerator(module).getI8(1), bytePtr);
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
		
		llvm::Function* genSetDeadDefaultFunctionDef(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto llvmFunction = genSetDeadFunctionDecl(module, typeInstance);
			
			const auto semFunction = typeInstance->functions().at(module.getCString("__setdead")).get();
			
			if (semFunction->isDefinition()) {
				// Custom method; generated in genFunctionDef().
				return llvmFunction;
			}
			
			if (semFunction->isPrimitive()) {
				// Generated in genFunctionDecl().
				return llvmFunction;
			}
			
			if (typeInstance->isClassDecl()) {
				// Don't generate code for imported functionality.
				return llvmFunction;
			}
			
			const auto argInfo = getFunctionArgInfo(module, semFunction->type());
			llvmFunction->addFnAttr(llvm::Attribute::InlineHint);
			
			Function functionGenerator(module, *llvmFunction, argInfo, &(module.templateBuilder(TemplatedObject::TypeInstance(typeInstance))));
			
			const auto debugSubprogram = genDebugFunctionInfo(module, semFunction, llvmFunction);
			assert(debugSubprogram);
			functionGenerator.attachDebugInfo(*debugSubprogram);
			
			functionGenerator.setDebugPosition(semFunction->debugInfo()->scopeLocation.range().start());
			
			auto& builder = functionGenerator.getBuilder();
			const auto selfType = typeInstance->selfType();
			
			const auto objectPointerValue = functionGenerator.getContextValue(typeInstance);
			
			const auto livenessIndicator = getLivenessIndicator(module, *typeInstance);
			
			switch (livenessIndicator.kind()) {
				case LivenessIndicator::NONE: {
					// Set member values to dead state; this only needs to
					// occur if any of the members have custom destructors
					// or custom move methods.
					if (typeInstanceHasDestructor(module, *typeInstance) || typeInstanceHasCustomMove(module, typeInstance)) {
						for (const auto& memberVar: typeInstance->variables()) {
							const auto memberIndex = module.getMemberVarMap().at(memberVar);
							const auto memberPtr = genMemberPtr(functionGenerator, objectPointerValue, selfType, memberIndex);
							genSetDeadState(functionGenerator, memberVar->type(), memberPtr);
						}
					}
					break;
				}
				case LivenessIndicator::MEMBER_INVALID_STATE: {
					// Set the relevant member into an invalid state.
					const auto memberVar = &(livenessIndicator.memberVar());
					const auto memberIndex = module.getMemberVarMap().at(memberVar);
					const auto memberPtr = genMemberPtr(functionGenerator, objectPointerValue, selfType, memberIndex);
					genSetInvalidState(functionGenerator, memberVar->constructType(), memberPtr);
					break;
				}
				case LivenessIndicator::CUSTOM_METHODS: {
					llvm_unreachable("Shouldn't reach custom __setdead method invocation inside auto-generated method.");
					break;
				}
				case LivenessIndicator::SUFFIX_BYTE:
				case LivenessIndicator::GAP_BYTE: {
					// Store zero into suffix/gap byte to represent dead state.
					const auto bytePtr = getLivenessBytePtr(functionGenerator, *typeInstance, livenessIndicator, objectPointerValue);
					builder.CreateStore(ConstantGenerator(module).getI8(0), bytePtr);
					break;
				}
			}
			
			builder.CreateRetVoid();
			
			return llvmFunction;
		}
		
		llvm::Function* genIsLiveFunctionDecl(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto semFunction = typeInstance->functions().at(module.getCString("__islive")).get();
			return genFunctionDecl(module, typeInstance, semFunction);
		}
		
		llvm::Function* genIsLiveDefaultFunctionDef(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto llvmFunction = genIsLiveFunctionDecl(module, typeInstance);
			
			const auto semFunction = typeInstance->functions().at(module.getCString("__islive")).get();
			
			if (semFunction->isDefinition()) {
				// Custom method; generated in genFunctionDef().
				return llvmFunction;
			}
			
			if (semFunction->isPrimitive()) {
				// Generated in genFunctionDecl().
				return llvmFunction;
			}
			
			if (typeInstance->isClassDecl()) {
				// Don't generate code for imported functionality.
				return llvmFunction;
			}
			
			const auto argInfo = getFunctionArgInfo(module, semFunction->type());
			llvmFunction->addFnAttr(llvm::Attribute::InlineHint);
			
			Function functionGenerator(module, *llvmFunction, argInfo, &(module.templateBuilder(TemplatedObject::TypeInstance(typeInstance))));
			
			const auto debugSubprogram = genDebugFunctionInfo(module, semFunction, llvmFunction);
			assert(debugSubprogram);
			functionGenerator.attachDebugInfo(*debugSubprogram);
			
			functionGenerator.setDebugPosition(semFunction->debugInfo()->scopeLocation.range().start());
			
			auto& builder = functionGenerator.getBuilder();
			const auto selfType = typeInstance->selfType();
			
			const auto contextValue = functionGenerator.getContextValue(typeInstance);
			const auto livenessIndicator = getLivenessIndicator(module, *typeInstance);
			
			switch (livenessIndicator.kind()) {
				case LivenessIndicator::NONE: {
					// Always consider object to be live.
					builder.CreateRet(ConstantGenerator(module).getI1(true));
					break;
				}
				case LivenessIndicator::MEMBER_INVALID_STATE: {
					// Query whether member has invalid state.
					const auto& memberVar = livenessIndicator.memberVar();
					const auto memberIndex = module.getMemberVarMap().at(&memberVar);
					const auto memberPtr = genMemberPtr(functionGenerator, contextValue, selfType, memberIndex);
					const auto memberType = memberVar.constructType();
					const auto functionType = semFunction->type();
					const MethodInfo methodInfo(memberType, module.getCString("__isvalid"), functionType, {});
					const auto contextArg = RefPendingResult(memberPtr, memberType);
					builder.CreateRet(genDynamicMethodCall(functionGenerator, methodInfo, contextArg, {}));
					break;
				}
				case LivenessIndicator::CUSTOM_METHODS: {
					llvm_unreachable("No custom __islive method exists for liveness indicator that references custom methods!");
				}
				case LivenessIndicator::SUFFIX_BYTE:
				case LivenessIndicator::GAP_BYTE: {
					const auto bytePtr = getLivenessBytePtr(functionGenerator, *typeInstance, livenessIndicator, contextValue);
					const auto byteValue = builder.CreateLoad(bytePtr);
					// Live if suffix/gap byte != 0.
					builder.CreateRet(builder.CreateICmpNE(byteValue, ConstantGenerator(module).getI8(0)));
					break;
				}
			}
			
			return llvmFunction;
		}
		
		llvm::Value* genIsLive(Function& function, const SEM::Type* const type, llvm::Value* const value) {
			auto& module = function.module();
			
			// Call __islive method.
			if (type->isObject()) {
				if (!typeInstanceHasLivenessIndicator(module, *(type->getObjectType()))) {
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
