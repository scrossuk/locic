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
#include <locic/AST/TemplateTypeVar.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/TypeVar.hpp>

namespace locic {
	
	namespace SEM {
		
		class Function;
		
	}
	
	namespace AST {
		
		struct Function {
			public:
				static Function* Decl(bool isVarArg, bool isStatic,
						Node<TypeDecl> returnType, Node<Name> name, Node<TypeVarList> parameters,
						Node<ConstSpecifier> constSpecifier,
						Node<RequireSpecifier> noexceptSpecifier,
						Node<RequireSpecifier> requireSpecifier);
				
				static Function* Def(bool isVarArg, bool isStatic,
						Node<TypeDecl> returnType, Node<Name> name, Node<TypeVarList> parameters,
						Node<Scope> scope,
						Node<ConstSpecifier> constSpecifier,
						Node<RequireSpecifier> noexceptSpecifier,
						Node<RequireSpecifier> requireSpecifier);
				
				static Function* StaticDecl(Node<TypeDecl> returnType,
				                            Node<Name> name,
				                            Node<TypeVarList> parameters,
				                            Node<RequireSpecifier> noexceptSpecifier,
				                            Node<RequireSpecifier> requireSpecifier);
				
				static Function* StaticDef(Node<TypeDecl> returnType,
				                           Node<Name> name,
				                           Node<TypeVarList> parameters,
				                           Node<Scope> scope,
				                           Node<RequireSpecifier> noexceptSpecifier,
				                           Node<RequireSpecifier> requireSpecifier);
				
				static Function* DefaultStaticMethodDef(Node<Name> name, Node<RequireSpecifier> requireSpecifier);
				
				static Function* DefaultMethodDef(Node<Name> name, Node<RequireSpecifier> requireSpecifier);
				
				static Function* Destructor(Node<Name> name, Node<Scope> scope);
				
				bool isDeclaration() const;
				bool isDefinition() const;
				bool isDefaultDefinition() const;
				
				bool isStatic() const;
				bool isVarArg() const;
				bool isNoExcept() const;
				bool isImported() const;
				bool isExported() const;
				bool isPrimitive() const;
				
				const Node<Name>& name() const;
				
				const Node<TemplateTypeVarList>& templateVariables() const;
				const Node<TypeDecl>& returnType() const;
				const Node<TypeVarList>& parameters() const;
				const Node<Scope>& scope() const;
				const Node<ConstSpecifier>& constSpecifier() const;
				const Node<RequireSpecifier>& noexceptSpecifier() const;
				const Node<RequireSpecifier>& requireSpecifier() const;
				
				void setTemplateVariables(Node<TemplateTypeVarList> templateVariables);
				void setRequireSpecifier(Node<RequireSpecifier> requireSpecifier);
				void setImport();
				void setExport();
				void setPrimitive();
				
				void setSEMFunction(SEM::Function& function);
				SEM::Function& semFunction();
				
				std::string toString() const;
				
			private:
				explicit Function(Node<Name> pName);
				
				bool isDefinition_, isDefaultDefinition_;
				bool isVarArg_, isStatic_;
				bool isImported_, isExported_;
				bool isPrimitive_;
				
				Node<Name> name_;
				Node<TemplateTypeVarList> templateVariables_;
				Node<TypeDecl> returnType_;
				Node<TypeVarList> parameters_;
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
