#include <memory>
#include <unordered_map>

#include <locic/AST/Node.hpp>
#include <locic/AST/ValueDecl.hpp>
#include <locic/SemanticAnalysis/AliasTypeResolver.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		class AliasResolveInfo {
		public:
			AliasResolveInfo(ScopeStack argScopeStack)
			: scopeStack_(std::move(argScopeStack)),
			isResolving_(false) { }
			
			AliasResolveInfo(AliasResolveInfo&&) = default;
			AliasResolveInfo& operator=(AliasResolveInfo&&) = default;
			
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
		
		Diag
		AliasDependsOnItselfDiag(Name name) {
			return Error("alias '%s' depends on itself via a cycle",
			             name.toString(/*addPrefix=*/false).c_str());
		}
		
		class AliasTypeResolverImpl {
		public:
			AliasTypeResolverImpl(Context& context)
			: context_(context) { }
			
			void addAlias(const AST::Alias& alias,
			              ScopeStack scopeStack) {
				AliasResolveInfo resolveInfo(std::move(scopeStack));
				resolveMap_.insert(std::make_pair(&alias,
				                                  std::move(resolveInfo)));
			}
			
			const AST::Type* resolveAliasType(AST::Alias& alias) {
				if (alias.type() != nullptr) {
					return alias.type();
				}
				
				assert(resolveMap_.find(&alias) != resolveMap_.end());
				
				auto& resolveInfo = resolveMap_.at(&alias);
				
				if (resolveInfo.isResolving()) {
					context_.issueDiag(AliasDependsOnItselfDiag(alias.fullName().copy()),
					                   alias.location());
					const auto voidType = context_.typeBuilder().getVoidType();
					alias.setType(voidType);
					return voidType;
				}
				
				resolveInfo.setIsResolving();
				
				SwapScopeStack swapScopeStack(context_.scopeStack(), resolveInfo.scopeStack());
				
				alias.setValue(ConvertValue(context_, alias.valueDecl()));
				
				return alias.type();
			}
			
		private:
			Context& context_;
			std::unordered_map<const AST::Alias*, AliasResolveInfo> resolveMap_;
			
		};
		
		AliasTypeResolver::AliasTypeResolver(Context& context)
		: impl_(new AliasTypeResolverImpl(context)) { }
		
		AliasTypeResolver::~AliasTypeResolver() { }
		
		void AliasTypeResolver::addAlias(const AST::Alias& alias,
		                                 ScopeStack scopeStack) {
			impl_->addAlias(alias, std::move(scopeStack));
		}
		
		const AST::Type* AliasTypeResolver::resolveAliasType(AST::Alias& alias) {
			return impl_->resolveAliasType(alias);
		}
		
	}
	
}
