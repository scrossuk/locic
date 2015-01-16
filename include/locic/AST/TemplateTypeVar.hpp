#ifndef LOCIC_AST_TEMPLATETYPEVAR_HPP
#define LOCIC_AST_TEMPLATETYPEVAR_HPP

#include <string>
#include <locic/String.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/Type.hpp>

namespace locic {

	namespace AST {
	
		struct TemplateTypeVar {
			Node<Type> varType;
			std::string name;
			Node<Type> specType;
			
			static TemplateTypeVar* NoSpec(Node<Type> varType, const std::string& name) {
				TemplateTypeVar* typeVar = new TemplateTypeVar();
				typeVar->varType = varType;
				typeVar->name = name;
				typeVar->specType = makeNode(Debug::SourceLocation::Null(), Type::Void());
				return typeVar;
			}
			
			static TemplateTypeVar* WithSpec(Node<Type> varType, const std::string& name, Node<Type> specType) {
				assert(!specType.isNull());
				TemplateTypeVar* typeVar = new TemplateTypeVar();
				typeVar->varType = varType;
				typeVar->name = name;
				typeVar->specType = specType;
				return typeVar;
			}
			
			TemplateTypeVar() { }
				
			std::string toString() const {
				return makeString("TemplateTypeVar(varType = %s, name = %s, specType = %s)",
					varType.toString().c_str(), name.c_str(), specType.toString().c_str());
			}
		};
		
		typedef std::vector<Node<TemplateTypeVar>> TemplateTypeVarList;
		
	}
	
}

#endif
