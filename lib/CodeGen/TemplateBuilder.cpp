#include <map>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TemplateBuilder.hpp>
#include <locic/SEM/Type.hpp>

namespace locic {

	namespace CodeGen {
		
		size_t roundUpToPowerOf2(size_t value) {
			value--;
			value |= value >> 1;
			value |= value >> 2;
			value |= value >> 4;
			value |= value >> 8;
			value |= value >> 16;
			value++;
			return value;
		}
		
		uint8_t getNextPowerOfTwo(size_t originalValue) {
			assert(originalValue > 0);
			
			size_t value = roundUpToPowerOf2(originalValue);
			
			uint8_t power = 0;
			while (value != 1) {
				value >>= 1;
				power++;
			}
			return power;
		}
		
		TemplateBuilder::TemplateBuilder(TemplatedObject object)
		: object_(object),
		isPassThroughOptimisationCandidate_(false) { }
		
		Optional<size_t> TemplateBuilder::getUse(const TemplateInst& templateInst) const {
			assert(!templateInst.arguments().empty());
			
			const auto it = templateUseMap_.find(templateInst);
			if (it != templateUseMap_.end()) {
				return make_optional(it->second);
			}
			
			if (templateInst.object() == object_ &&
			    templateInst.allArgumentsAreSelfTemplateVars()) {
				// Don't need to add anything to the path when
				// passing our own template arguments to ourselves.
				return Optional<size_t>(-1);
			}
			
			return None;
		}
		
		size_t TemplateBuilder::addUse(const TemplateInst& templateInst) {
			assert(!templateInst.arguments().empty());
			
			const auto existingId = getUse(templateInst);
			if (existingId) {
				return existingId.value();
			}
			
			const size_t nextId = templateUseMap_.size();
			templateUseMap_.insert(std::make_pair(templateInst.copy(), nextId));
			
			const auto arguments = templateInst.arguments();
			
			const auto objectTemplateVars = object_.templateVariables();
			if (arguments.size() == objectTemplateVars.size() &&
			    templateInst.allArgumentsAreTemplateVars(objectTemplateVars)) {
				// If we're passing our own template arguments to
				// something else unchanged, then we could apply
				// the 'pass-through optimisation', which effectively
				// means we don't need to allocate any space on
				// the path and just call directly down to the
				// next template generator.
				// 
				// Note that this only works if there's exaclty
				// one template call, hence it's just considered
				// a 'candidate' at this point.
				isPassThroughOptimisationCandidate_ = true;
			}
			
			for (const auto& arg: arguments) {
				if (arg.isTypeRef()) {
					const auto typeArg = arg.typeRefType();
					if (typeArg->isObject() && !typeArg->templateArguments().empty()) {
						(void) addUse(TemplateInst::Type(typeArg));
					}
				}
			}
			
			return nextId;
		}
		
		bool TemplateBuilder::canUsePassThroughOptimisation() const {
			return templateUseMap_.size() == 1 &&
			       isPassThroughOptimisationCandidate_;
		}
		
		size_t TemplateBuilder::bitsRequired() const {
			if (templateUseMap_.empty()) {
				// When there are no uses, the path
				// terminates and no bits are required.
				return 0;
			}
			
			if (templateUseMap_.size() == 1) {
				// If there is one use, then we still
				// need a bit to determine whether or
				// not to take that path.
				return 1;
			}
			
			return getNextPowerOfTwo(templateUseMap_.size());
		}
		
		const TemplateUseMap& TemplateBuilder::templateUseMap() const {
			return templateUseMap_;
		}
		
		void TemplateBuilder::addInstruction(llvm::Instruction* instruction) {
			instructions_.push_back(instruction);
		}
		
		void TemplateBuilder::updateAllInstructions(Module& module) {
			for (const auto instruction: instructions_) {
				instruction->setOperand(1, ConstantGenerator(module).getI64(bitsRequired()));
			}
		}
		
	}
	
}

