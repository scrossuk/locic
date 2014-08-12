#ifndef LOCIC_SEMANTICANALYSIS_CONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_CONTEXT_HPP

#include <set>
#include <tuple>

#include <locic/Debug.hpp>

#include <locic/SEM/Context.hpp>

#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		using TemplateInstTuple = std::tuple<SEM::TemplateVar*, SEM::Type*, SEM::TemplateVarMap, Name, Debug::SourceLocation>;
		
		class Context {
			public:
				Context(Debug::Module& pDebugModule, SEM::Context& pSemContext);
				
				Debug::Module& debugModule();
				
				ScopeStack& scopeStack();
				const ScopeStack& scopeStack() const;
				
				SEM::Context& semContext();
				
				/**
				 * \brief Maintains a list of pairs of a template var
				 *        and a type used to instantiate it.
				 * 
				 * This allows early Semantic Analysis passes to instantiate
				 * templates without checking whether they are valid,
				 * since the specification type isn't generated until a
				 * later pass. A subsequent pass then re-visits these
				 * instantiations to check they're valid.
				 */
				std::vector<TemplateInstTuple>& templateInstantiations();
				
				const std::set<std::string>& validVarArgTypes() const;
			
			private:
				// Non-copyable.
				Context(const Context&) = delete;
				Context& operator=(const Context&) = delete;
				
				Debug::Module& debugModule_;
				ScopeStack scopeStack_;
				SEM::Context& semContext_;
				std::vector<TemplateInstTuple> templateInstantiations_;
				std::set<std::string> validVarArgTypes_;
				
		};
		
		SEM::Value* getSelfValue(Context& context, const Debug::SourceLocation& location);
		
	}
	
}

#endif
