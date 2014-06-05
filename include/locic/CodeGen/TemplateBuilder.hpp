#ifndef LOCIC_CODEGEN_TEMPLATEBUILDER_HPP
#define LOCIC_CODEGEN_TEMPLATEBUILDER_HPP

#include <map>

#include <locic/SEM.hpp>

namespace locic {

	namespace CodeGen {
		
		typedef std::map<SEM::Type*, size_t, bool(*)(SEM::Type*, SEM::Type*)> TemplateUseMap;
		
		class TemplateBuilder {
			public:
				TemplateBuilder(SEM::TypeInstance* typeInstance);
				
				size_t addUse(SEM::Type* type);
				
				size_t bitsRequired() const;
				
				const TemplateUseMap& templateUseMap() const;
				
			private:
				TemplateUseMap templateUseMap_;
				
		};
		
	}
	
}

#endif
