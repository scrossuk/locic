#ifndef LOCIC_SEMANTICANALYSIS_CONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_CONTEXT_HPP

#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include <locic/SEM/TemplateVarMap.hpp>

namespace locic {
	
	template <typename T>
	class Optional;
	class String;
	class StringHost;
	
	namespace Debug {
		
		class Module;
		class SourceLocation;
		
	}
	
	namespace SEM {
		
		class Context;
		class TemplatedObject;
		class TemplateVar;
		class Type;
		class Value;
		
	}
	
	namespace SemanticAnalysis {
		
		class MethodSet;
		class ScopeStack;
		using TemplateInstTuple = std::tuple<ScopeStack, SEM::TemplateVarMap, const SEM::TemplatedObject*, Name, Debug::SourceLocation>;
		
		class Context {
			public:
				Context(const StringHost& stringHost, Debug::Module& pDebugModule, SEM::Context& pSemContext);
				~Context();
				
				String getCString(const char* cString) const;
				
				Debug::Module& debugModule();
				
				ScopeStack& scopeStack();
				const ScopeStack& scopeStack() const;
				
				SEM::Context& semContext();
				
				const MethodSet* findMethodSet(const SEM::Type* type) const;
				void addMethodSet(const SEM::Type* type, const MethodSet* methodSet);
				
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
				bool templateRequirementsComplete() const;
				void setTemplateRequirementsComplete();
				
				const std::set<String>& validVarArgTypes() const;
				
				const MethodSet* getMethodSet(MethodSet methodSet) const;
				
				Optional<bool> getCapability(const SEM::Type* type, const String& capability) const;
				
				void setCapability(const SEM::Type* type, const String& capability, bool isCapable);
				
			private:
				// Non-copyable.
				Context(const Context&) = delete;
				Context& operator=(const Context&) = delete;
				
				std::unique_ptr<class ContextImpl> impl_;
				
		};
		
		SEM::Value getSelfValue(Context& context, const Debug::SourceLocation& location);
		
		SEM::Value getThisValue(Context& context, const Debug::SourceLocation& location);
		
	}
	
}

#endif
