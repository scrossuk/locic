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
		
		class Function;
		typedef std::vector<Node<Function>> FunctionList;
		
		class TemplateVar;
		typedef std::vector<Node<TemplateVar>> TemplateVarList;
		
		class Var;
		typedef std::vector<Node<Var>> VarList;
		
		struct TypeInstance;
		typedef std::vector<Node<TypeInstance>> TypeInstanceList;
		
		struct TypeInstance {
			enum Kind {
				PRIMITIVE,
				ENUM,
				STRUCT,
				OPAQUE_STRUCT,
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
			Node<TemplateVarList> templateVariables;
			Node<TypeInstanceList> variants;
			Node<VarList> variables;
			Node<FunctionList> functions;
			Node<ExceptionInitializer> initializer;
			Node<RequireSpecifier> moveSpecifier;
			Node<RequireSpecifier> requireSpecifier;
			Node<StringList> noTagSet;
			
			public:
				static TypeInstance* Primitive(const String& name, Node<FunctionList> functions);
				
				static TypeInstance* Enum(const String& name, Node<StringList> constructors);
				
				static TypeInstance* Struct(const String& name, Node<VarList> variables);
				
				static TypeInstance* OpaqueStruct(const String& name);
				
				static TypeInstance* Union(const String& name, Node<VarList> variables);
				
				static TypeInstance* ClassDecl(const String& name, Node<FunctionList> functions);
				
				static TypeInstance* ClassDef(const String& name, Node<VarList> variables, Node<FunctionList> functions);
				
				static TypeInstance* Datatype(const String& name, Node<VarList> variables);
				
				static TypeInstance* UnionDatatype(const String& name, Node<TypeInstanceList> variants);
				
				static TypeInstance* Interface(const String& name, Node<FunctionList> functions);
				
				static TypeInstance* Exception(const String& name, Node<VarList> variables, Node<ExceptionInitializer> initializer);
				
				~TypeInstance();
				
				void setMoveSpecifier(Node<RequireSpecifier> pMoveSpecifier);
				void setRequireSpecifier(Node<RequireSpecifier> pRequireSpecifier);
				void setTemplateVariables(Node<TemplateVarList> pTemplateVariables);
				void setNoTagSet(Node<StringList> pNoTagSet);
				
				std::string toString() const;
				
			private:
				TypeInstance(Kind k, const String& n,
					Node<VarList> v, Node<FunctionList> f);
				
		};
		
	}
	
}

#endif
