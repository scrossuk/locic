#ifndef LOCIC_AST_TYPEINSTANCE_HPP
#define LOCIC_AST_TYPEINSTANCE_HPP

#include <string>
#include <vector>

#include <locic/AST/Node.hpp>

namespace locic {

	namespace AST {
	
		struct Function;
		typedef std::vector<Node<Function>> FunctionList;
		
		struct TemplateTypeVar;
		typedef std::vector<Node<TemplateTypeVar>> TemplateTypeVarList;
		
		struct TypeVar;
		typedef std::vector<Node<TypeVar>> TypeVarList;
		
		struct TypeInstance {
			enum Kind {
				PRIMITIVE,
				STRUCT,
				CLASSDECL,
				CLASSDEF,
				DATATYPE,
				INTERFACE,
			} kind;
			
			std::string name;
			Node<TemplateTypeVarList> templateVariables;
			Node<TypeVarList> variables;
			Node<FunctionList> functions;
			
			public:
				static TypeInstance* Primitive(const std::string& name, const Node<FunctionList>& functions);
				
				static TypeInstance* Struct(const std::string& name, const Node<TypeVarList>& variables);
				
				static TypeInstance* ClassDecl(const std::string& name, const Node<FunctionList>& functions);
				
				static TypeInstance* ClassDef(const std::string& name, const Node<TypeVarList>& variables, const Node<FunctionList>& functions);
				
				static TypeInstance* Datatype(const std::string& name, const Node<TypeVarList>& variables);
				
				static TypeInstance* Interface(const std::string& name, const Node<FunctionList>& functions);
				
				std::string toString() const;
				
			private:
				TypeInstance(Kind k, const std::string& n,
					const Node<TypeVarList>& v, const Node<FunctionList>& f);
				
		};
		
		typedef std::vector<Node<TypeInstance>> TypeInstanceList;
		
	}
	
}

#endif
