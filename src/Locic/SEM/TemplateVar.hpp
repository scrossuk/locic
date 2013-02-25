#ifndef LOCIC_SEM_VAR_HPP
#define LOCIC_SEM_VAR_HPP

#include <cstddef>
#include <Locic/String.hpp>

namespace Locic {

	namespace SEM {
	
		struct Type;
		struct TypeInstance;
		
		struct Var {
			enum TypeEnum {
				LOCAL,
				PARAM,
				MEMBER
			} typeEnum;
			
			size_t id;
			Type* type;
			TypeInstance* parent;
			
			inline Var(TypeEnum e, size_t i, Type* t, TypeInstance* p = NULL)
				: typeEnum(e), id(i), type(t), parent(p) { }
			
			std::string toString() const;
		};
		
		enum TemplateVarType{
			TEMPLATEVAR_TYPENAME,
			TEMPLATEVAR_ANY
		};
		
		class TemplateVar{
			public:
				inline static TemplateVar * Create(TemplateVarType type, const std::string& name
			
		};
		
	}
	
}

#endif
