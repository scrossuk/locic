#ifndef LOCIC_SEM_VALUE_HPP
#define LOCIC_SEM_VALUE_HPP

#include <vector>

#include <locic/Map.hpp>

namespace locic {
	
	class Constant;

	namespace SEM {
		
		class Function;
		class TemplateVar;
		class Type;
		class TypeInstance;
		class Var;
	
		class Value {
			public:
				enum Kind {
					NONE,
					CONSTANT,
					LOCALVAR,
					MEMBERVAR,
					REINTERPRET,
					DEREF_REFERENCE,
					TERNARY,
					CAST,
					POLYCAST,
					LVAL,
					REF,
					INTERNALCONSTRUCT,
					MEMBERACCESS,
					FUNCTIONCALL,
					FUNCTIONREF,
					METHODOBJECT,
					METHODCALL,
					INTERFACEMETHODOBJECT,
					INTERFACEMETHODCALL,
					
					// Used by Semantic Analysis to create a 'dummy'
					// value to test if types can be cast.
					CASTDUMMYOBJECT
				};
				
				locic::Constant* constant;
				
				struct {
					Var* var;
				} localVar;
				
				struct {
					Var* var;
				} memberVar;
				
				struct {
					Value* value;
				} reinterpretValue;
				
				struct {
					Value* value;
				} derefReference;
				
				struct {
					Value* condition, * ifTrue, * ifFalse;
				} ternary;
				
				struct {
					Type* targetType;
					Value* value;
				} cast;
				
				struct {
					Type* targetType;
					Value* value;
				} polyCast;
				
				struct {
					Type* targetType;
					Value* value;
				} makeLval;
				
				struct {
					Type* targetType;
					Value* value;
				} makeRef;
				
				struct {
					std::vector<Value*> parameters;
				} internalConstruct;
				
				struct {
					Value* object;
					Var* memberVar;
				} memberAccess;
				
				struct {
					Value* functionValue;
					std::vector<Value*> parameters;
				} functionCall;
				
				struct {
					Type* parentType;
					Function* function;
				} functionRef;
				
				struct {
					Value* method;
					Value* methodOwner;
				} methodObject;
				
				struct {
					Value* methodValue;
					std::vector<Value*> parameters;
				} methodCall;
				
				struct {
					Value* method;
					Value* methodOwner;
				} interfaceMethodObject;
				
				struct {
					Value* methodValue;
					std::vector<Value*> parameters;
				} interfaceMethodCall;
				
				static Value* Constant(Constant* constant, SEM::Type* type);
				
				static Value* LocalVar(Var* var);
				
				static Value* MemberVar(Var* var);
				
				static Value* Reinterpret(Value* operand, Type* type);
				
				static Value* DerefReference(Value* operand);
				
				static Value* Ternary(Value* condition, Value* ifTrue, Value* ifFalse);
				
				static Value* Cast(Type* targetType, Value* operand);
				
				static Value* PolyCast(Type* targetType, Value* operand);
				
				static Value* Lval(Type* targetType, Value* operand);
				
				static Value* Ref(Type* targetType, Value* operand);
				
				static Value* InternalConstruct(TypeInstance* typeInstance, const std::vector<Value*>& parameters);
				
				static Value* MemberAccess(Value* object, Var* var, Type* type);
				
				static Value* FunctionCall(Value* functionValue, const std::vector<Value*>& parameters);
				
				static Value* FunctionRef(Type* parentType, Function* function,
					const Map<TemplateVar*, Type*>& templateVarMap);
				
				static Value* MethodObject(Value* method, Value* methodOwner);
				
				static Value* MethodCall(Value* methodValue, const std::vector<Value*>& parameters);
				
				static Value* InterfaceMethodObject(Value* method, Value* methodOwner);
				
				static Value* InterfaceMethodCall(Value* methodValue, const std::vector<Value*>& parameters);
				
				static Value* CastDummy(Type* type);
				
				Kind kind() const;
				
				Type* type() const;
				
				std::string toString() const;
				
			private:
				// Value();
				Value(Kind k, Type* t);
				
				Kind kind_;
				Type* type_;
				
		};
		
	}
	
}

#endif
