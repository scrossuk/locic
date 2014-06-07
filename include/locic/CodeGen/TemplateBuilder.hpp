#ifndef LOCIC_CODEGEN_TEMPLATEBUILDER_HPP
#define LOCIC_CODEGEN_TEMPLATEBUILDER_HPP

#include <map>

#include <locic/SEM.hpp>

namespace locic {

	namespace CodeGen {
		
		typedef std::map<SEM::Type*, size_t, bool(*)(SEM::Type*, SEM::Type*)> TemplateUseMap;
		
		class TemplateBuilder {
			public:
				TemplateBuilder();
				
				/**
				 * \brief Add template use.
				 * 
				 * Generates an entry ID for an intermediate template type
				 * usage; if the same type is used multiple times the same
				 * ID will be returned.
				 */
				size_t addUse(SEM::Type* type);
				
				/**
				 * \brief Bits required to identify template use.
				 * 
				 * Get the number of bits that must be assigned in the
				 * path for determining the relevant component entry.
				 */
				size_t bitsRequired() const;
				
				/**
				 * \brief Get template use map.
				 * 
				 * Returns the mapping from type uses to their
				 * corresponding entry IDs.
				 */
				const TemplateUseMap& templateUseMap() const;
				
			private:
				TemplateUseMap templateUseMap_;
				
		};
		
	}
	
}

#endif
