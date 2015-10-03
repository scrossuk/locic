#include <memory>

#include <locic/AST/Node.hpp>
#include <locic/AST/Value.hpp>
#include <locic/SemanticAnalysis/AliasTypeResolver.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {
	
	namespace SEM {
		
		class Alias;
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class AliasResolveInfo {
		public:
			AliasResolveInfo(AST::Node<AST::Value> argASTValue,
			                 ScopeStack argScopeStack)
			: astValue_(std::move(argASTValue)),
			scopeStack_(std::move(argScopeStack)),
			isResolving_(false) { }
			
			AliasResolveInfo(AliasResolveInfo&&) = default;
			AliasResolveInfo& operator=(AliasResolveInfo&&) = default;
			
			const AST::Node<AST::Value>& astValue() const {
				return astValue_;
			}
			
			ScopeStack& scopeStack() {
				return scopeStack_;
			}
			
			void setIsResolving() {
				isResolving_ = true;
			}
			
			bool isResolving() const {
				return isResolving_;
			}
			
		private:
			AST::Node<AST::Value> astValue_;
			ScopeStack scopeStack_;
			bool isResolving_;
			
		};
		
		namespace {
			
			class SwapScopeStack {
			public:
				SwapScopeStack(ScopeStack& first,
				               ScopeStack& second)
				: first_(first), second_(second) {
					std::swap(first_, second_);
				}
				
				~SwapScopeStack() {
					std::swap(first_, second_);
				}
				
			private:
				ScopeStack& first_;
				ScopeStack& second_;
				
			};
			
		}
		
		class AliasTypeResolverImpl {
		public:
			AliasTypeResolverImpl(Context& context)
			: context_(context) { }
			
			void addAlias(const SEM::Alias& alias,
			              AST::Node<AST::Value> astValue,
			              ScopeStack scopeStack) {
				AliasResolveInfo resolveInfo(std::move(astValue),
				                             std::move(scopeStack));
				resolveMap_.insert(std::make_pair(&alias,
				                                  std::move(resolveInfo)));
			}
			
			const SEM::Type* resolveAliasType(SEM::Alias& alias) {
				if (alias.type() != nullptr) {
					return alias.type();
				}
				
				assert(resolveMap_.find(&alias) != resolveMap_.end());
				
				auto& resolveInfo = resolveMap_.at(&alias);
				
				if (resolveInfo.isResolving()) {
					throw ErrorException(makeString("Circular dependency of alias '%s'.",
					                                alias.name().toString().c_str()));
				}
				
				resolveInfo.setIsResolving();
				
				SwapScopeStack swapScopeStack(context_.scopeStack(), resolveInfo.scopeStack());
				
				alias.setValue(ConvertValue(context_, resolveInfo.astValue()));
				
				return alias.type();
			}
			
		private:
			Context& context_;
			std::unordered_map<const SEM::Alias*, AliasResolveInfo> resolveMap_;
			
		};
		
		AliasTypeResolver::AliasTypeResolver(Context& context)
		: impl_(new AliasTypeResolverImpl(context)) { }
		
		AliasTypeResolver::~AliasTypeResolver() { }
		
		void AliasTypeResolver::addAlias(const SEM::Alias& alias,
		                                 AST::Node<AST::Value> astValue,
		                                 ScopeStack scopeStack) {
			impl_->addAlias(alias,
			                std::move(astValue),
			                std::move(scopeStack));
		}
		
		const SEM::Type* AliasTypeResolver::resolveAliasType(SEM::Alias& alias) {
			return impl_->resolveAliasType(alias);
		}
		
	}
	
}
