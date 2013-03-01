#ifndef LOCIC_SEM_TEMPLATEVAR_HPP
#define LOCIC_SEM_TEMPLATEVAR_HPP

#include <string>
#include <vector>

namespace Locic {

	namespace SEM {
	
		enum TemplateVarType {
			TEMPLATEVAR_TYPENAME,
			TEMPLATEVAR_ANY
		};
		
		class Type;
		
		class TemplateVar {
			public:
				inline TemplateVar(TemplateVarType t,
						const std::string& n, const std::vector<Type*>& r)
					: type_(t), name_(n), requirements_(r) { }
					
				inline TemplateVarType type() const {
					return type_;
				}
				
				inline const std::string& name() const {
					return name_;
				}
				
				inline const std::vector<Type*>& requirements() const {
					return requirements_;
				}
				
			private:
				TemplateVarType type_;
				const std::string& name_;
				const std::vector<Type*>& requirements_;
				
		};
		
	}
	
}

#endif
