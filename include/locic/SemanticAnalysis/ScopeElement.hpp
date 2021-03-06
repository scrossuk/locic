#ifndef LOCIC_SEMANTICANALYSIS_SCOPEELEMENT_HPP
#define LOCIC_SEMANTICANALYSIS_SCOPEELEMENT_HPP

#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		class Alias;
		class CatchClause;
		class Function;
		class Namespace;
		class Scope;
		class SwitchCase;
		class TypeInstance;
		
	}
	
	namespace SemanticAnalysis {
		
		class ScopeElement {
			public:
				enum Kind {
					NAMESPACE,
					ALIAS,
					TYPEINSTANCE,
					FUNCTION,
					SCOPE,
					SWITCHCASE,
					CATCHCLAUSE,
					LOOP,
					SCOPEACTION,
					TRYSCOPE,
					ASSERTNOEXCEPT
				};
				
				static ScopeElement Namespace(AST::Namespace& nameSpace);
				
				static ScopeElement Alias(AST::Alias& alias);
				
				static ScopeElement TypeInstance(AST::TypeInstance& typeInstance);
				
				static ScopeElement Function(AST::Function& function);
				
				static ScopeElement Scope(AST::Scope& scope);
				
				static ScopeElement SwitchCase(AST::SwitchCase& switchCase);
				
				static ScopeElement CatchClause(AST::CatchClause& catchClause);
				
				static ScopeElement Loop();
				
				static ScopeElement ScopeAction(const String& state);
				
				static ScopeElement TryScope();
				
				static ScopeElement AssertNoExcept();
				
				Kind kind() const;
				
				bool isNamespace() const;
				bool isAlias() const;
				bool isTypeInstance() const;
				bool isFunction() const;
				bool isScope() const;
				bool isSwitchCase() const;
				bool isCatchClause() const;
				bool isLoop() const;
				bool isScopeAction() const;
				bool isTryScope() const;
				bool isAssertNoExcept() const;
				
				AST::Namespace& nameSpace() const;
				AST::Alias& alias() const;
				AST::TypeInstance& typeInstance() const;
				AST::Function& function() const;
				AST::Scope& scope() const;
				AST::SwitchCase& switchCase() const;
				AST::CatchClause& catchClause() const;
				const String& scopeActionState() const;
				
				bool hasName() const;
				const String& name() const;
				
			private:
				ScopeElement(Kind pKind);
				
				Kind kind_;
				
				union {
					void* ptr;
					AST::Namespace* nameSpace;
					AST::Alias* alias;
					AST::TypeInstance* typeInstance;
					AST::Function* function;
					AST::Scope* scope;
					AST::SwitchCase* switchCase;
					AST::CatchClause* catchClause;
					String scopeActionState;
				} data_;
				
		};
		
	}
	
}

#endif
