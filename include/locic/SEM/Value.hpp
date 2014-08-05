#ifndef LOCIC_SEM_VALUE_HPP
#define LOCIC_SEM_VALUE_HPP

#include <unordered_map>
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
		
		typedef std::unordered_map<TemplateVar*, Type*> TemplateVarMap;
	
		class Value {
			public:
				enum Kind {
					NONE,
					SELF,
					THIS,
					CONSTANT,
					LOCALVAR,
					SIZEOF,
					REINTERPRET,
					DEREF_REFERENCE,
					TERNARY,
					CAST,
					POLYCAST,
					LVAL,
					NOLVAL,
					REF,
					NOREF,
					STATICREF,
					NOSTATICREF,
					INTERNALCONSTRUCT,
					MEMBERACCESS,
					REFVALUE,
					TYPEREF,
					FUNCTIONCALL,
					FUNCTIONREF,
					METHODOBJECT,
					INTERFACEMETHODOBJECT,
					STATICINTERFACEMETHODOBJECT,
					
					// Used by Semantic Analysis to create a 'dummy'
					// value to test if types can be cast.
					CASTDUMMYOBJECT
				};
				
				const locic::Constant* constant;
				
				struct {
					Var* var;
				} localVar;
				
				struct {
					Type* targetType;
				} sizeOf;
				
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
					Value* value;
				} makeNoLval;
				
				struct {
					Type* targetType;
					Value* value;
				} makeRef;
				
				struct {
					Value* value;
				} makeNoRef;
				
				struct {
					Type* targetType;
					Value* value;
				} makeStaticRef;
				
				struct {
					Value* value;
				} makeNoStaticRef;
				
				struct {
					std::vector<Value*> parameters;
				} internalConstruct;
				
				struct {
					Value* object;
					Var* memberVar;
				} memberAccess;
				
				struct {
					Value* value;
				} refValue;
				
				struct {
					Type* targetType;
				} typeRef;
				
				struct {
					Value* functionValue;
					std::vector<Value*> parameters;
				} functionCall;
				
				struct {
					Type* parentType;
					Function* function;
					std::vector<Type*> templateArguments;
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
					Value* method;
					Value* typeRef;
				} staticInterfaceMethodObject;
				
				static Value* Self(Type* type);
				
				static Value* This(Type* type);
				
				static Value* Constant(const Constant* constant, Type* type);
				
				static Value* LocalVar(Var* var, Type* type);
				
				static Value* SizeOf(Type* targetType, Type* sizeType);
				
				static Value* Reinterpret(Value* operand, Type* type);
				
				static Value* DerefReference(Value* operand);
				
				static Value* Ternary(Value* condition, Value* ifTrue, Value* ifFalse);
				
				static Value* Cast(Type* targetType, Value* operand);
				
				static Value* PolyCast(Type* targetType, Value* operand);
				
				static Value* Lval(Type* targetType, Value* operand);
				
				static Value* NoLval(Value* operand);
				
				static Value* Ref(Type* targetType, Value* operand);
				
				static Value* NoRef(Value* operand);
				
				static Value* StaticRef(Type* targetType, Value* operand);
				
				static Value* NoStaticRef(Value* operand);
				
				static Value* InternalConstruct(TypeInstance* typeInstance, const std::vector<Value*>& parameters);
				
				static Value* MemberAccess(Value* object, Var* var, Type* type);
				
				static Value* RefValue(Value* operand, Type* type);
				
				static Value* TypeRef(Type* targetType, Type* type);
				
				static Value* FunctionCall(Value* functionValue, const std::vector<Value*>& parameters);
				
				static Value* FunctionRef(Type* parentType, Function* function, const std::vector<Type*>& templateArguments, const TemplateVarMap& templateVarMap);
				
				static Value* MethodObject(Value* method, Value* methodOwner);
				
				static Value* InterfaceMethodObject(Value* method, Value* methodOwner);
				
				static Value* StaticInterfaceMethodObject(Value* method, Value* typeRef);
				
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
