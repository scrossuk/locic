#ifndef LOCIC_AST_TEMPLATETYPEVAR_HPP
#define LOCIC_AST_TEMPLATETYPEVAR_HPP

#include <string>
#include <Locic/AST/Type.hpp>

namespace AST {

	struct TemplateTypeVar {
		std::string name;
		Type * specType;
		
		inline static TemplateTypeVar * WithoutSpecType(const std::string& name){
			TemplateTypeVar * typeVar = new TemplateTypeVar();
			typeVar->name = name;
			typeVar->specType = NULL;
			return typeVar;
		}
		
		inline static TemplateTypeVar * WithSpecType(const std::string& name, Type* specType){
			assert(specType != NULL);
			TemplateTypeVar * typeVar = new TemplateTypeVar();
			typeVar->name = name;
			typeVar->specType = specType;
			return typeVar;
		}
		
		inline TemplateTypeVar()
			: name(), specType(NULL){ }
	};
	
}

#endif
