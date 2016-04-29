#ifndef LOCIC_AST_TEMPLATETYPEVAR_HPP
#define LOCIC_AST_TEMPLATETYPEVAR_HPP

#include <string>

#include <locic/AST/Node.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace AST {
	
		struct TemplateTypeVar {
			Node<TypeDecl> varType;
			String name;
			Node<TypeDecl> specType;
			
			static TemplateTypeVar* NoSpec(Node<TypeDecl> varType, const String& name) {
				TemplateTypeVar* typeVar = new TemplateTypeVar();
				typeVar->varType = std::move(varType);
				typeVar->name = name;
				typeVar->specType = makeNode(Debug::SourceLocation::Null(), TypeDecl::Void());
				return typeVar;
			}
			
			static TemplateTypeVar* WithSpec(Node<TypeDecl> varType, const String& name, Node<TypeDecl> specType) {
				assert(!specType.isNull());
				TemplateTypeVar* typeVar = new TemplateTypeVar();
				typeVar->varType = std::move(varType);
				typeVar->name = name;
				typeVar->specType = std::move(specType);
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
