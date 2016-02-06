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
			Node<TemplateTypeVarList> templateVariables;
			Node<TypeInstanceList> variants;
			Node<TypeVarList> variables;
			Node<FunctionList> functions;
			Node<ExceptionInitializer> initializer;
			Node<RequireSpecifier> moveSpecifier;
			Node<RequireSpecifier> requireSpecifier;
			Node<StringList> noTagSet;
			
			public:
				static TypeInstance* Primitive(const String& name, Node<FunctionList> functions);
				
				static TypeInstance* Enum(const String& name, Node<StringList> constructors);
				
				static TypeInstance* Struct(const String& name, Node<TypeVarList> variables);
				
				static TypeInstance* OpaqueStruct(const String& name);
				
				static TypeInstance* Union(const String& name, Node<TypeVarList> variables);
				
				static TypeInstance* ClassDecl(const String& name, Node<FunctionList> functions);
				
				static TypeInstance* ClassDef(const String& name, Node<TypeVarList> variables, Node<FunctionList> functions);
				
				static TypeInstance* Datatype(const String& name, Node<TypeVarList> variables);
				
				static TypeInstance* UnionDatatype(const String& name, Node<TypeInstanceList> variants);
				
				static TypeInstance* Interface(const String& name, Node<FunctionList> functions);
				
				static TypeInstance* Exception(const String& name, Node<TypeVarList> variables, Node<ExceptionInitializer> initializer);
				
				~TypeInstance();
				
				void setMoveSpecifier(Node<RequireSpecifier> pMoveSpecifier);
				void setRequireSpecifier(Node<RequireSpecifier> pRequireSpecifier);
				void setTemplateVariables(Node<TemplateTypeVarList> pTemplateVariables);
				void setNoTagSet(Node<StringList> pNoTagSet);
				
				std::string toString() const;
				
			private:
				TypeInstance(Kind k, const String& n,
					Node<TypeVarList> v, Node<FunctionList> f);
				
		};
		
	}
	
}

#endif
