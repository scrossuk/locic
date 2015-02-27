#ifndef LOCIC_SEM_VALUE_HPP
#define LOCIC_SEM_VALUE_HPP

#include <unordered_map>
#include <vector>

#include <locic/Map.hpp>
#include <locic/Optional.hpp>
#include <locic/Debug/ValueInfo.hpp>
#include <locic/SEM/ExitStates.hpp>
#include <locic/SEM/TemplateVarMap.hpp>
#include <locic/SEM/TypeArray.hpp>
#include <locic/String.hpp>

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
					SELF,
					THIS,
					CONSTANT,
					LOCALVAR,
					UNIONTAG,
					SIZEOF,
					UNIONDATAOFFSET,
					MEMBEROFFSET,
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
					TEMPLATEFUNCTIONREF,
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
					std::unique_ptr<Value> operand;
				} unionTag;
				
				struct {
					const Type* targetType;
				} sizeOf;
				
				struct {
					const TypeInstance* typeInstance;
				} unionDataOffset;
				
				struct {
					const TypeInstance* typeInstance;
					size_t memberIndex;
				} memberOffset;
				
				struct {
					std::unique_ptr<Value> value;
				} reinterpretValue;
				
				struct {
					std::unique_ptr<Value> value;
				} derefReference;
				
				struct {
					std::unique_ptr<Value> condition, ifTrue, ifFalse;
				} ternary;
				
				struct {
					const Type* targetType;
					std::unique_ptr<Value> value;
				} cast;
				
				struct {
					const Type* targetType;
					std::unique_ptr<Value> value;
				} polyCast;
				
				struct {
					const Type* targetType;
					std::unique_ptr<Value> value;
				} makeLval;
				
				struct {
					std::unique_ptr<Value> value;
				} makeNoLval;
				
				struct {
					const Type* targetType;
					std::unique_ptr<Value> value;
				} makeRef;
				
				struct {
					std::unique_ptr<Value> value;
				} makeNoRef;
				
				struct {
					const Type* targetType;
					std::unique_ptr<Value> value;
				} makeStaticRef;
				
				struct {
					std::unique_ptr<Value> value;
				} makeNoStaticRef;
				
				struct {
					std::vector<Value> parameters;
				} internalConstruct;
				
				struct {
					std::unique_ptr<Value> object;
					Var* memberVar;
				} memberAccess;
				
				struct {
					std::unique_ptr<Value> value;
				} refValue;
				
				struct {
					const Type* targetType;
				} typeRef;
				
				struct {
					std::unique_ptr<Value> functionValue;
					std::vector<Value> parameters;
				} functionCall;
				
				struct {
					const Type* parentType;
					Function* function;
					TypeArray templateArguments;
				} functionRef;
				
				struct {
					const Type* parentType;
					String name;
					const Type* functionType;
					TemplateVarMap templateVarMap;
				} templateFunctionRef;
				
				struct {
					std::unique_ptr<Value> method;
					std::unique_ptr<Value> methodOwner;
				} methodObject;
				
				struct {
					std::unique_ptr<Value> methodValue;
					std::vector<Value> parameters;
				} methodCall;
				
				struct {
					std::unique_ptr<Value> method;
					std::unique_ptr<Value> methodOwner;
				} interfaceMethodObject;
				
				struct {
					std::unique_ptr<Value> method;
					std::unique_ptr<Value> typeRef;
				} staticInterfaceMethodObject;
				
				static Value Self(const Type* type);
				
				static Value This(const Type* type);
				
				static Value Constant(const Constant* constant, const Type* type);
				
				static Value LocalVar(Var* var, const Type* type);
				
				static Value UnionTag(Value operand, const Type* type);
				
				static Value SizeOf(const Type* targetType, const Type* sizeType);
				
				static Value UnionDataOffset(const TypeInstance* typeInstance, const Type* sizeType);
				
				static Value MemberOffset(const TypeInstance* typeInstance, size_t memberIndex, const Type* sizeType);
				
				static Value Reinterpret(Value operand, const Type* type);
				
				static Value DerefReference(Value operand);
				
				static Value Ternary(Value condition, Value ifTrue, Value ifFalse);
				
				static Value Cast(const Type* targetType, Value operand);
				
				static Value PolyCast(const Type* targetType, Value operand);
				
				static Value Lval(const Type* targetType, Value operand);
				
				static Value NoLval(Value operand);
				
				static Value Ref(const Type* targetType, Value operand);
				
				static Value NoRef(Value operand);
				
				static Value StaticRef(const Type* targetType, Value operand);
				
				static Value NoStaticRef(Value operand);
				
				static Value InternalConstruct(TypeInstance* typeInstance, std::vector<Value> parameters);
				
				static Value MemberAccess(Value object, Var* var, const Type* type);
				
				static Value RefValue(Value operand, const Type* type);
				
				static Value TypeRef(const Type* targetType, const Type* type);
				
				static Value FunctionCall(Value functionValue, std::vector<Value> parameters);
				
				static Value FunctionRef(const Type* parentType, Function* function, TypeArray templateArguments, const Type* const type);
				
				static Value TemplateFunctionRef(const Type* parentType, const String& name, const Type* functionType);
				
				static Value MethodObject(Value method, Value methodOwner);
				
				static Value InterfaceMethodObject(Value method, Value methodOwner);
				
				static Value StaticInterfaceMethodObject(Value method, Value typeRef);
				
				static Value CastDummy(const Type* type);
				
				Value();
				
				Value(Value&&) = default;
				Value& operator=(Value&&) = default;
				
				Kind kind() const;
				
				Value copy() const;
				
				const Type* type() const;
				
				ExitStates exitStates() const;
				
				void setDebugInfo(Debug::ValueInfo debugInfo);
				Optional<Debug::ValueInfo> debugInfo() const;
				
				std::string toString() const;
				
			private:
				Value(Kind k, const Type* t, ExitStates exitStates);
				
				// Non-copyable.
				Value(const Value&) = delete;
				Value& operator=(const Value&) = delete;
				
				Kind kind_;
				ExitStates exitStates_;
				const Type* type_;
				Optional<Debug::ValueInfo> debugInfo_;
				
		};
		
	}
	
}

#endif
