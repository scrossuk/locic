#ifndef LOCIC_SEMANTICANALYSIS_CANCAST_HPP
#define LOCIC_SEMANTICANALYSIS_CANCAST_HPP

#include <locic/Debug.hpp>
#include <locic/Optional.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
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
		
		SEM::Value ImplicitCast(Context& context, SEM::Value value, const SEM::Type* type, const Debug::SourceLocation& location, bool formatOnly = false);
		
		const SEM::Type* ImplicitCastTypeFormatOnly(const SEM::Type* sourceType, const SEM::Type* destType, const Debug::SourceLocation& location);
		
		Optional<SEM::Value> ImplicitCastFormatOnly(SEM::Value value, const SEM::Type* destType, const Debug::SourceLocation& location);
		
		bool TypeSatisfiesInterface(Context& context, const SEM::Type* objectType, const SEM::Type* interfaceType);
		
		const SEM::Type* UnifyTypes(Context& context, const SEM::Type* first, const SEM::Type* second, const Debug::SourceLocation& location);
		
		bool CanDoImplicitCast(Context& context, const SEM::Type* sourceType, const SEM::Type* destType, const Debug::SourceLocation& location);
		
		class CastTypeMismatchException: public CastException {
			public:
				inline CastTypeMismatchException(SEM::Value* value, const SEM::Type* sourceType, const SEM::Type* destType)
					: value_(value), sourceType_(sourceType), destType_(destType) { }
					
				inline std::string toString() const {
					return makeString("Fundamental type mismatch in cast of value '%s' from type '%s' to type '%s'.",
							value_->toString().c_str(),
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				SEM::Value* value_;
				const SEM::Type* sourceType_;
				const SEM::Type* destType_;
				
		};
		
		class CastConstCorrectnessViolationException: public CastException {
			public:
				inline CastConstCorrectnessViolationException(const SEM::Type* sourceType, const SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				inline std::string toString() const {
					return makeString("Const-correctness violation in cast from type '%s' to type '%s'.",
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				const SEM::Type* sourceType_;
				const SEM::Type* destType_;
				
		};
		
		class CastLValueToRValueException: public CastException {
			public:
				inline CastLValueToRValueException(const SEM::Type* sourceType, const SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				inline std::string toString() const {
					return makeString("Unable to convert lvalue to rvalue in cast from type '%s' to type '%s'.",
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				const SEM::Type* sourceType_;
				const SEM::Type* destType_;
				
		};
		
		class CastRValueToReferenceException: public CastException {
			public:
				inline CastRValueToReferenceException(const SEM::Type* sourceType, const SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				inline std::string toString() const {
					return makeString("Unable to convert rvalue to reference in cast from type '%s' to type '%s'.",
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				const SEM::Type* sourceType_;
				const SEM::Type* destType_;
				
		};
		
		class CastObjectTypeMismatchException: public CastException {
			public:
				inline CastObjectTypeMismatchException(const SEM::Type* sourceType, const SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				inline std::string toString() const {
					return makeString("Object types don't match in cast from type '%s' to type '%s'.",
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				const SEM::Type* sourceType_;
				const SEM::Type* destType_;
				
		};
		
		class CastConstChainingViolationException: public CastException {
			public:
				inline CastConstChainingViolationException(const SEM::Type* sourceType, const SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				inline std::string toString() const {
					return makeString("Const chaining violation in cast from type '%s' to type '%s'; "
							"a template argument target type can only be cast to const if ALL of its parent types are also const.",
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				const SEM::Type* sourceType_;
				const SEM::Type* destType_;
				
		};
		
		class CastFunctionParameterNumberMismatchException: public CastException {
			public:
				inline CastFunctionParameterNumberMismatchException(const SEM::Type* sourceType, const SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				inline std::string toString() const {
					return makeString("Function parameter counts don't match in cast from type '%s' to type '%s'.",
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				const SEM::Type* sourceType_;
				const SEM::Type* destType_;
				
		};
		
		class CastFunctionVarArgsMismatchException: public CastException {
			public:
				inline CastFunctionVarArgsMismatchException(const SEM::Type* sourceType, const SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				inline std::string toString() const {
					return makeString("Function 'varargs' property doesn't match in cast from type '%s' to type '%s'.",
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				const SEM::Type* sourceType_;
				const SEM::Type* destType_;
				
		};
		
		class PolyCastMissingMethodException: public CastException {
			public:
				inline PolyCastMissingMethodException(const SEM::Type* sourceType, const SEM::Type* destType,
						SEM::Function* methodFunction)
					: sourceType_(sourceType), destType_(destType),
					  methodFunction_(methodFunction) { }
					  
				inline std::string toString() const {
					return makeString("Method '%s' missing in source type of polymorphic cast (from type '%s' to type '%s').",
							methodFunction_->name().toString().c_str(),
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				const SEM::Type* sourceType_;
				const SEM::Type* destType_;
				SEM::Function* methodFunction_;
				
		};
		
		class PolyCastMethodMismatchException: public CastException {
			public:
				inline PolyCastMethodMismatchException(const Name& methodName,
						const SEM::Type* sourceType, const SEM::Type* destType,
						const SEM::Type* sourceMethodType, const SEM::Type* destMethodType)
					: methodName_(methodName),
					  sourceType_(sourceType), destType_(destType),
					  sourceMethodType_(sourceMethodType), destMethodType_(destMethodType) { }
					  
				inline std::string toString() const {
					return makeString("Method '%s' doesn't have matching types ('%s' vs '%s') in polymorphic cast (from type '%s' to type '%s').",
							methodName_.toString().c_str(),
							sourceMethodType_->toString().c_str(),
							destMethodType_->toString().c_str(),
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				Name methodName_;
				const SEM::Type* sourceType_;
				const SEM::Type* destType_;
				const SEM::Type* sourceMethodType_;
				const SEM::Type* destMethodType_;
				
		};
		
	}
	
}

#endif
