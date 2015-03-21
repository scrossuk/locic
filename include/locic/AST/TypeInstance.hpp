#ifndef LOCIC_AST_TYPEINSTANCE_HPP
#define LOCIC_AST_TYPEINSTANCE_HPP

#include <string>
#include <vector>

#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/StringList.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace AST {
	
		struct ExceptionInitializer;
		
		struct Function;
		typedef std::vector<Node<Function>> FunctionList;
		
		struct TemplateTypeVar;
		typedef std::vector<Node<TemplateTypeVar>> TemplateTypeVarList;
		
		class TypeVar;
		typedef std::vector<Node<TypeVar>> TypeVarList;
		
		struct TypeInstance;
		typedef std::vector<Node<TypeInstance>> TypeInstanceList;
		
		struct TypeInstance {
			enum Kind {
				PRIMITIVE,
				ENUM,
				STRUCT,
				UNION,
				CLASSDECL,
				CLASSDEF,
				DATATYPE,
				UNION_DATATYPE,
				INTERFACE,
				EXCEPTION
			} kind;
			
			String name;
			Node<StringList> constructors;
			Node<TemplateTypeVarList> templateVariables;
			Node<TypeInstanceList> variants;
			Node<TypeVarList> variables;
			Node<FunctionList> functions;
			Node<ExceptionInitializer> initializer;
			Node<RequireSpecifier> moveSpecifier;
			Node<RequireSpecifier> requireSpecifier;
			Node<StringList> noTagSet;
			
			public:
				static TypeInstance* Primitive(const String& name, const Node<FunctionList>& functions);
				
				static TypeInstance* Enum(const String& name, const Node<StringList>& constructors);
				
				static TypeInstance* Struct(const String& name, const Node<TypeVarList>& variables);
				
				static TypeInstance* Union(const String& name, const Node<TypeVarList>& variables);
				
				static TypeInstance* ClassDecl(const String& name, const Node<FunctionList>& functions);
				
				static TypeInstance* ClassDef(const String& name, const Node<TypeVarList>& variables, const Node<FunctionList>& functions);
				
				static TypeInstance* Datatype(const String& name, const Node<TypeVarList>& variables);
				
				static TypeInstance* UnionDatatype(const String& name, const Node<TypeInstanceList>& variants);
				
				static TypeInstance* Interface(const String& name, const Node<FunctionList>& functions);
				
				static TypeInstance* Exception(const String& name, const Node<TypeVarList>& variables, const Node<ExceptionInitializer>& initializer);
				
				void setMoveSpecifier(const Node<RequireSpecifier>& pMoveSpecifier);
				void setRequireSpecifier(const Node<RequireSpecifier>& pRequireSpecifier);
				void setTemplateVariables(const Node<TemplateTypeVarList>& pTemplateVariables);
				void setNoTagSet(const Node<StringList>& pNoTagSet);
				
				std::string toString() const;
				
			private:
				TypeInstance(Kind k, const String& n,
					const Node<TypeVarList>& v, const Node<FunctionList>& f);
				
		};
		
	}
	
}

#endif
