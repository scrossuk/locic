#ifndef LOCIC_SEM_TEMPLATEVAR_HPP
#define LOCIC_SEM_TEMPLATEVAR_HPP

#include <string>

namespace locic {

	namespace SEM {
	
		class TypeInstance;
		
		enum TemplateVarType {
			TEMPLATEVAR_TYPENAME,
			TEMPLATEVAR_POLYMORPHIC
		};
		
		class TemplateVar {
			public:
				TemplateVar(TemplateVarType t);
				
				TemplateVarType type() const;
				
				void setSpecType(TypeInstance * spec);
				
				TypeInstance * specType() const;
				
				std::string toString() const;
				
			private:
				TemplateVarType type_;
				TypeInstance * specType_;
				
		};
		
	}
	
}

#endif
