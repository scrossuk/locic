#include <string>
#include <vector>

#include <locic/Support/String.hpp>

#include <locic/AST/ExceptionInitializer.hpp>
#include <locic/AST/Function.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/StringList.hpp>
#include <locic/AST/TemplateTypeVar.hpp>
#include <locic/AST/TypeInstance.hpp>
#include <locic/AST/TypeVar.hpp>

namespace locic {

	namespace AST {
	
		TypeInstance::TypeInstance(Kind k, const String& n, const Node<TypeVarList>& v, const Node<FunctionList>& f)
			: kind(k), name(n), templateVariables(makeDefaultNode<TemplateTypeVarList>()),
			  variants(makeDefaultNode<TypeInstanceList>()),
			  variables(v), functions(f),
			  moveSpecifier(makeNode<RequireSpecifier>(Debug::SourceLocation::Null(), RequireSpecifier::None())),
			  requireSpecifier(makeNode<RequireSpecifier>(Debug::SourceLocation::Null(), RequireSpecifier::None())) { }
		
		TypeInstance* TypeInstance::Primitive(const String& name, const Node<FunctionList>& functions) {
			return new TypeInstance(PRIMITIVE, name, makeDefaultNode<TypeVarList>(), functions);
		}
		
		TypeInstance* TypeInstance::Enum(const String& name, const Node<StringList>& constructors) {
			const auto typeInstance = new TypeInstance(ENUM, name, makeDefaultNode<TypeVarList>(), makeDefaultNode<FunctionList>());
			typeInstance->constructors = constructors;
			return typeInstance;
		}
		
		TypeInstance* TypeInstance::Struct(const String& name, const Node<TypeVarList>& variables) {
			return new TypeInstance(STRUCT, name, variables, makeDefaultNode<FunctionList>());
		}
		
		TypeInstance* TypeInstance::Union(const String& name, const Node<TypeVarList>& variables) {
			return new TypeInstance(UNION, name, variables, makeDefaultNode<FunctionList>());
		}
		
		TypeInstance* TypeInstance::ClassDecl(const String& name, const Node<FunctionList>& functions) {
			return new TypeInstance(CLASSDECL, name, makeDefaultNode<TypeVarList>(), functions);
		}
		
		TypeInstance* TypeInstance::ClassDef(const String& name, const Node<TypeVarList>& variables, const Node<FunctionList>& functions) {
			return new TypeInstance(CLASSDEF, name, variables, functions);
		}
		
		TypeInstance* TypeInstance::Datatype(const String& name, const Node<TypeVarList>& variables) {
			return new TypeInstance(DATATYPE, name, variables, makeDefaultNode<FunctionList>());
		}
		
		TypeInstance* TypeInstance::UnionDatatype(const String& name, const Node<TypeInstanceList>& variants) {
			TypeInstance* typeInstance = new TypeInstance(UNION_DATATYPE, name,  makeDefaultNode<TypeVarList>(), makeDefaultNode<FunctionList>());
			typeInstance->variants = variants;
			return typeInstance;
		}
		
		TypeInstance* TypeInstance::Interface(const String& name, const Node<FunctionList>& functions) {
			return new TypeInstance(INTERFACE, name, makeDefaultNode<TypeVarList>(), functions);
		}
		
		TypeInstance* TypeInstance::Exception(const String& name, const Node<TypeVarList>& variables, const Node<ExceptionInitializer>& initializer) {
			const auto typeInstance = new TypeInstance(EXCEPTION, name, variables, makeDefaultNode<FunctionList>());
			typeInstance->initializer = initializer;
			return typeInstance;
		}
		
		void TypeInstance::setMoveSpecifier(const Node<RequireSpecifier>& pMoveSpecifier) {
			moveSpecifier = pMoveSpecifier;
		}
		
		void TypeInstance::setRequireSpecifier(const Node<RequireSpecifier>& pRequireSpecifier) {
			requireSpecifier = pRequireSpecifier;
		}
		
		void TypeInstance::setTemplateVariables(const Node<TemplateTypeVarList>& pTemplateVariables) {
			templateVariables = pTemplateVariables;
		}
		
		void TypeInstance::setNoTagSet(const Node<StringList>& pNoTagSet) {
			noTagSet = pNoTagSet;
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

