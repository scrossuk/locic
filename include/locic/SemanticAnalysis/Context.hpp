#ifndef LOCIC_SEMANTICANALYSIS_CONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_CONTEXT_HPP

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <locic/AST/TemplateVarMap.hpp>
#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Frontend/OptionalDiag.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	class DiagnosticReceiver;
	class SharedMaps;
	class String;
	class StringHost;
	
	namespace AST {
		
		class TemplatedObject;
		class Type;
		class Value;
		
	}
	
	namespace Debug {
		
		class Module;
		class SourceLocation;
		
	}
	
	namespace SEM {
		
		class Context;
		class Predicate;
		
	}
	
	namespace SemanticAnalysis {
		
		class AliasTypeResolver;
		class MethodSet;
		class ScopeStack;
		class TemplateInst;
		class TypeBuilder;
		
		class Context {
			public:
				Context(const SharedMaps& sharedMaps, Debug::Module& debugModule,
				        SEM::Context& semContext, DiagnosticReceiver& diagReceiver);
				~Context();
				
				DiagnosticReceiver& diagnosticReceiver();
				void setDiagnosticReceiver(DiagnosticReceiver& receiver);
				
				template <typename DiagType>
				void issueDiag(DiagType diag, const Debug::SourceLocation& location,
				               OptionalDiag chain = OptionalDiag()) {
					issueDiagPtr(std::unique_ptr<Diag>(new DiagType(std::move(diag))),
					             location, std::move(chain));
				}
				
				void issueDiagPtr(std::unique_ptr<Diag> diag,
				                  const Debug::SourceLocation& location,
				                  OptionalDiag chain);
				
				AliasTypeResolver& aliasTypeResolver();
				const SharedMaps& sharedMaps() const;
				
				String getCString(const char* cString) const;
				String getString(const std::string& string) const;
				
				Debug::Module& debugModule();
				
				ScopeStack& scopeStack();
				const ScopeStack& scopeStack() const;
				
				SEM::Context& semContext();
				
				TypeBuilder& typeBuilder();
				
				const MethodSet* findMethodSet(const AST::TemplatedObject* templatedObject,
				                               const AST::Type* type) const;
				
				void addMethodSet(const AST::TemplatedObject* templatedObject,
				                  const AST::Type* type,
				                  const MethodSet* methodSet);
				
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
				
				const MethodSet* getMethodSet(MethodSet methodSet) const;
				
				Optional<bool> getCapability(const AST::Type* type, const String& capability) const;
				
				void setCapability(const AST::Type* type, const String& capability, bool isCapable);
				
				// For handling cycles in method set computation.
				bool isComputingMethodSet(const AST::TemplateVar* templateVar, const SEM::Predicate& predicate) const;
				void pushComputingMethodSet(const AST::TemplateVar* templateVar, const SEM::Predicate& predicate);
				void popComputingMethodSet();
				
				// For handling cycles in require predicates.
				bool isAssumedSatisfies(const AST::Type* checkType, const AST::Type* requireType) const;
				void pushAssumeSatisfies(const AST::Type* checkType, const AST::Type* requireType);
				void popAssumeSatisfies();
				
			private:
				// Non-copyable.
				Context(const Context&) = delete;
				Context& operator=(const Context&) = delete;
				
				std::unique_ptr<class ContextImpl> impl_;
				
		};
		
		AST::Value getSelfValue(Context& context, const Debug::SourceLocation& location);
		
		AST::Value getThisValue(Context& context, const Debug::SourceLocation& location);
		
	}
	
}

#endif
