#ifndef LOCIC_CODEGEN_TEMPLATEBUILDER_HPP
#define LOCIC_CODEGEN_TEMPLATEBUILDER_HPP

#include <map>
#include <vector>

#include <locic/CodeGen/TemplatedObject.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace CodeGen {
		
		typedef std::map<TemplateInst, size_t> TemplateUseMap;
		
		class TemplateBuilder {
			public:
				TemplateBuilder(TemplatedObject object);
				
				/**
				 * \brief Add template use.
				 * 
				 * Generates an entry ID for an intermediate template
				 * instantiation; if the same instantiation is used
				 * multiple times the same ID will be returned.
				 */
				size_t addUse(const TemplateInst& templateInst);
				
				/**
				 * \brief Get template use.
				 * 
				 * Gets entry ID if template use already added,
				 * otherwise returns None.
				 */
				Optional<size_t> getUse(const TemplateInst& templateInst) const;
				
				/**
				 * \brief Whether using 'pass-through' optimisation.
				 * 
				 * The 'pass-through' optimisation allows a
				 * template generator to call straight down into
				 * next template generator and hence doesn't need
				 * to allocate any space on the path.
				 * 
				 * This can only be used when there is exactly one
				 * template instantiation with the same arguments
				 * as the current templated object (or where one
				 * is the prefix of the other).
				 */
				bool canUsePassThroughOptimisation() const;
				
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
				 * Returns the mapping from template instantiations
				 * to their corresponding entry IDs.
				 */
				const TemplateUseMap& templateUseMap() const;
				
				void addInstruction(llvm::Instruction* instruction);
				
				void updateAllInstructions(Module& module);
				
			private:
				TemplatedObject object_;
				TemplateUseMap templateUseMap_;
				std::vector<llvm::Instruction*> instructions_;
				bool isPassThroughOptimisationCandidate_;
				
		};
		
	}
	
}

#endif
