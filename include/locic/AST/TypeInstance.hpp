#ifndef LOCIC_AST_TYPEINSTANCE_HPP
#define LOCIC_AST_TYPEINSTANCE_HPP

#include <string>
#include <vector>

#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>

namespace locic {

	namespace AST {
	
		struct ExceptionInitializer;
		
		struct Function;
		typedef std::vector<Node<Function>> FunctionList;
		
		struct TemplateTypeVar;
		typedef std::vector<Node<TemplateTypeVar>> TemplateTypeVarList;
		
		struct TypeVar;
		typedef std::vector<Node<TypeVar>> TypeVarList;
		
		struct TypeInstance;
		typedef std::vector<Node<TypeInstance>> TypeInstanceList;
		
		struct TypeInstance {
			enum Kind {
				PRIMITIVE,
				STRUCT,
				CLASSDECL,
				CLASSDEF,
				DATATYPE,
				UNION_DATATYPE,
				INTERFACE,
				EXCEPTION
			} kind;
			
			std::string name;
			Node<TemplateTypeVarList> templateVariables;
			Node<TypeInstanceList> variants;
			Node<TypeVarList> variables;
			Node<FunctionList> functions;
			Node<ExceptionInitializer> initializer;
			Node<RequireSpecifier> requireSpecifier;
			
			public:
				static TypeInstance* Primitive(const std::string& name, const Node<FunctionList>& functions);
				
				static TypeInstance* Struct(const std::string& name, const Node<TypeVarList>& variables);
				
				static TypeInstance* ClassDecl(const std::string& name, const Node<FunctionList>& functions);
				
				static TypeInstance* ClassDef(const std::string& name, const Node<TypeVarList>& variables, const Node<FunctionList>& functions);
				
				static TypeInstance* Datatype(const std::string& name, const Node<TypeVarList>& variables);
				
				static TypeInstance* UnionDatatype(const std::string& name, const Node<TypeInstanceList>& variants);
				
				static TypeInstance* Interface(const std::string& name, const Node<FunctionList>& functions);
				
				static TypeInstance* Exception(const std::string& name, const Node<TypeVarList>& variables, const Node<ExceptionInitializer>& initializer);
				
				void setRequireSpecifier(const Node<RequireSpecifier>& pRequireSpecifier);
				void setTemplateVariables(const Node<TemplateTypeVarList>& pTemplateVariables);
				
				std::string toString() const;
				
			private:
				TypeInstance(Kind k, const std::string& n,
					const Node<TypeVarList>& v, const Node<FunctionList>& f);
				
		};
		
	}
	
}

#endif
