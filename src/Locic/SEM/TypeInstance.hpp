#ifndef LOCIC_SEM_TYPEINSTANCE_HPP
#define LOCIC_SEM_TYPEINSTANCE_HPP

#include <string>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/SEM/TemplateVar.hpp>
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
				
				inline TypeInstance(Kind k, const Locic::Name& n)
					: kind_(k), name_(n) { }
					
				inline const Locic::Name& name() const {
					return name_;
				}
				
				inline Kind kind() const {
					return kind_;
				}
				
				inline std::vector<TemplateVar*>& templateVariables() {
					return templateVariables_;
				}
				
				inline const std::vector<TemplateVar*>& templateVariables() const {
					return templateVariables_;
				}
				
				inline Locic::StringMap<Var*>& variables() {
					return variables_;
				}
				
				inline const Locic::StringMap<Var*>& variables() const {
					return variables_;
				}
				
				inline Locic::StringMap<Function*>& functions() {
					return functions_;
				}
				
				inline const Locic::StringMap<Function*>& functions() const {
					return functions_;
				}
				
				inline bool isPrimitive() const {
					return kind() == PRIMITIVE;
				}
				
				inline bool isStructDecl() const {
					return kind() == STRUCTDECL;
				}
				
				inline bool isStructDef() const {
					return kind() == STRUCTDEF;
				}
				
				inline bool isStruct() const {
					return isStructDecl() || isStructDef();
				}
				
				inline bool isClassDecl() const {
					return kind() == CLASSDECL;
				}
				
				inline bool isClassDef() const {
					return kind() == CLASSDEF;
				}
				
				inline bool isClass() const {
					return isClassDecl() || isClassDef();
				}
				
				inline bool isInterface() const {
					return kind() == INTERFACE;
				}
				
				inline bool isDeclaration() const {
					return isStructDecl() || isClassDecl();
				}
				
				inline bool isDefinition() const {
					return isStructDef() || isClassDef();
				}
				
				void unifyToKind(Kind newKind) {
					if(newKind == kind()) return;
					switch(newKind) {
						case STRUCTDEF: {
							assert(kind() == STRUCTDECL);
							kind_ = newKind;
							return;
						}
						case CLASSDEF: {
							assert(kind() == CLASSDECL);
							kind_ = newKind;
							return;
						}
						default:
							assert(false && "Cannot unify type instance kinds.");
							return;
					}
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
