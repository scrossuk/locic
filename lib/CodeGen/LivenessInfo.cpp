#include <locic/CodeGen/LivenessInfo.hpp>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>

#include <locic/AST/Function.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/CodeGen/ASTFunctionGenerator.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/LivenessIndicator.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

#include <locic/Support/Utils.hpp>

namespace locic {
	
	namespace CodeGen {
		
		LivenessInfo::LivenessInfo(Module& module)
		: module_(module) { }
		
		Optional<LivenessIndicator>
		LivenessInfo::getCustomLivenessIndicator(const AST::TypeInstance& typeInstance) {
			// Check if we have custom __islive and __setdead methods.
			const auto& isLiveMethod = typeInstance.getFunction(module_.getCString("__islive"));
			const auto& setDeadMethod = typeInstance.getFunction(module_.getCString("__setdead"));
			if (!isLiveMethod.isAutoGenerated() && !setDeadMethod.isAutoGenerated()) {
				// We can just call the __islive and __setdead methods.
				return make_optional(LivenessIndicator::CustomMethods());
			}
			
			// No custom liveness indicator.
			return None;
		}	
		
		Optional<LivenessIndicator>
		LivenessInfo::getMemberLivenessIndicator(const AST::TypeInstance& typeInstance) {
			// See if one of the member variables has an invalid state we can use.
			for (const auto& var: typeInstance.variables()) {
				const auto type = var->type();
				if (!type->isObject()) {
					continue;
				}
				
				const auto objectType = type->getObjectType();
				
				if (objectType->findFunction(module_.getCString("__setinvalid")) != nullptr
					&& objectType->findFunction(module_.getCString("__isvalid")) != nullptr) {
					// Member variable has invalid state, so just use
					// that to determine whether the object is live.
					return make_optional(LivenessIndicator::MemberInvalidState(*var));
				}
			}
			
			// No member variables have invalid states.
			return None;
		}
		
		Optional<LivenessIndicator>
		LivenessInfo::getGapByteLivenessIndicator(const AST::TypeInstance& typeInstance) {
			size_t currentOffset = 0;
			TypeInfo typeInfo(module_);
			for (const auto& var: typeInstance.variables()) {
				if (!typeInfo.isSizeKnownInThisModule(var->type())) {
					// Reached an unknown-size member, so give up here.
					break;
				}
				
				const auto abiType = genABIType(module_, var->type());
				const auto& abiTypeInfo = module_.abi().typeInfo();
				const size_t nextOffset = roundUpToAlign(currentOffset,
				                                         abiTypeInfo.getTypeRequiredAlign(abiType).asBytes());
				if (currentOffset != nextOffset) {
					// Found a gap of one or more bytes, so let's
					// insert a byte field here to store the
					// invalid state.
					return make_optional(LivenessIndicator::GapByte(currentOffset));
				}
				
				currentOffset = nextOffset + abiTypeInfo.getTypeAllocSize(abiType).asBytes();
			}
			
			// No gaps available.
			return None;
		}
		
		LivenessIndicator
		LivenessInfo::getLivenessIndicator(const AST::TypeInstance& typeInstance) {
			TypeInfo typeInfo(module_);
			if (!typeInfo.objectHasLivenessIndicator(typeInstance)) {
				// Only classes need liveness indicators because only they
				// have custom destructors and move operations.
				return LivenessIndicator::None();
			}
			
			// Prefer to use user-specified liveness indicator if available.
			const auto customLivenessIndicator = getCustomLivenessIndicator(typeInstance);
			if (customLivenessIndicator) {
				return *customLivenessIndicator;
			}
			
			// Try to find member variable with invalid state that can be used
			// to represent whether this object is live/dead.
			const auto memberLivenessIndicator = getMemberLivenessIndicator(typeInstance);
			if (memberLivenessIndicator) {
				return *memberLivenessIndicator;
			}
			
			const auto gapByteLivenessIndicator = getGapByteLivenessIndicator(typeInstance);
			if (gapByteLivenessIndicator) {
				return *gapByteLivenessIndicator;
			}
			
			// Worst case scenario; just put a byte at the end.
			return LivenessIndicator::SuffixByte();
		}
		
	}
	
}
