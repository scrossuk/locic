#ifndef LOCIC_AST_FUNCTION_HPP
#define LOCIC_AST_FUNCTION_HPP

#include <string>
#include <vector>

#include <locic/Support/Name.hpp>

#include <locic/AST/ConstSpecifier.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/Scope.hpp>
#include <locic/AST/Symbol.hpp>
#include <locic/AST/TemplateVar.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/Var.hpp>

namespace locic {
	
	namespace SEM {
		
		class Function;
		
	}
	
	namespace AST {
		
		class Function {
			public:
				Function();
				
				bool isDeclaration() const;
				bool isDefinition() const;
				void setIsDefinition(bool value);
				
				bool isDefaultDefinition() const;
				void setIsDefaultDefinition(bool value);
				
				bool isStatic() const;
				void setIsStatic(bool value);
				
				bool isVarArg() const;
				void setIsVarArg(bool value);
				
				bool isImported() const;
				void setIsImported(bool value);
				
				bool isExported() const;
				void setIsExported(bool value);
				
				bool isPrimitive() const;
				void setIsPrimitive(bool value);
				
				const Node<Name>& name() const;
				void setName(Node<Name> name);
				
				const Node<TypeDecl>& returnType() const;
				void setReturnType(Node<TypeDecl> returnType);
				
				const Node<VarList>& parameters() const;
				void setParameters(Node<VarList> parameters);
				
				const Node<Scope>& scope() const;
				void setScope(Node<Scope> scope);
				
				const Node<ConstSpecifier>& constSpecifier() const;
				void setConstSpecifier(Node<ConstSpecifier> constSpecifier);
				
				const Node<RequireSpecifier>& noexceptSpecifier() const;
				void setNoexceptSpecifier(Node<RequireSpecifier> noexceptSpecifier);
				
				const Node<RequireSpecifier>& requireSpecifier() const;
				void setRequireSpecifier(Node<RequireSpecifier> requireSpecifier);
				
				const Node<TemplateVarList>& templateVariables() const;
				void setTemplateVariables(Node<TemplateVarList> templateVariables);
				
				void setSEMFunction(SEM::Function& function);
				SEM::Function& semFunction();
				
				std::string toString() const;
				
			private:
				bool isDefinition_, isDefaultDefinition_;
				bool isVarArg_, isStatic_;
				bool isImported_, isExported_;
				bool isPrimitive_;
				
				Node<Name> name_;
				Node<TemplateVarList> templateVariables_;
				Node<TypeDecl> returnType_;
				Node<VarList> parameters_;
				Node<Scope> scope_;
				Node<ConstSpecifier> constSpecifier_;
				Node<RequireSpecifier> noexceptSpecifier_;
				Node<RequireSpecifier> requireSpecifier_;
				SEM::Function* semFunction_;
				
		};
		
		typedef std::vector<Node<Function>> FunctionList;
		
	}
	
}

#endif
