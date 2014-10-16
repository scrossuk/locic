#ifndef LOCIC_AST_FUNCTION_HPP
#define LOCIC_AST_FUNCTION_HPP

#include <string>
#include <vector>

#include <locic/Name.hpp>

#include <locic/AST/Node.hpp>
#include <locic/AST/Scope.hpp>
#include <locic/AST/Symbol.hpp>
#include <locic/AST/TemplateTypeVar.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeVar.hpp>

namespace locic {

	namespace AST {
	
		struct Function {
			public:
				static Function* Decl(bool isVarArg, bool isStatic, bool isConst, bool isNoExcept, const Node<Type>& returnType, const Node<Name>& name, const Node<TypeVarList>& parameters);
				
				static Function* Def(bool isStatic, bool isConst, bool isNoExcept, const Node<Type>& returnType, const Node<Name>& name, const Node<TypeVarList>& parameters, const Node<Scope>& scope);
				
				static Function* StaticMethodDecl(bool isNoExcept, const Node<Type>& returnType, const Node<Name>& name, const Node<TypeVarList>& parameters);
				
				static Function* MethodDecl(bool isConst, bool isNoExcept, const Node<Type>& returnType, const Node<Name>& name, const Node<TypeVarList>& parameters);
				
				static Function* DefaultStaticMethodDef(const Node<Name>& name);
				
				static Function* DefaultMethodDef(const Node<Name>& name);
				
				static Function* Destructor(const Node<Name>& name, const Node<Scope>& scope);
				
				static Function* StaticMethodDef(bool isNoExcept, const Node<Type>& returnType, const Node<Name>& name, const Node<TypeVarList>& parameters, const Node<Scope>& scope);
				
				static Function* MethodDef(bool isConst, bool isNoExcept, const Node<Type>& returnType, const Node<Name>& name, const Node<TypeVarList>& parameters, const Node<Scope>& scope);
				
				bool isDeclaration() const;
				bool isDefinition() const;
				bool isDefaultDefinition() const;
				
				bool isConst() const;
				bool isStatic() const;
				bool isVarArg() const;
				bool isNoExcept() const;
				bool isImported() const;
				bool isExported() const;
				
				const Node<Name>& name() const;
				
				const Node<TemplateTypeVarList>& templateVariables() const;
				const Node<Type>& returnType() const;
				const Node<TypeVarList>& parameters() const;
				const Node<Scope>& scope() const;
				
				void setTemplateVariables(const Node<TemplateTypeVarList>& pTemplateVariables);
				void setImport();
				void setExport();
				
				std::string toString() const;
				
			private:
				explicit Function(const Node<Name>& pName);
				
				bool isDefinition_, isDefaultDefinition_, isVarArg_;
				bool isConst_, isStatic_, isNoExcept_;
				bool isImported_, isExported_;
				
				Node<Name> name_;
				Node<TemplateTypeVarList> templateVariables_;
				Node<Type> returnType_;
				Node<TypeVarList> parameters_;
				Node<Scope> scope_;
				
		};
		
		typedef std::vector<Node<Function>> FunctionList;
		
	}
	
}

#endif
