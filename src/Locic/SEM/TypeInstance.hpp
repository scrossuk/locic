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
				enum TypeEnum {
					PRIMITIVE,
					STRUCTDECL,
					STRUCTDEF,
					CLASSDECL,
					CLASSDEF,
					INTERFACE
				};
				
				inline TypeInstance(TypeEnum e, const Locic::Name& n)
					: typeEnum(e), name(n) { }
				
				inline static TypeInstance * Primitive(const Locic::Name& name,
					const std::vector<TemplateVar *>& templateVars){
					
				}
				
				inline bool isAbstract() const {
					return typeEnum == 
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
			
			Type * getFunctionReturnType(const std::string& name);
			
			Type * getImplicitCopyType();
			
			Type * instantiate(const std::vector<Type *>& templateArgs);
			
			std::string toString() const;
			
			private:
				TypeEnum typeEnum_;
				bool isAbstract_;
				Locic::Name name_;
				std::vector<
				Locic::StringMap<Var*> variables_;
				Locic::StringMap<Function*> functions_;
			
		};
		
	}
	
}

#endif
