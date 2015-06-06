#ifndef LOCIC_SEMANTICANALYSIS_CONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_CONTEXT_HPP

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <locic/SEM/TemplateVarMap.hpp>

namespace locic {
	
	template <typename T>
	class Optional;
	class SharedMaps;
	class String;
	class StringHost;
	
	namespace Debug {
		
		class Module;
		class SourceLocation;
		
	}
	
	namespace SEM {
		
		class Context;
		class Predicate;
		class TemplatedObject;
		class TemplateVar;
		class Type;
		class Value;
		
	}
	
	namespace SemanticAnalysis {
		
		class AliasTypeResolver;
		class MethodSet;
		class ScopeStack;
		class TemplateInst;
		
		class Context {
			public:
				Context(const SharedMaps& sharedMaps, Debug::Module& debugModule, SEM::Context& semContext);
				~Context();
				
				AliasTypeResolver& aliasTypeResolver();
				const SharedMaps& sharedMaps() const;
				
				String getCString(const char* cString) const;
				String getString(const std::string& string) const;
				
				Debug::Module& debugModule();
				
				ScopeStack& scopeStack();
				const ScopeStack& scopeStack() const;
				
				SEM::Context& semContext();
				
				const MethodSet* findMethodSet(const SEM::TemplatedObject& templatedObject, const SEM::Type* type) const;
				void addMethodSet(const SEM::TemplatedObject& templatedObject, const SEM::Type* type, const MethodSet* methodSet);
				
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
				std::vector<TemplateInst>& templateInstantiations();
				bool templateRequirementsComplete() const;
				void setTemplateRequirementsComplete();
				
				bool methodSetsComplete() const;
				void setMethodSetsComplete();
				
				const std::set<String>& validVarArgTypes() const;
				
				const MethodSet* getMethodSet(MethodSet methodSet) const;
				
				Optional<bool> getCapability(const SEM::Type* type, const String& capability) const;
				
				void setCapability(const SEM::Type* type, const String& capability, bool isCapable);
				
				// For handling cycles in method set computation.
				bool isComputingMethodSet(const SEM::TemplateVar* templateVar, const SEM::Predicate& predicate) const;
				void pushComputingMethodSet(const SEM::TemplateVar* templateVar, const SEM::Predicate& predicate);
				void popComputingMethodSet();
				
				// For handling cycles in require predicates.
				bool isAssumedSatisfies(const SEM::Type* checkType, const SEM::Type* requireType) const;
				void pushAssumeSatisfies(const SEM::Type* checkType, const SEM::Type* requireType);
				void popAssumeSatisfies();
				
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
