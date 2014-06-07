#include <map>

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
		
		TemplateBuilder::TemplateBuilder()
			: templateUseMap_(isTypeLessThan) { }
		
		size_t TemplateBuilder::addUse(SEM::Type* type) {
			assert(type->isObject() && !type->templateArguments().empty());
			
			const auto it = templateUseMap_.find(type);
			if (it != templateUseMap_.end()) {
				return it->second;
			}
			
			const size_t nextId = templateUseMap_.size();
			templateUseMap_.insert(std::make_pair(type, nextId));
			
			for (const auto& arg: type->templateArguments()) {
				if (arg->isObject() && !arg->templateArguments().empty()) {
					(void) addUse(arg);
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
		
	}
	
}

