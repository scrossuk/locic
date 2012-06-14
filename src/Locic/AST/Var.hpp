#ifndef LOCIC_AST_VAR_HPP
#define LOCIC_AST_VAR_HPP

#include <string>

namespace AST{

	struct Var{
		enum Type{
			LOCAL,
			MEMBER
		} type;
		
		std::string name;
		
		inline Var(Type t, const std::string& n)
			: type(t), name(n){ }
		
		inline static Var * Local(const std::string& name){
			return new Var(LOCAL, name);
		}
		
		inline static Var * Member(const std::string& name){
			return new Var(MEMBER, name);
		}
	};

}

#endif
