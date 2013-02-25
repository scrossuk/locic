#ifndef LOCIC_SEM_TYPEINSTANCE_HPP
#define LOCIC_SEM_TYPEINSTANCE_HPP

#include <string>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/SEM/Var.hpp>

namespace Locic {

	namespace SEM {
	
		struct Function;
		struct NamespaceNode;
		
		class TypeInstance {
			public:
				enum Kind {
					PRIMITIVE,
					STRUCTDECL,
					STRUCTDEF,
					CLASSDECL,
					CLASSDEF,
					INTERFACE
				};
				
				inline TypeInstance(Kind kind, const Locic::Name& name)
					: kind_(kind), name_(name) { }
				
				inline const Locic::Name& getName() const{
					return name_;
				}
				
				inline Kind getKind() const {
					return kind_;
				}
				
				inline std::vector<TemplateVar*>& templateVariables(){
					return templateVariables_;
				}
				
				inline const std::vector<TemplateVar*>& templateVariables() const {
					return templateVariables_;
				}
				
				inline Locic::StringMap<Var*>& variables(){
					return variables_;
				}
				
				inline const Locic::StringMap<Var*>& variables() const {
					return variables_;
				}
				
				inline Locic::StringMap<Function*>& functions(){
					return functions_;
				}
				
				inline const Locic::StringMap<Function*>& functions() const {
					return functions_;
				}
					
				inline bool isPrimitive() const {
					return typeEnum == PRIMITIVE;
				}
				
				inline bool isStructDecl() const {
					return typeEnum == STRUCTDECL;
				}
				
				inline bool isStructDef() const {
					return typeEnum == STRUCTDEF;
				}
				
				inline bool isStruct() const {
					return isStructDecl() || isStructDef();
				}
				
				inline bool isClassDecl() const {
					return typeEnum == CLASSDECL;
				}
				
				inline bool isClassDef() const {
					return typeEnum == CLASSDEF;
				}
				
				inline bool isClass() const {
					return isClassDecl() || isClassDef();
				}
				
				inline bool isInterface() const {
					return typeEnum == INTERFACE;
				}
				
				inline bool isDeclaration() const {
					return isStructDecl() || isClassDecl();
				}
				
				inline bool isDefinition() const {
					return isStructDef() || isClassDef();
				}
				
				NamespaceNode lookup(const Locic::Name& targetName);
				
				bool supportsNullConstruction() const;
				
				bool supportsImplicitCopy() const;
				
				Type* getFunctionReturnType(const std::string& name);
				
				Type* getImplicitCopyType();
				
				std::string toString() const;
				
			private:
				Kind kind_;
				Locic::Name name_;
				std::vector<TemplateVar*> templateVariables_;
				Locic::StringMap<Var*> variables_;
				Locic::StringMap<Function*> functions_;
				
		};
		
	}
	
}

#endif
