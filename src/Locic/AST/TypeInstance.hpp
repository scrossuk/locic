#ifndef LOCIC_AST_TYPEINSTANCE_HPP
#define LOCIC_AST_TYPEINSTANCE_HPP

#include <string>
#include <vector>
#include <Locic/AST/Function.hpp>
#include <Locic/AST/Node.hpp>
#include <Locic/AST/TemplateTypeVar.hpp>
#include <Locic/AST/TypeVar.hpp>

namespace AST {

	struct TypeInstance {
		enum TypeEnum {
			PRIMITIVE,
			STRUCT,
			CLASSDECL,
			CLASSDEF,
			INTERFACE
		} typeEnum;
		
		std::string name;
		Node<TemplateTypeVarList> templateVariables;
		Node<TypeVarList> variables;
		Node<FunctionList> functions;
		
		inline TypeInstance(TypeEnum e, const std::string& n,
				const Node<TypeVarList>& v, const Node<FunctionList>& f)
			: typeEnum(e), name(n), templateVariables(makeDefaultNode<TemplateTypeVarList>()),
			  variables(v), functions(f) { }
			  
		inline static TypeInstance* Primitive(const std::string& name, const Node<FunctionList>& functions) {
			return new TypeInstance(PRIMITIVE, name, makeDefaultNode<TypeVarList>(), functions);
		}
		
		inline static TypeInstance* ClassDecl(const std::string& name, const Node<FunctionList>& functions) {
			return new TypeInstance(CLASSDECL, name, makeDefaultNode<TypeVarList>(), functions);
		}
		
		inline static TypeInstance* ClassDef(const std::string& name, const Node<TypeVarList>& variables, const Node<FunctionList>& functions) {
			return new TypeInstance(CLASSDEF, name, variables, functions);
		}
		
		inline static TypeInstance* Interface(const std::string& name, const Node<FunctionList>& functions) {
			return new TypeInstance(INTERFACE, name, makeDefaultNode<TypeVarList>(), functions);
		}
		
		inline static TypeInstance* Struct(const std::string& name, const Node<TypeVarList>& variables) {
			return new TypeInstance(STRUCT, name, variables, makeDefaultNode<FunctionList>());
		}
		
		inline std::string toString() const {
			std::string s = Locic::makeString("TypeInstance[name = %s](", name.c_str());
			
			{
				s += "TemplateVariableList(";
				
				bool isFirst = true;
				for (auto node: *templateVariables) {
					if (!isFirst) s += ", ";
					isFirst = false;
					s += node.toString();
				}
				
				s += ")";
			}
			
			s += ", ";
			
			{
				s += "MemberVariableList(";
				
				bool isFirst = true;
				for (auto node: *variables) {
					if (!isFirst) s += ", ";
					isFirst = false;
					s += node.toString();
				}
				
				s += ")";
			}
			
			s += ", ";
			
			{
				s += "FunctionList(";
				
				bool isFirst = true;
				for (auto node: *functions) {
					if (!isFirst) s += ", ";
					isFirst = false;
					s += node.toString();
				}
				
				s += ")";
			}
			
			s += ")";
			return s;
		}
	};
	
	typedef std::vector<Node<TypeInstance>> TypeInstanceList;
	
}

#endif
