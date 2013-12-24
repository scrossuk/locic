#ifndef LOCIC_AST_TEMPLATETYPEVAR_HPP
#define LOCIC_AST_TEMPLATETYPEVAR_HPP

#include <string>
#include <Locic/String.hpp>
#include <Locic/AST/Node.hpp>
#include <Locic/AST/Type.hpp>

namespace Locic {

	namespace AST {
	
		struct TemplateTypeVar {
			enum Kind {
				TYPENAME,
				POLYMORPHIC
			} kind;
			std::string name;
			Node<Type> specType;
			
			inline static TemplateTypeVar* Typename(const std::string& name) {
				TemplateTypeVar* typeVar = new TemplateTypeVar(TYPENAME);
				typeVar->name = name;
				typeVar->specType = makeNode(NullLocation(), Type::Void());
				return typeVar;
			}
			
			inline static TemplateTypeVar* TypenameSpec(const std::string& name, Node<Type> specType) {
				assert(!specType.isNull());
				TemplateTypeVar* typeVar = new TemplateTypeVar(TYPENAME);
				typeVar->name = name;
				typeVar->specType = specType;
				return typeVar;
			}
			
			inline static TemplateTypeVar* Polymorphic(const std::string& name) {
				TemplateTypeVar* typeVar = new TemplateTypeVar(POLYMORPHIC);
				typeVar->name = name;
				typeVar->specType = makeNode(NullLocation(), Type::Void());
				return typeVar;
			}
			
			inline TemplateTypeVar(Kind k)
				: kind(k) { }
				
			inline std::string toString() const {
				return makeString("TemplateTypeVar(name = %s, specType = %s)",
										 name.c_str(), specType.toString().c_str());
			}
		};
		
		typedef std::vector<Node<TemplateTypeVar>> TemplateTypeVarList;
		
	}
	
}

#endif
