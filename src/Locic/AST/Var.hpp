#ifndef LOCIC_AST_VAR_HPP
#define LOCIC_AST_VAR_HPP

#include <string>

namespace AST{

	struct Var{
		enum TypeEnum{
			LOCAL,
			MEMBER
		} typeEnum;
		
		std::string name;
		
		inline Var(TypeEnum e, const std::string& n)
			: typeEnum(e), name(n){ }
		
		inline static Var * Local(const std::string& name){
			return new Var(LOCAL, name);
		}
		
		inline static Var * Member(const std::string& name){
			return new Var(MEMBER, name);
		}
	};

}

#endif
