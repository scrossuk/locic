#ifndef LOCIC_AST_TEMPLATETYPEVAR_HPP
#define LOCIC_AST_TEMPLATETYPEVAR_HPP

#include <string>
#include <Locic/AST/Node.hpp>
#include <Locic/AST/Type.hpp>

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
			typeVar->specType = AST::Node<Type>();
			return typeVar;
		}
		
		inline static TemplateTypeVar* TypenameSpec(const std::string& name, Node<Type> specType) {
			assert(specType.get() != NULL);
			TemplateTypeVar* typeVar = new TemplateTypeVar(TYPENAME);
			typeVar->name = name;
			typeVar->specType = specType;
			return typeVar;
		}
		
		inline static TemplateTypeVar* Polymorphic(const std::string& name) {
			TemplateTypeVar* typeVar = new TemplateTypeVar(POLYMORPHIC);
			typeVar->name = name;
			typeVar->specType = AST::Node<Type>();
			return typeVar;
		}
		
		inline TemplateTypeVar(Kind k)
			: kind(k) { }
	};
	
	typedef std::vector<Node<TemplateTypeVar>> TemplateTypeVarList;
	
}

#endif
