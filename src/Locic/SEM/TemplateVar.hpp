#ifndef LOCIC_SEM_TEMPLATEVAR_HPP
#define LOCIC_SEM_TEMPLATEVAR_HPP

#include <string>
#include <vector>

namespace Locic {

	namespace SEM {
	
		enum TemplateVarType {
			TEMPLATEVAR_TYPENAME,
			TEMPLATEVAR_POLYMORPHIC
		};
		
		class Type;
		
		class TemplateVar {
			public:
				inline TemplateVar(TemplateVarType t,
						size_t i)
					: type_(t), id_(i), specType_(NULL) { }
					
				inline TemplateVarType type() const {
					return type_;
				}
				
				inline size_t id() const {
					return id_;
				}
				
				inline void setSpecType(Type * spec){
					assert(specType_ == NULL);
					assert(spec != NULL);
					specType_ = spec;
				}
				
				inline Type * specType() const {
					return specType_;
				}
				
			private:
				TemplateVarType type_;
				size_t id_;
				Type * specType_;
				
		};
		
	}
	
}

#endif
