#ifndef LOCIC_SEMANTICANALYSIS_CAST_HPP
#define LOCIC_SEMANTICANALYSIS_CAST_HPP

#include <locic/Debug.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace AST {
		
		class Type;
		class Value;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		/**
		 * Types of value casting/converting:
		 *    1. Implicit cast - allows implicit copying, casting to void,
		 *                       null construction, const qualification,
		 *                       polymorphic casts and co-variant/
		 *                       contra-variant function casts.
		 *    2. Explicit cast - allows using 'cast' constructor on top of
		 *                       implicit cast.
		 *    3. Static cast - allows potentially-valid pointer casts
		 *                     to occur at compile-time (e.g. cast from
		 *                     'void *' to an object-type pointer).
		 *    4. Const cast - for adding/removing const.
		 *    5. Dynamic cast - cast up/down class-interface hierarchy.
		 *    6. Reinterpret cast - to 'reinterpret' the byte contents of a value.
		 */
		
		AST::Value ImplicitCast(Context& context, AST::Value value, const AST::Type* type, const Debug::SourceLocation& location, bool formatOnly = false);

		const AST::Type*
		ImplicitCastTypeFormatOnly(Context& context, const AST::Type* sourceType, const AST::Type* destType,
		                           const Debug::SourceLocation& location);

		Optional<AST::Value>
		ImplicitCastFormatOnly(Context& context, AST::Value value, const AST::Type* destType,
		                       const Debug::SourceLocation& location);
		
		const AST::Type* UnifyTypes(Context& context, const AST::Type* first, const AST::Type* second, const Debug::SourceLocation& location);
		
		bool CanDoImplicitCast(Context& context, const AST::Type* sourceType, const AST::Type* destType, const Debug::SourceLocation& location);
		
	}
	
}

#endif
