#include <string>
#include <vector>

#include <locic/String.hpp>

#include <locic/AST/ExceptionInitializer.hpp>
#include <locic/AST/Function.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/TemplateTypeVar.hpp>
#include <locic/AST/TypeInstance.hpp>
#include <locic/AST/TypeVar.hpp>

namespace locic {

	namespace AST {
	
		TypeInstance::TypeInstance(Kind k, const std::string& n, const Node<TypeVarList>& v, const Node<FunctionList>& f)
			: kind(k), name(n), templateVariables(makeDefaultNode<TemplateTypeVarList>()),
			  variants(makeDefaultNode<TypeInstanceList>()),
			  variables(v), functions(f),
			  requireSpecifier(makeNode<RequireSpecifier>(Debug::SourceLocation::Null(), RequireSpecifier::None())) { }
		
		TypeInstance* TypeInstance::Primitive(const std::string& name, const Node<FunctionList>& functions) {
			return new TypeInstance(PRIMITIVE, name, makeDefaultNode<TypeVarList>(), functions);
		}
		
		TypeInstance* TypeInstance::ClassDecl(const std::string& name, const Node<FunctionList>& functions) {
			return new TypeInstance(CLASSDECL, name, makeDefaultNode<TypeVarList>(), functions);
		}
		
		TypeInstance* TypeInstance::ClassDef(const std::string& name, const Node<TypeVarList>& variables, const Node<FunctionList>& functions) {
			return new TypeInstance(CLASSDEF, name, variables, functions);
		}
		
		TypeInstance* TypeInstance::Datatype(const std::string& name, const Node<TypeVarList>& variables) {
			return new TypeInstance(DATATYPE, name, variables, makeDefaultNode<FunctionList>());
		}
		
		TypeInstance* TypeInstance::UnionDatatype(const std::string& name, const Node<TypeInstanceList>& variants) {
			TypeInstance* typeInstance = new TypeInstance(UNION_DATATYPE, name,  makeDefaultNode<TypeVarList>(), makeDefaultNode<FunctionList>());
			typeInstance->variants = variants;
			return typeInstance;
		}
		
		TypeInstance* TypeInstance::Interface(const std::string& name, const Node<FunctionList>& functions) {
			return new TypeInstance(INTERFACE, name, makeDefaultNode<TypeVarList>(), functions);
		}
		
		TypeInstance* TypeInstance::Struct(const std::string& name, const Node<TypeVarList>& variables) {
			return new TypeInstance(STRUCT, name, variables, makeDefaultNode<FunctionList>());
		}
		
		TypeInstance* TypeInstance::Exception(const std::string& name, const Node<TypeVarList>& variables, const Node<ExceptionInitializer>& initializer) {
			const auto typeInstance = new TypeInstance(EXCEPTION, name, variables, makeDefaultNode<FunctionList>());
			typeInstance->initializer = initializer;
			return typeInstance;
		}
		
		void TypeInstance::setRequireSpecifier(const Node<RequireSpecifier>& pRequireSpecifier) {
			requireSpecifier = pRequireSpecifier;
		}
		
		void TypeInstance::setTemplateVariables(const Node<TemplateTypeVarList>& pTemplateVariables) {
			templateVariables = pTemplateVariables;
		}
		
		std::string TypeInstance::toString() const {
			std::string s = makeString("TypeInstance[name = %s](", name.c_str());
			
			{
				s += "templateVariableList: (";
				
				bool isFirst = true;
				
				for (auto node : *templateVariables) {
					if (!isFirst) {
						s += ", ";
					}
					
					isFirst = false;
					s += node.toString();
				}
				
				s += ")";
			}
			
			s += ", ";
			
			{
				s += "variantList: (";
				
				bool isFirst = true;
				
				for (auto node : *variants) {
					if (!isFirst) {
						s += ", ";
					}
					
					isFirst = false;
					s += node.toString();
				}
				
				s += ")";
			}
			
			s += ", ";
			
			{
				s += "memberVariableList: (";
				
				bool isFirst = true;
				
				for (auto node : *variables) {
					if (!isFirst) {
						s += ", ";
					}
					
					isFirst = false;
					s += node.toString();
				}
				
				s += ")";
			}
			
			s += ", ";
			
			{
				s += "functionList: (";
				
				bool isFirst = true;
				
				for (auto node : *functions) {
					if (!isFirst) {
						s += ", ";
					}
					
					isFirst = false;
					s += node.toString();
				}
				
				s += ")";
			}
			
			s += ", ";
			
			s += makeString("initializer: %s", initializer.toString().c_str());
			
			s += ")";
			return s;
		}
		
	}
	
}

