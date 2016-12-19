#ifndef LOCIC_SEMANTICANALYSIS_ALIASTYPERESOLVER_HPP
#define LOCIC_SEMANTICANALYSIS_ALIASTYPERESOLVER_HPP

#include <memory>

#include <locic/AST/Node.hpp>

namespace locic {
	
	namespace AST {
		
		class Alias;
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		class ScopeStack;
		
		/**
		 * \brief Alias Type Resolver
		 * 
		 * Working out the types of aliases is actually a non-trivial
		 * problem, since they don't have a statically encoded type and
		 * converting one alias value may require converting another
		 * value. For example:
		 * 
		 *   using A = B;
		 *   using B = ...;
		 * 
		 * Hence this class builds up a mapping from aliases to their
		 * AST values, so that we can start resolving an alias and then
		 * resolve aliases it depends on at the same time. We also have
		 * to be careful to produce an error in the case of circular
		 * dependencies, such as:
		 * 
		 *   using A = B;
		 *   using B = A;
		 * 
		 * For this class, we're not actually interested in the values
		 * themselves, we're just looking to work out the type 
		 */
		class AliasTypeResolver {
		public:
			AliasTypeResolver(Context& context);
			~AliasTypeResolver();
			
			void addAlias(const AST::Alias& alias,
			              ScopeStack scopeStack);
			
			const AST::Type* resolveAliasType(AST::Alias& alias);
			
		private:
			std::unique_ptr<class AliasTypeResolverImpl> impl_;
			
		};
		
	}
	
}

#endif
