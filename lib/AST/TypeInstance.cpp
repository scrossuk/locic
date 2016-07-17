#include <string>
#include <vector>

#include <locic/Support/String.hpp>

#include <locic/AST/ExceptionInitializer.hpp>
#include <locic/AST/Function.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/StringList.hpp>
#include <locic/AST/TemplateVar.hpp>
#include <locic/AST/TypeInstance.hpp>
#include <locic/AST/Var.hpp>

namespace locic {

	namespace AST {
	
		TypeInstance::TypeInstance(Kind k, const String& n, Node<VarList> v, Node<FunctionList> f)
		: kind(k), name(n), templateVariables(makeDefaultNode<TemplateVarList>()),
		  variants(makeDefaultNode<TypeInstanceList>()),
		  variables(std::move(v)), functions(std::move(f)),
		  moveSpecifier(makeNode<RequireSpecifier>(Debug::SourceLocation::Null(), RequireSpecifier::None())),
		  requireSpecifier(makeNode<RequireSpecifier>(Debug::SourceLocation::Null(), RequireSpecifier::None())),
		  semTypeInstance_(nullptr) { }
		
		TypeInstance* TypeInstance::Primitive(const String& name, Node<FunctionList> functions) {
			return new TypeInstance(PRIMITIVE, name, makeDefaultNode<VarList>(), std::move(functions));
		}
		
		TypeInstance* TypeInstance::Enum(const String& name, Node<StringList> constructors) {
			const auto typeInstance = new TypeInstance(ENUM, name, makeDefaultNode<VarList>(), makeDefaultNode<FunctionList>());
			typeInstance->constructors = std::move(constructors);
			return typeInstance;
		}
		
		TypeInstance* TypeInstance::Struct(const String& name, Node<VarList> variables) {
			return new TypeInstance(STRUCT, name, std::move(variables), makeDefaultNode<FunctionList>());
		}
		
		TypeInstance* TypeInstance::OpaqueStruct(const String& name) {
			return new TypeInstance(OPAQUE_STRUCT, name, makeDefaultNode<VarList>(), makeDefaultNode<FunctionList>());
		}
		
		TypeInstance* TypeInstance::Union(const String& name, Node<VarList> variables) {
			return new TypeInstance(UNION, name, std::move(variables), makeDefaultNode<FunctionList>());
		}
		
		TypeInstance* TypeInstance::ClassDecl(const String& name, Node<FunctionList> functions) {
			return new TypeInstance(CLASSDECL, name, makeDefaultNode<VarList>(), std::move(functions));
		}
		
		TypeInstance* TypeInstance::ClassDef(const String& name, Node<VarList> variables, Node<FunctionList> functions) {
			return new TypeInstance(CLASSDEF, name, std::move(variables), std::move(functions));
		}
		
		TypeInstance* TypeInstance::Datatype(const String& name, Node<VarList> variables) {
			return new TypeInstance(DATATYPE, name, std::move(variables), makeDefaultNode<FunctionList>());
		}
		
		TypeInstance* TypeInstance::UnionDatatype(const String& name, Node<TypeInstanceList> variants) {
			TypeInstance* typeInstance = new TypeInstance(UNION_DATATYPE, name,  makeDefaultNode<VarList>(), makeDefaultNode<FunctionList>());
			typeInstance->variants = std::move(variants);
			return typeInstance;
		}
		
		TypeInstance* TypeInstance::Interface(const String& name, Node<FunctionList> functions) {
			return new TypeInstance(INTERFACE, name, makeDefaultNode<VarList>(), std::move(functions));
		}
		
		TypeInstance* TypeInstance::Exception(const String& name, Node<VarList> variables, Node<ExceptionInitializer> initializer) {
			const auto typeInstance = new TypeInstance(EXCEPTION, name, std::move(variables), makeDefaultNode<FunctionList>());
			typeInstance->initializer = std::move(initializer);
			return typeInstance;
		}
		
		TypeInstance::~TypeInstance() { }
		
		void TypeInstance::setMoveSpecifier(Node<RequireSpecifier> pMoveSpecifier) {
			moveSpecifier = std::move(pMoveSpecifier);
		}
		
		void TypeInstance::setRequireSpecifier(Node<RequireSpecifier> pRequireSpecifier) {
			requireSpecifier = std::move(pRequireSpecifier);
		}
		
		void TypeInstance::setTemplateVariables(Node<TemplateVarList> pTemplateVariables) {
			templateVariables = std::move(pTemplateVariables);
		}
		
		void TypeInstance::setNoTagSet(Node<StringList> pNoTagSet) {
			noTagSet = std::move(pNoTagSet);
		}
		
		void TypeInstance::setSEMTypeInstance(SEM::TypeInstance& typeInstance) {
			assert(semTypeInstance_ == nullptr);
			semTypeInstance_ = &typeInstance;
		}
		
		SEM::TypeInstance& TypeInstance::semTypeInstance() {
			assert(semTypeInstance_ != nullptr);
			return *semTypeInstance_;
		}
		
		std::string TypeInstance::toString() const {
			std::string s = makeString("TypeInstance[name = %s](", name.c_str());
			
			{
				s += "templateVariableList: (";
				
				bool isFirst = true;
				
				for (const auto& node : *templateVariables) {
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
				
				for (const auto& node : *variants) {
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
				
				for (const auto& node : *variables) {
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
				
				for (const auto& node : *functions) {
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

