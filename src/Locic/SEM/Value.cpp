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
			switch(kind()) {
				case CONSTANT:
					return makeString("Constant(%s)",
							constant->toString().c_str());
				case COPY:
					return makeString("Copy(%s)",
							copyValue.value->toString().c_str());
				case VAR:
					return makeString("VarValue(%s)",
							varValue.var->toString().c_str());
				case ADDRESSOF:
					return makeString("AddressOf(%s)",
							addressOf.value->toString().c_str());
				case DEREF_POINTER:
					return makeString("DerefPointer(%s)",
							derefPointer.value->toString().c_str());
				case REFERENCEOF:
					return makeString("ReferenceOf(%s)",
							referenceOf.value->toString().c_str());
				case DEREF_REFERENCE:
					return makeString("DerefReference(%s)",
							derefReference.value->toString().c_str());
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
					return makeString("MemberAccess(object: %s, var: %s)",
							memberAccess.object->toString().c_str(),
							memberAccess.memberVar->toString().c_str());
				case FUNCTIONCALL:
					return makeString("FunctionCall(funcValue: %s, args: %s)",
							functionCall.functionValue->toString().c_str(),
							makeArrayString(functionCall.parameters).c_str());
				case FUNCTIONREF:
					return makeString("FunctionRef(parentType: %s, name: %s)",
							functionRef.parentType != NULL ?
								functionRef.parentType->toString().c_str() :
								"[NONE]",
							functionRef.function->name().toString().c_str());
				case METHODOBJECT:
					return makeString("MethodObject(object: %s, method: %s)",
							methodObject.methodOwner->toString().c_str(),
							methodObject.method->toString().c_str());
				case METHODCALL:
					return makeString("MethodCall(methodObject: %s, args: %s)",
							methodCall.methodValue->toString().c_str(),
							makeArrayString(methodCall.parameters).c_str());
				case INTERFACEMETHODOBJECT:
					return makeString("InterfaceMethodObject(object: %s, method: %s)",
							interfaceMethodObject.methodOwner->toString().c_str(),
							interfaceMethodObject.method->toString().c_str());
				case INTERFACEMETHODCALL:
					return makeString("InterfaceMethodCall(methodObject: %s, args: %s)",
							interfaceMethodCall.methodValue->toString().c_str(),
							makeArrayString(interfaceMethodCall.parameters).c_str());
				case CASTDUMMYOBJECT:
					return "[CAST DUMMY OBJECT (FOR SEMANTIC ANALYSIS)]";
				case NONE:
					return "[NONE]";
				default:
					return makeString("[UNKNOWN VALUE (kind = %u)]", (unsigned int) kind());
			}
		}
	}
	
}

