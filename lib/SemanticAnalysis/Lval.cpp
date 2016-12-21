#include <stdexcept>

#include <locic/AST/Type.hpp>
#include <locic/Debug.hpp>


#include <locic/SemanticAnalysis/CallValue.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/GetMethod.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeCapabilities.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		const AST::Type* makeValueLvalType(Context& context, const AST::Type* const valueType) {
			return getBuiltInType(context, context.getCString("value_lval_t"), { valueType })->createLvalType();
		}
		
		const AST::Type* makeFinalLvalType(Context& context, const AST::Type* const valueType) {
			return getBuiltInType(context, context.getCString("final_lval_t"), { valueType })->createLvalType();
		}
		
		const AST::Type* makeLvalType(Context& context, const bool isFinal, const AST::Type* const valueType) {
			if (getDerefType(valueType)->isLval()) {
				return valueType;
			}
			
			if (isFinal) {
				return makeFinalLvalType(context, valueType);
			} else {
				return makeValueLvalType(context, valueType);
			}
		}
		
		AST::Value dissolveLval(Context& context, AST::Value value, const Debug::SourceLocation& location) {
			assert(value.type()->isLval() || (value.type()->isRef() && value.type()->isBuiltInReference() && value.type()->refTarget()->isLval()));
			if (!value.type()->isRef()) {
				value = bindReference(context, std::move(value));
			}
			return CallValue(context, GetSpecialMethod(context, std::move(value), context.getCString("dissolve"), location), {}, location);
		}
		
		AST::Value tryDissolveValue(Context& context, AST::Value value, const Debug::SourceLocation& location) {
			if (getSingleDerefType(value.type())->isLval()) {
				return dissolveLval(context, std::move(value), location);
			} else {
				return value;
			}
		}
		
	}
	
}


