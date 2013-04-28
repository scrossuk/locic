#ifndef LOCIC_SEMANTICANALYSIS_CANCAST_HPP
#define LOCIC_SEMANTICANALYSIS_CANCAST_HPP

#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/Exception.hpp>

namespace Locic {

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
		
		SEM::Value* PolyCastValueToType(SEM::Value* value, SEM::Type* type);
		
		SEM::Value* ImplicitCast(SEM::Value* value, SEM::Type* type);
		
		SEM::Type* UnifyTypes(SEM::Type* first, SEM::Type* second);
		
		inline bool CanDoImplicitCast(SEM::Type* sourceType, SEM::Type* destType) {
			try {
				(void) ImplicitCast(SEM::Value::CastDummy(sourceType), destType);
				return true;
			} catch(const CastException& e) {
				return false;
			}
		}
		
		class CastTypeMismatchException: public CastException {
			public:
				inline CastTypeMismatchException(SEM::Type* sourceType, SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				inline std::string toString() const {
					return makeString("Fundamental type mismatch in cast from type '%s' to type '%s'.",
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				SEM::Type* sourceType_;
				SEM::Type* destType_;
				
		};
		
		class CastConstCorrectnessViolationException: public CastException {
			public:
				inline CastConstCorrectnessViolationException(SEM::Type* sourceType, SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				inline std::string toString() const {
					return makeString("Const-correctness violation in cast from type '%s' to type '%s'.",
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				SEM::Type* sourceType_;
				SEM::Type* destType_;
				
		};
		
		class CastLValueToRValueException: public CastException {
			public:
				inline CastLValueToRValueException(SEM::Type* sourceType, SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				inline std::string toString() const {
					return makeString("Unable to convert lvalue to rvalue in cast from type '%s' to type '%s'.",
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				SEM::Type* sourceType_;
				SEM::Type* destType_;
				
		};
		
		class CastRValueToReferenceException: public CastException {
			public:
				inline CastRValueToReferenceException(SEM::Type* sourceType, SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				inline std::string toString() const {
					return makeString("Unable to convert rvalue to reference in cast from type '%s' to type '%s'.",
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				SEM::Type* sourceType_;
				SEM::Type* destType_;
				
		};
		
		class CastObjectTypeMismatchException: public CastException {
			public:
				inline CastObjectTypeMismatchException(SEM::Type* sourceType, SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				inline std::string toString() const {
					return makeString("Object types don't match in cast from type '%s' to type '%s'.",
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				SEM::Type* sourceType_;
				SEM::Type* destType_;
				
		};
		
		class CastConstChainingViolationException: public CastException {
			public:
				inline CastConstChainingViolationException(SEM::Type* sourceType, SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				inline std::string toString() const {
					return makeString("Const chaining violation in cast from type '%s' to type '%s'; "
							"a template argument target type can only be cast to const if ALL of its parent types are also const.",
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				SEM::Type* sourceType_;
				SEM::Type* destType_;
				
		};
		
		class CastFunctionParameterNumberMismatchException: public CastException {
			public:
				inline CastFunctionParameterNumberMismatchException(SEM::Type* sourceType, SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				inline std::string toString() const {
					return makeString("Function parameter counts don't match in cast from type '%s' to type '%s'.",
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				SEM::Type* sourceType_;
				SEM::Type* destType_;
				
		};
		
		class CastFunctionVarArgsMismatchException: public CastException {
			public:
				inline CastFunctionVarArgsMismatchException(SEM::Type* sourceType, SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				inline std::string toString() const {
					return makeString("Function 'varargs' property doesn't match in cast from type '%s' to type '%s'.",
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				SEM::Type* sourceType_;
				SEM::Type* destType_;
				
		};
		
		class PolyCastMissingMethodException: public CastException {
			public:
				inline PolyCastMissingMethodException(SEM::Type* sourceType, SEM::Type* destType,
						SEM::Function* methodFunction)
					: sourceType_(sourceType), destType_(destType),
					  methodFunction_(methodFunction) { }
					  
				inline std::string toString() const {
					return makeString("Method '%s' missing in source type of polymorphic cast (from type '%s' to type '%s').",
							methodFunction_->name().c_str(),
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				SEM::Type* sourceType_;
				SEM::Type* destType_;
				SEM::Function* methodFunction_;
				
		};
		
		class PolyCastMethodMismatchException: public CastException {
			public:
				inline PolyCastMethodMismatchException(SEM::Type* sourceType, SEM::Type* destType,
						SEM::Function* sourceMethod, SEM::Function* destMethod)
					: sourceType_(sourceType), destType_(destType),
					  sourceMethod_(sourceMethod), destMethod_(destMethod) { }
					  
				inline std::string toString() const {
					return makeString("Method '%s' doesn't have matching types ('%s' vs '%s') in polymorphic cast (from type '%s' to type '%s').",
							sourceMethod_->name().c_str(),
							sourceMethod_->type()->toString().c_str(),
							destMethod_->type()->toString().c_str(),
							sourceType_->toString().c_str(),
							destType_->toString().c_str());
				}
				
			private:
				SEM::Type* sourceType_;
				SEM::Type* destType_;
				SEM::Function* sourceMethod_;
				SEM::Function* destMethod_;
				
		};
		
	}
	
}

#endif
