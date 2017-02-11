#include <locic/AST/Type.hpp>
#include <locic/Constant.hpp>

#include <locic/SemanticAnalysis/CallValue.hpp>
#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/GetMethod.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>
#include <locic/SemanticAnalysis/TypeCapabilities.hpp>
#include <locic/SemanticAnalysis/VarArgCast.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		namespace {
			
			AST::Value addDebugInfo(AST::Value value, const Debug::SourceLocation& location) {
				Debug::ValueInfo valueInfo;
				valueInfo.location = location;
				value.setDebugInfo(valueInfo);
				return value;
			}
		
			HeapArray<AST::Value> CastFunctionArguments(Context& context, HeapArray<AST::Value> arguments,
			                                            const AST::TypeArray& types, const Debug::SourceLocation& location) {
				HeapArray<AST::Value> castValues;
				castValues.reserve(arguments.size());
				
				for (size_t i = 0; i < arguments.size(); i++) {
					auto& argumentValue = arguments.at(i);
					const auto argLocation = argumentValue.debugInfo() ?
						argumentValue.debugInfo()->location : location;
					
					// Cast arguments to the function type's corresponding
					// argument type; var-arg arguments should be cast to
					// one of the allowed types (since there's no specific
					// destination type).
					auto castArgumentValue = (i < types.size()) ?
						ImplicitCast(context, std::move(argumentValue), types.at(i), argLocation) :
						VarArgCast(context, std::move(argumentValue), argLocation);
					
					castValues.push_back(std::move(castArgumentValue));
				}
				
				return castValues;
			}
			
		}
		
		Diag
		TypeNotCallableDiag(const AST::Type* const type) {
			return Error("type '%s' is not callable; it needs a 'call' method",
			             type->toDiagString().c_str());
		}
		
		Diag
		CallIncorrectArgCountDiag(std::string valueString, const size_t argsGiven,
		                          const size_t argsRequired) {
			return Error("function '%s' called with %zu "
			             "parameter(s); expected %zu",
			             valueString.c_str(), argsGiven,
			             argsRequired);
		}
		
		Diag
		VarArgTooFewArgsDiag(std::string valueString, const size_t argsGiven,
		                     const size_t argsRequired) {
			return Error("vararg function '%s' called with %zu "
			             "parameter(s); expected at least %zu",
			             valueString.c_str(), argsGiven,
			             argsRequired);
		}
		
		Diag
		CallReturnTypeIsUnsizedDiag(const AST::Type* const type) {
			return Error("return type '%s' of function call does not have a size",
			             type->toDiagString().c_str());
		}
		
		AST::Value CallValue(Context& context, AST::Value rawValue, HeapArray<AST::Value> args, const Debug::SourceLocation& location) {
			auto value = derefValue(std::move(rawValue));
			
			if (getDerefType(value.type())->isTypename()) {
				return CallValue(context, GetStaticMethod(context, std::move(value), context.getCString("create"), location), std::move(args), location);
			}
			
			if (!value.type()->isCallable()) {
				// Try to use 'call' method.
				if (TypeCapabilities(context).hasCallMethod(getDerefType(value.type()))) {
					return CallValue(context, GetMethod(context, std::move(value),
					                                    context.getCString("call"), location),
					                 std::move(args), location);
				} else {
					context.issueDiag(TypeNotCallableDiag(getDerefType(value.type())),
					                  location);
					return AST::Value::Constant(Constant::Integer(0), context.typeBuilder().getIntType());
				}
			}
			
			const auto functionType = value.type()->asFunctionType();
			const auto& typeList = functionType.parameterTypes();
			
			if (functionType.attributes().isVarArg()) {
				if (args.size() < typeList.size()) {
					context.issueDiag(VarArgTooFewArgsDiag(value.toDiagString(),
					                                       args.size(), typeList.size()),
					                  location);
				}
			} else {
				if (args.size() != typeList.size()) {
					context.issueDiag(CallIncorrectArgCountDiag(value.toDiagString(),
					                                            args.size(), typeList.size()),
					                  location);
				}
			}
			
			if (!TypeCapabilities(context).isSized(functionType.returnType())) {
				// TODO: also check that the type is not abstract.
				context.issueDiag(CallReturnTypeIsUnsizedDiag(functionType.returnType()),
				                  location);
			}
			
			return addDebugInfo(AST::Value::Call(std::move(value), CastFunctionArguments(context, std::move(args), typeList, location)), location);
		}
		
	}
	
}


