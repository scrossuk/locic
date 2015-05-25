#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/CallValue.hpp>
#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/GetMethod.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/VarArgCast.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		namespace {
			
			SEM::Value addDebugInfo(SEM::Value value, const Debug::SourceLocation& location) {
				Debug::ValueInfo valueInfo;
				valueInfo.location = location;
				value.setDebugInfo(valueInfo);
				return value;
			}
			
		}
		
		HeapArray<SEM::Value> CastFunctionArguments(Context& context, HeapArray<SEM::Value> arguments, const SEM::TypeArray& types, const Debug::SourceLocation& location) {
			HeapArray<SEM::Value> castValues;
			castValues.reserve(arguments.size());
			
			for (size_t i = 0; i < arguments.size(); i++) {
				auto& argumentValue = arguments.at(i);
				
				// Cast arguments to the function type's corresponding
				// argument type; var-arg arguments should be cast to
				// one of the allowed types (since there's no specific
				// destination type).
				auto castArgumentValue = (i < types.size()) ?
					ImplicitCast(context, std::move(argumentValue), types.at(i), location) :
					VarArgCast(context, std::move(argumentValue), location);
				
				castValues.push_back(std::move(castArgumentValue));
			}
			
			return castValues;
		}
		
		SEM::Value CallValue(Context& context, SEM::Value rawValue, HeapArray<SEM::Value> args, const Debug::SourceLocation& location) {
			auto value = tryDissolveValue(context, derefValue(std::move(rawValue)), location);
			
			if (getDerefType(value.type())->isStaticRef()) {
				return CallValue(context, GetStaticMethod(context, std::move(value), context.getCString("create"), location), std::move(args), location);
			}
			
			if (!value.type()->isCallable()) {
				// Try to use 'call' method.
				return CallValue(context, GetMethod(context, std::move(value), context.getCString("call"), location), std::move(args), location);
			}
			
			const auto functionType = value.type()->asFunctionType();
			const auto& typeList = functionType.parameterTypes();
			
			if (functionType.attributes().isVarArg()) {
				if (args.size() < typeList.size()) {
					throw ErrorException(makeString("Var Arg Function [%s] called with %llu "
						"parameters; expected at least %llu at position %s.",
						value.toString().c_str(),
						(unsigned long long) args.size(),
						(unsigned long long) typeList.size(),
						location.toString().c_str()));
				}
			} else {
				if (args.size() != typeList.size()) {
					throw ErrorException(makeString("Function [%s] called with %llu "
						"parameters; expected %llu at position %s.",
						value.toString().c_str(),
						(unsigned long long) args.size(),
						(unsigned long long) typeList.size(),
						location.toString().c_str()));
				}
			}
			
			return addDebugInfo(SEM::Value::Call(std::move(value), CastFunctionArguments(context, std::move(args), typeList, location)), location);
		}
		
	}
	
}


