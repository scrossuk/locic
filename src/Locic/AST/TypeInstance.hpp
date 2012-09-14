#ifndef LOCIC_AST_TYPEINSTANCE_HPP
#define LOCIC_AST_TYPEINSTANCE_HPP

#include <string>
#include <vector>
#include <Locic/AST/TypeVar.hpp>

namespace AST{

	struct Function;
	
	struct TypeInstance{
		enum TypeEnum{
			CLASSDECL = 0,
			CLASSDEF,
			STRUCT
		} typeEnum;
		
		std::string name;
		std::vector<TypeVar *> variables;
		std::vector<Function*> constructors;
		std::vector<Function*> functions;
		
		inline TypeInstance(TypeEnum e, const std::string& n,
			const std::vector<TypeVar *>& v, const std::vector<Function*>& c,
			const std::vector<Function*>& f)
			: typeEnum(e), name(n), variables(v),
			constructors(c), functions(f){ }
		
		inline static TypeInstance * ClassDecl(const std::string& name, const std::vector<Function*>& constructors, const std::vector<Function*>& functions){
			return new TypeInstance(CLASSDECL, name, std::vector<TypeVar *>(), constructors, functions);
		}
		
		inline static TypeInstance * ClassDef(const std::string& name, const std::vector<TypeVar *>& variables, const std::vector<Function*>& constructors, const std::vector<Function*>& functions){
			return new TypeInstance(CLASSDEF, name, variables, constructors, functions);
		}
		
		inline static TypeInstance * Struct(const std::string& name, const std::vector<TypeVar *>& variables){
			return new TypeInstance(STRUCT, name, variables, std::vector<Function *>(), std::vector<Function *>());
		}
		
		inline std::string getFullName() const{
			return name;
		}
	};

}

#endif
