#ifndef LOCIC_SEMANTICANALYSIS_SCOPEELEMENT_HPP
#define LOCIC_SEMANTICANALYSIS_SCOPEELEMENT_HPP

#include <string>

#include <locic/SEM.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		class ScopeElement {
			public:
				enum Kind {
					NAMESPACE,
					TYPEINSTANCE,
					FUNCTION,
					SCOPE,
					SWITCHCASE,
					CATCHCLAUSE,
					LOOP,
					SCOPEACTION,
					TRYSCOPE
				};
				
				static ScopeElement Namespace(SEM::Namespace* nameSpace);
				
				static ScopeElement TypeInstance(SEM::TypeInstance* typeInstance);
				
				static ScopeElement Function(SEM::Function* function);
				
				static ScopeElement Scope(SEM::Scope* scope);
				
				static ScopeElement SwitchCase(SEM::SwitchCase* switchCase);
				
				static ScopeElement CatchClause(SEM::CatchClause* catchClause);
				
				static ScopeElement Loop();
				
				static ScopeElement ScopeAction(const std::string& state);
				
				static ScopeElement TryScope();
				
				Kind kind() const;
				
				bool isNamespace() const;
				bool isTypeInstance() const;
				bool isFunction() const;
				bool isScope() const;
				bool isSwitchCase() const;
				bool isCatchClause() const;
				bool isLoop() const;
				bool isScopeAction() const;
				bool isTryScope() const;
				
				SEM::Namespace* nameSpace() const;
				SEM::TypeInstance* typeInstance() const;
				SEM::Function* function() const;
				SEM::Scope* scope() const;
				SEM::SwitchCase* switchCase() const;
				SEM::CatchClause* catchClause() const;
				const std::string& scopeActionState() const;
				
				bool hasName() const;
				std::string name() const;
				
			private:
				ScopeElement(Kind pKind);
				
				Kind kind_;
				
				union {
					void* ptr;
					SEM::Namespace* nameSpace;
					SEM::TypeInstance* typeInstance;
					SEM::Function* function;
					SEM::Scope* scope;
					SEM::SwitchCase* switchCase;
					SEM::CatchClause* catchClause;
				} data_;
				
				std::string scopeActionState_;
				
		};
		
	}
	
}

#endif
