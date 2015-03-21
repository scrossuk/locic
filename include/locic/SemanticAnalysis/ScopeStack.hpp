#ifndef LOCIC_SEMANTICANALYSIS_SCOPESTACK_HPP
#define LOCIC_SEMANTICANALYSIS_SCOPESTACK_HPP

#include <vector>

#include <locic/Debug.hpp>
#include <locic/Support/Name.hpp>
#include <locic/SEM.hpp>
#include <locic/Support/String.hpp>

#include <locic/SemanticAnalysis/ScopeElement.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		class ScopeStack {
		public:
			typedef Array<ScopeElement, 8> ArrayType;
			
			ScopeStack() { }
			
			ScopeStack(ArrayType argArray)
			: array_(std::move(argArray)) { }
			
			ScopeStack copy() const {
				return ScopeStack(array_.copy());
			}
			
			bool empty() const {
				return array_.empty();
			}
			
			size_t size() const {
				return array_.size();
			}
			
			ScopeElement& back() {
				return array_.back();
			}
			
			const ScopeElement& back() const {
				return array_.back();
			}
			
			void push_back(ScopeElement element) {
				array_.push_back(std::move(element));
			}
			
			void pop_back() {
				array_.pop_back();
			}
			
			ScopeElement& operator[](const size_t index) {
				return array_[index];
			}
			
			const ScopeElement& operator[](const size_t index) const {
				return array_[index];
			}
			
		private:
			ArrayType array_;
			
		};
		
		class PushScopeElement {
			public:
				inline PushScopeElement(ScopeStack& stack, ScopeElement element)
					: stack_(stack) {
						stack_.push_back(std::move(element));
					}
					
				inline ~PushScopeElement() {
					stack_.pop_back();
				}
				
			private:
				// Non-copyable.
				PushScopeElement(const PushScopeElement&) = delete;
				PushScopeElement& operator=(const PushScopeElement&) = delete;
				
				ScopeStack& stack_;
				
		};
		
		Name getCurrentName(const ScopeStack& scopeStack);
		
		SEM::TypeInstance* lookupParentType(const ScopeStack& scopeStack);
		
		SEM::Function* lookupParentFunction(const ScopeStack& scopeStack);
		
		const SEM::TemplatedObject& lookupTemplatedObject(const ScopeStack& scopeStack);
		
		const SEM::Type* getParentFunctionReturnType(const ScopeStack& scopeStack);
		
		class Context;
		
		const SEM::Type* getBuiltInType(Context& context, const String& typeName, SEM::TypeArray templateArgs);
		
	}
	
}

#endif
