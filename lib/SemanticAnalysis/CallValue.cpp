#include <locic/Constant.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/CallValue.hpp>
#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/GetMethod.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>
#include <locic/SemanticAnalysis/TypeCapabilities.hpp>
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
		
			HeapArray<SEM::Value> CastFunctionArguments(Context& context, HeapArray<SEM::Value> arguments,
			                                            const SEM::TypeArray& types, const Debug::SourceLocation& location) {
				HeapArray<SEM::Value> castValues;
				castValues.reserve(arguments.size());
				
				for (size_t i = 0; i < arguments.size(); i++) {
					auto& argumentValue = arguments.at(i);
					const auto& argLocation = argumentValue.debugInfo() ?
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
		
		class TypeNotCallableDiag: public Error {
		public:
			TypeNotCallableDiag(const SEM::Type* type)
			: typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("type '%s' is not callable; it needs a 'call' method",
				                  typeString_.c_str());
			}
			
		private:
			std::string typeString_;
			
		};
		
		class CallIncorrectArgCountDiag: public Error {
		public:
			CallIncorrectArgCountDiag(std::string valueString,
			                          size_t argsGiven, size_t argsRequired)
			: valueString_(std::move(valueString)), argsGiven_(argsGiven),
			argsRequired_(argsRequired) { }
			
			std::string toString() const {
				return makeString("function '%s' called with %llu "
				                  "parameter(s); expected %llu",
				                  valueString_.c_str(),
				                  static_cast<unsigned long long>(argsGiven_),
				                  static_cast<unsigned long long>(argsRequired_));
			}
			
		private:
			std::string valueString_;
			size_t argsGiven_;
			size_t argsRequired_;
			
		};
		
		class VarArgTooFewArgsDiag: public Error {
		public:
			VarArgTooFewArgsDiag(std::string valueString,
			                     size_t argsGiven, size_t argsRequired)
			: valueString_(std::move(valueString)), argsGiven_(argsGiven),
			argsRequired_(argsRequired) { }
			
			std::string toString() const {
				return makeString("vararg function '%s' called with %llu "
				                  "parameter(s); expected at least %llu",
				                  valueString_.c_str(),
				                  static_cast<unsigned long long>(argsGiven_),
				                  static_cast<unsigned long long>(argsRequired_));
			}
			
		private:
			std::string valueString_;
			size_t argsGiven_;
			size_t argsRequired_;
			
		};
		
		SEM::Value CallValue(Context& context, SEM::Value rawValue, HeapArray<SEM::Value> args, const Debug::SourceLocation& location) {
			auto value = tryDissolveValue(context, derefValue(std::move(rawValue)), location);
			
			if (getDerefType(value.type())->isStaticRef()) {
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
					return SEM::Value::Constant(Constant::Integer(0), context.typeBuilder().getIntType());
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
			
			return addDebugInfo(SEM::Value::Call(std::move(value), CastFunctionArguments(context, std::move(args), typeList, location)), location);
		}
		
	}
	
}


