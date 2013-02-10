#include <vector>
#include <Locic/Constant.hpp>
#include <Locic/String.hpp>
#include <Locic/SEM/Function.hpp>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/TypeInstance.hpp>
#include <Locic/SEM/Value.hpp>
#include <Locic/SEM/Var.hpp>

namespace Locic {

	namespace SEM {
	
		std::string Value::toString() const {
			switch(typeEnum) {
				case CONSTANT:
					return makeString("Constant(%s)",
									  constant->toString().c_str());
				case COPY:
					return makeString("Copy(%s)",
									  copyValue.value->toString().c_str());
				case VAR:
					return makeString("Var(%s)",
									  varValue.var->toString().c_str());
				case TERNARY:
					return makeString("Ternary(cond: %s, ifTrue: %s, ifFalse: %s)",
									  ternary.condition->toString().c_str(),
									  ternary.ifTrue->toString().c_str(),
									  ternary.ifFalse->toString().c_str());
				case CAST:
					return makeString("Cast(value: %s, targetType: %s)",
									  cast.value->toString().c_str(),
									  cast.targetType->toString().c_str());
				case POLYCAST:
					return makeString("PolyCast(value: %s, targetType: %s)",
									  polyCast.value->toString().c_str(),
									  polyCast.targetType->toString().c_str());
				case INTERNALCONSTRUCT:
					return makeString("InternalConstruct(args: %s)",
									  makeArrayString(internalConstruct.parameters).c_str());
				case MEMBERACCESS:
					return makeString("MemberAccess(object: %s, memberId: %llu)",
									  memberAccess.object->toString().c_str(),
									  (unsigned long long) memberAccess.memberId);
				case FUNCTIONCALL:
					return makeString("FunctionCall(funcValue: %s, args: %s)",
									  functionCall.functionValue->toString().c_str(),
									  makeArrayString(functionCall.parameters).c_str());
				case FUNCTIONREF:
					return makeString("FunctionRef(%s)",
									  functionRef.function->name.toString().c_str());
				case METHODOBJECT:
					return makeString("MethodObject(object: %s, method: %s)",
									  methodObject.methodOwner->toString().c_str(),
									  methodObject.method->toString().c_str());
				case METHODCALL:
					return makeString("MethodCall(methodObject: %s, args: %s)",
									  methodCall.methodValue->toString().c_str(),
									  makeArrayString(methodCall.parameters).c_str());
				default:
					return "[UNKNOWN VALUE]";
			}
		}
	}
	
}

