#ifndef LOCIC_SEMANTICANALYSIS_CASTEXCEPTION_HPP
#define LOCIC_SEMANTICANALYSIS_CASTEXCEPTION_HPP

#include <string>

#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/Support/MakeString.hpp>
#include <locic/Support/Name.hpp>

namespace locic {
	
	namespace SEM {
		
		class Type;
		class Value;
		
	}
	
	namespace SemanticAnalysis {
		
		class CastTypeMismatchException: public CastException {
			public:
				CastTypeMismatchException(SEM::Value* value, const SEM::Type* sourceType, const SEM::Type* destType)
					: value_(value), sourceType_(sourceType), destType_(destType) { }
					
				std::string toString() const {
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
				CastConstCorrectnessViolationException(const SEM::Type* sourceType, const SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				std::string toString() const {
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
				CastLValueToRValueException(const SEM::Type* sourceType, const SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				std::string toString() const {
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
				CastRValueToReferenceException(const SEM::Type* sourceType, const SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				std::string toString() const {
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
				CastObjectTypeMismatchException(const SEM::Type* sourceType, const SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				std::string toString() const {
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
				CastConstChainingViolationException(const SEM::Type* sourceType, const SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				std::string toString() const {
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
				CastFunctionParameterNumberMismatchException(const SEM::Type* sourceType, const SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				std::string toString() const {
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
				CastFunctionVarArgsMismatchException(const SEM::Type* sourceType, const SEM::Type* destType)
					: sourceType_(sourceType), destType_(destType) { }
					
				std::string toString() const {
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
				PolyCastMissingMethodException(const SEM::Type* sourceType, const SEM::Type* destType,
						SEM::Function* methodFunction)
					: sourceType_(sourceType), destType_(destType),
					  methodFunction_(methodFunction) { }
					  
				std::string toString() const {
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
				PolyCastMethodMismatchException(Name methodName,
						const SEM::Type* sourceType, const SEM::Type* destType,
						const SEM::Type* sourceMethodType, const SEM::Type* destMethodType)
					: methodName_(std::move(methodName)),
					  sourceType_(sourceType), destType_(destType),
					  sourceMethodType_(sourceMethodType), destMethodType_(destMethodType) { }
					  
				std::string toString() const {
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
