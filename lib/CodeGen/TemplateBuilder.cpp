#include <map>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TemplateBuilder.hpp>

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
		
		TemplateBuilder::TemplateBuilder() { }
		
		size_t TemplateBuilder::addUse(const TemplateInst& templateInst) {
			assert(!templateInst.arguments().empty());
			
			const auto it = templateUseMap_.find(templateInst);
			if (it != templateUseMap_.end()) {
				return it->second;
			}
			
			const size_t nextId = templateUseMap_.size();
			templateUseMap_.insert(std::make_pair(templateInst.copy(), nextId));
			
			for (const auto& arg: templateInst.arguments()) {
				if (arg.isTypeRef()) {
					const auto typeArg = arg.typeRefType();
					if (typeArg->isObject() && !typeArg->templateArguments().empty()) {
						(void) addUse(TemplateInst::Type(typeArg));
					}
				}
			}
			
			return nextId;
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
				instruction->setOperand(1, ConstantGenerator(module).getI32(bitsRequired()));
			}
		}
		
	}
	
}

