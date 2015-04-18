#include <locic/CodeGen/ConstantGenerator.hpp>
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

namespace locic {
	
	namespace CodeGen {
		
		bool typeInstanceHasLivenessIndicator(const SEM::TypeInstance& typeInstance) {
			// TODO: enable when this all works...
			//return typeInstance.isClassDef();
			return false;
		}
		
		static inline bool isPowerOf2(size_t value) {
			return value != 0 && (value & (value - 1)) == 0;
		}
		
		static inline size_t roundUpToAlign(size_t position, size_t align) {
			assert(isPowerOf2(align));
			return (position + (align - 1)) & (~(align - 1));
		}
		
		Optional<LivenessIndicator> getCustomLivenessIndicator(Module& module, const SEM::TypeInstance& typeInstance) {
			// Check if we have a custom __islive method.
			const auto& isLiveMethod = typeInstance.functions().at(module.getCString("__islive"));
			if (!isLiveMethod->isDefault()) {
				// We can just call the __islive method.
				return make_optional(LivenessIndicator::CustomMethods());
			}
			
			// No custom liveness indicator.
			return None;
		}	
		
		Optional<LivenessIndicator> getMemberLivenessIndicator(Module& module, const SEM::TypeInstance& typeInstance) {
			// See if one of the member variables has an invalid state we can use.
			for (const auto& var: typeInstance.variables()) {
				const auto type = var->type();
				if (!type->isObject()) {
					continue;
				}
				
				const auto objectType = type->getObjectType();
				const auto& functions = objectType->functions();
				
				// TODO: check these methods have the right types!
				if (functions.find(module.getCString("__invalid")) != functions.end()
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
			size_t index = 0;
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
					return make_optional(LivenessIndicator::GapByte(index));
				}
				
				currentOffset = nextOffset + module.abi().typeSize(abiType);
				index++;
			}
			
			// No gaps available.
			return None;
		}
		
		LivenessIndicator getLivenessIndicator(Module& module, const SEM::TypeInstance& typeInstance) {
			if (!typeInstanceHasLivenessIndicator(typeInstance)) {
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
			
			// Worst case scenario; just put a byte at the beginning.
			return LivenessIndicator::PrefixByte();
		}
		
		llvm::Function* genDeadFunctionDecl(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto semFunction = typeInstance->functions().at(module.getCString("__dead")).get();
			return genFunctionDecl(module, typeInstance, semFunction);
		}
		
		llvm::Function* genDeadDefaultFunctionDef(Module& module, const SEM::TypeInstance* const typeInstance) {
			const auto llvmFunction = genDeadFunctionDecl(module, typeInstance);
			
			const auto semFunction = typeInstance->functions().at(module.getCString("__dead")).get();
			
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
			
			const auto livenessIndicator = getLivenessIndicator(module, *typeInstance);
			
			const auto deadValue = genAlloca(functionGenerator, selfType, functionGenerator.getReturnVarOrNull());
			
			switch (livenessIndicator.kind()) {
				case LivenessIndicator::NONE: {
					// Return object with dead members.
					for (const auto& memberVar: typeInstance->variables()) {
						const auto memberIndex = module.getMemberVarMap().at(memberVar);
						const auto memberPtr = genMemberPtr(functionGenerator, deadValue, selfType, memberIndex);
						const auto deadMemberValue = genDeadValue(functionGenerator, memberVar->type(), memberPtr);
						genMoveStore(functionGenerator, deadMemberValue, memberPtr, memberVar->type());
					}
					break;
				}
				case LivenessIndicator::MEMBER_INVALID_STATE: {
					// Return object with dead members.
					for (const auto& memberVar: typeInstance->variables()) {
						if (memberVar == &(livenessIndicator.memberVar())) {
							// TODO: create invalid value!
							llvm_unreachable("TODO: create invalid value");
						} else {
							const auto memberIndex = module.getMemberVarMap().at(memberVar);
							const auto memberPtr = genMemberPtr(functionGenerator, deadValue, selfType, memberIndex);
							const auto deadMemberValue = genDeadValue(functionGenerator, memberVar->type(), memberPtr);
							genMoveStore(functionGenerator, deadMemberValue, memberPtr, memberVar->type());
						}
					}
					break;
				}
				case LivenessIndicator::CUSTOM_METHODS: {
					llvm_unreachable("No custom __dead method exists for liveness indicator that references custom methods!");
				}
				case LivenessIndicator::PREFIX_BYTE: {
					// TODO!
					llvm_unreachable("TODO: create dead value with prefix byte");
					break;
				}
				case LivenessIndicator::GAP_BYTE: {
					// TODO!
					llvm_unreachable("TODO: create dead value with gap byte");
					break;
				}
			}
			
			if (argInfo.hasReturnVarArgument()) {
				builder.CreateRetVoid();
			} else {
				builder.CreateRet(genMoveLoad(functionGenerator, deadValue, selfType));
			}
			
			return llvmFunction;
		}
		
		llvm::Value* genDeadValue(Function& function, const SEM::Type* const type, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			
			// Call __dead method.
			const bool isVarArg = false;
			const bool isMethod = false;
			const bool isTemplated = type->isObject() && !type->getObjectType()->templateVariables().empty();
			auto noexceptPredicate = SEM::Predicate::True();
			const auto returnType = type;
			const auto functionType = SEM::Type::Function(isVarArg, isMethod, isTemplated, std::move(noexceptPredicate), returnType, {});
			
			MethodInfo methodInfo(type, module.getCString("__dead"), functionType, {});
			return genStaticMethodCall(function, methodInfo, {}, hintResultValue);
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
					builder.CreateRet(genDynamicMethodCall(functionGenerator, methodInfo, memberPtr, {}));
					break;
				}
				case LivenessIndicator::CUSTOM_METHODS: {
					llvm_unreachable("No custom __islive method exists for liveness indicator that references custom methods!");
				}
				case LivenessIndicator::PREFIX_BYTE: {
					const auto i8PtrType = TypeGenerator(module).getI8PtrType();
					const auto castValue = builder.CreatePointerCast(contextValue, i8PtrType);
					const auto byteValue = builder.CreateLoad(castValue);
					builder.CreateRet(builder.CreateICmpNE(byteValue, ConstantGenerator(module).getI8(0)));
					break;
				}
				case LivenessIndicator::GAP_BYTE: {
					const auto memberIndex = livenessIndicator.gapByteIndex();
					const auto memberPtr = genMemberPtr(functionGenerator, contextValue, selfType, memberIndex);
					const auto i8PtrType = TypeGenerator(module).getI8PtrType();
					const auto castValue = builder.CreatePointerCast(memberPtr, i8PtrType);
					const auto byteValue = builder.CreateLoad(castValue);
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
				if (!typeInstanceHasLivenessIndicator(*(type->getObjectType()))) {
					// Assume value is always live.
					return ConstantGenerator(module).getI1(true);
				}
				
				// Call __islive method.
				const auto methodName = module.getCString("__islive");
				const auto functionType = type->getObjectType()->functions().at(methodName)->type();
				
				MethodInfo methodInfo(type, methodName, functionType, {});
				return genDynamicMethodCall(function, methodInfo, value, {});
			}
			
			llvm_unreachable("Unknown islive value type.");
		}
		
	}
	
}
