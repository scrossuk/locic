#ifndef LOCIC_SEM_TYPEINSTANCE_HPP
#define LOCIC_SEM_TYPEINSTANCE_HPP

#include <string>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>

#include <Locic/SEM/Object.hpp>
#include <Locic/SEM/Var.hpp>

namespace Locic {

	namespace SEM {
	
		class Function;
		class TemplateVar;
		
		class TypeInstance: public Object {
			public:
				enum Kind {
					PRIMITIVE,
					STRUCTDECL,
					STRUCTDEF,
					CLASSDECL,
					CLASSDEF,
					INTERFACE,
					TEMPLATETYPE
				};
				
				inline TypeInstance(Kind k, const Name& n)
					: kind_(k), name_(n) { }
				
				inline ObjectKind objectKind() const {
					return OBJECT_TYPEINSTANCE;
				}
					
				inline const Name& name() const {
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
				
				inline std::vector<Var*>& variables() {
					return variables_;
				}
				
				inline const std::vector<Var*>& variables() const {
					return variables_;
				}
				
				inline std::vector<Function*>& functions() {
					return functions_;
				}
				
				inline const std::vector<Function*>& functions() const {
					return functions_;
				}
				
				inline std::vector<Type*>& constructTypes() {
					return constructTypes_;
				}
				
				inline const std::vector<Type*>& constructTypes() const {
					return constructTypes_;
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
				
				inline bool isTemplateType() const {
					return kind() == TEMPLATETYPE;
				}
				
				inline void unifyToKind(Kind newKind) {
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
				
				// TODO: 'type properties' should be moved out of SEM tree
				//       representation into Semantic Analysis nodes.
				inline bool hasProperty(const std::string& propertyName) const {
					return typeProperties_.has(propertyName);
				}
				
				inline Function* getProperty(const std::string& propertyName) const {
					return typeProperties_.get(propertyName);
				}
				
				inline void addProperty(const std::string& propertyName, Function* function) {
					typeProperties_.insert(propertyName, function);
				}
				
				std::string refToString() const;
				
				std::string toString() const;
				
			private:
				Kind kind_;
				Name name_;
				
				StringMap<Function*> typeProperties_;
				
				std::vector<TemplateVar*> templateVariables_;
				std::vector<Var*> variables_;
				std::vector<Function*> functions_;
				std::vector<Type*> constructTypes_;
				
		};
		
	}
	
}

#endif
