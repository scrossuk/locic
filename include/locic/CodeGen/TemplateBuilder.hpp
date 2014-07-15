#ifndef LOCIC_CODEGEN_TEMPLATEBUILDER_HPP
#define LOCIC_CODEGEN_TEMPLATEBUILDER_HPP

#include <map>

#include <locic/SEM.hpp>

#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/TemplatedObject.hpp>

namespace locic {

	namespace CodeGen {
		
		typedef std::map<TemplateInst, size_t> TemplateUseMap;
		
		class TemplateBuilder {
			public:
				TemplateBuilder();
				
				/**
				 * \brief Add template use.
				 * 
				 * Generates an entry ID for an intermediate template
				 * instantiation; if the same instantiation is used
				 * multiple times the same ID will be returned.
				 */
				size_t addUse(const TemplateInst& templateInst);
				
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
				TemplateUseMap templateUseMap_;
				std::vector<llvm::Instruction*> instructions_;
				
		};
		
	}
	
}

#endif
