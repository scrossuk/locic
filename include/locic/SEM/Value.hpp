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
		
		typedef std::unordered_map<TemplateVar*, const Type*> TemplateVarMap;
	
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
					Value* operand;
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
					Value* value;
				} reinterpretValue;
				
				struct {
					Value* value;
				} derefReference;
				
				struct {
					Value* condition, * ifTrue, * ifFalse;
				} ternary;
				
				struct {
					const Type* targetType;
					Value* value;
				} cast;
				
				struct {
					const Type* targetType;
					Value* value;
				} polyCast;
				
				struct {
					const Type* targetType;
					Value* value;
				} makeLval;
				
				struct {
					Value* value;
				} makeNoLval;
				
				struct {
					const Type* targetType;
					Value* value;
				} makeRef;
				
				struct {
					Value* value;
				} makeNoRef;
				
				struct {
					const Type* targetType;
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
					const Type* targetType;
				} typeRef;
				
				struct {
					Value* functionValue;
					std::vector<Value*> parameters;
				} functionCall;
				
				struct {
					const Type* parentType;
					Function* function;
					std::vector<const Type*> templateArguments;
				} functionRef;
				
				struct {
					const Type* parentType;
					std::string name;
					const Type* functionType;
				} templateFunctionRef;
				
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
				
				static Value* Self(const Type* type);
				
				static Value* This(const Type* type);
				
				static Value* Constant(const Constant* constant, const Type* type);
				
				static Value* LocalVar(Var* var, const Type* type);
				
				static Value* UnionTag(Value* operand, const Type* type);
				
				static Value* SizeOf(const Type* targetType, const Type* sizeType);
				
				static Value* UnionDataOffset(const TypeInstance* typeInstance, const Type* sizeType);
				
				static Value* MemberOffset(const TypeInstance* typeInstance, size_t memberIndex, const Type* sizeType);
				
				static Value* Reinterpret(Value* operand, const Type* type);
				
				static Value* DerefReference(Value* operand);
				
				static Value* Ternary(Value* condition, Value* ifTrue, Value* ifFalse);
				
				static Value* Cast(const Type* targetType, Value* operand);
				
				static Value* PolyCast(const Type* targetType, Value* operand);
				
				static Value* Lval(const Type* targetType, Value* operand);
				
				static Value* NoLval(Value* operand);
				
				static Value* Ref(const Type* targetType, Value* operand);
				
				static Value* NoRef(Value* operand);
				
				static Value* StaticRef(const Type* targetType, Value* operand);
				
				static Value* NoStaticRef(Value* operand);
				
				static Value* InternalConstruct(TypeInstance* typeInstance, const std::vector<Value*>& parameters);
				
				static Value* MemberAccess(Value* object, Var* var, const Type* type);
				
				static Value* RefValue(Value* operand, const Type* type);
				
				static Value* TypeRef(const Type* targetType, const Type* type);
				
				static Value* FunctionCall(Value* functionValue, const std::vector<Value*>& parameters);
				
				static Value* FunctionRef(const Type* parentType, Function* function, const std::vector<const Type*>& templateArguments, const TemplateVarMap& templateVarMap);
				
				static Value* TemplateFunctionRef(const Type* parentType, const std::string& name, const Type* functionType);
				
				static Value* MethodObject(Value* method, Value* methodOwner);
				
				static Value* InterfaceMethodObject(Value* method, Value* methodOwner);
				
				static Value* StaticInterfaceMethodObject(Value* method, Value* typeRef);
				
				static Value* CastDummy(const Type* type);
				
				Kind kind() const;
				
				const Type* type() const;
				
				std::string toString() const;
				
			private:
				// Value();
				Value(Kind k, const Type* t);
				
				Kind kind_;
				const Type* type_;
				
		};
		
	}
	
}

#endif
