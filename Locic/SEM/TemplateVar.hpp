#ifndef LOCIC_SEM_TEMPLATEVAR_HPP
#define LOCIC_SEM_TEMPLATEVAR_HPP

#include <string>
#include <vector>

#include <Locic/SEM/Object.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace Locic {

	namespace SEM {
	
		enum TemplateVarType {
			TEMPLATEVAR_TYPENAME,
			TEMPLATEVAR_POLYMORPHIC
		};
		
		class TemplateVar: public Object {
			public:
				inline TemplateVar(TemplateVarType t)
					: type_(t), specType_(NULL) { }
				
				inline ObjectKind objectKind() const {
					return OBJECT_TEMPLATEVAR;
				}
				
				inline TemplateVarType type() const {
					return type_;
				}
				
				inline void setSpecType(TypeInstance * spec){
					assert(specType_ == NULL);
					assert(spec != NULL);
					specType_ = spec;
				}
				
				inline TypeInstance * specType() const {
					return specType_;
				}
				
				inline std::string toString() const {
					return makeString("TemplateVar(specType = %s)",
						specType_ != NULL ?
							specType_->refToString().c_str() :
							"[NONE]");
				}
				
			private:
				TemplateVarType type_;
				TypeInstance * specType_;
				
		};
		
	}
	
}

#endif
