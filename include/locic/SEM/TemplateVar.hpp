#ifndef LOCIC_SEM_TEMPLATEVAR_HPP
#define LOCIC_SEM_TEMPLATEVAR_HPP

#include <string>
#include <unordered_map>

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
				TemplateVar(Context& pContext, TemplateVarType t, size_t i);
				
				Context& context() const;
				
				TemplateVarType type() const;
				
				size_t index() const;
				
				void setSpecType(const Type* spec);
				
				const Type* specType() const;
				
				void setSpecTypeInstance(TypeInstance* spec);
				
				TypeInstance* specTypeInstance() const;
				
				std::string toString() const;
				
			private:
				Context& context_;
				TemplateVarType type_;
				size_t index_;
				const Type* specType_;
				TypeInstance* specTypeInstance_;
				
		};
		
		typedef std::unordered_map<TemplateVar*, const Type*> TemplateVarMap;
		
	}
	
}

#endif
