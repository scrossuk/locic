#ifndef LOCIC_SEM_TEMPLATEVAR_HPP
#define LOCIC_SEM_TEMPLATEVAR_HPP

#include <string>
#include <unordered_map>

#include <locic/Name.hpp>

namespace locic {

	namespace SEM {
	
		class Context;
		class Type;
		class TypeInstance;
		
		enum TemplateVarType {
			TEMPLATEVAR_TYPENAME,
			TEMPLATEVAR_POLYMORPHIC
		};
		
		class TemplateVar {
			public:
				TemplateVar(Context& pContext, TemplateVarType t, const Name& name, size_t i);
				
				Context& context() const;
				
				TemplateVarType type() const;
				
				const Name& name() const;
				
				size_t index() const;
				
				std::string toString() const;
				
			private:
				Context& context_;
				TemplateVarType type_;
				Name name_;
				size_t index_;
				
		};
		
		/**
		 * \brief Template Var Map
		 * 
		 * An assignment of type values for template variables.
		 */
		typedef std::unordered_map<TemplateVar*, const Type*> TemplateVarMap;
		
	}
	
}

#endif
