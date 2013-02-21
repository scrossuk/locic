#ifndef LOCIC_AST_TYPEINSTANCE_HPP
#define LOCIC_AST_TYPEINSTANCE_HPP

#include <string>
#include <vector>
#include <Locic/AST/TemplateTypeVar.hpp>
#include <Locic/AST/TypeVar.hpp>

namespace AST{

	struct Function;
	
	struct TypeInstance{
		enum TypeEnum{
			PRIMITIVE,
			STRUCT,
			CLASSDECL,
			CLASSDEF,
			INTERFACE
		} typeEnum;
		
		std::string name;
		std::vector<TemplateTypeVar *> templateVariables;
		std::vector<TypeVar *> variables;
		std::vector<Function*> functions;
		
		inline TypeInstance(TypeEnum e, const std::string& n,
			const std::vector<TypeVar *>& v, const std::vector<Function*>& f)
			: typeEnum(e), name(n), templateVariables(),
			variables(v), functions(f){ }
		
		inline static TypeInstance * Primitive(const std::string& name, const std::vector<Function*>& functions){
			return new TypeInstance(PRIMITIVE, name, std::vector<TypeVar *>(), functions);
		}
		
		inline static TypeInstance * ClassDecl(const std::string& name, const std::vector<Function*>& functions){
			return new TypeInstance(CLASSDECL, name, std::vector<TypeVar *>(), functions);
		}
		
		inline static TypeInstance * ClassDef(const std::string& name, const std::vector<TypeVar *>& variables, const std::vector<Function*>& functions){
			return new TypeInstance(CLASSDEF, name, variables, functions);
		}
		
		inline static TypeInstance * Interface(const std::string& name, const std::vector<Function*>& functions){
			return new TypeInstance(INTERFACE, name, std::vector<TypeVar *>(), functions);
		}
		
		inline static TypeInstance * Struct(const std::string& name, const std::vector<TypeVar *>& variables){
			return new TypeInstance(STRUCT, name, variables, std::vector<Function *>());
		}
	};

}

#endif
