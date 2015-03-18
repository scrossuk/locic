#ifndef LOCIC_SEM_VALUE_HPP
#define LOCIC_SEM_VALUE_HPP

#include <locic/Constant.hpp>
#include <locic/Debug/ValueInfo.hpp>
#include <locic/SEM/ExitStates.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TemplateVarArray.hpp>
#include <locic/SEM/TypeArray.hpp>
#include <locic/Support/HeapArray.hpp>
#include <locic/Support/Map.hpp>
#include <locic/Support/Optional.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
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
					ZEROINITIALISE,
					MEMCOPY,
					SELF,
					THIS,
					CONSTANT,
					PREDICATE,
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
					BIND_REFERENCE,
					TYPEREF,
					TEMPLATEVARREF,
					CALL,
					FUNCTIONREF,
					TEMPLATEFUNCTIONREF,
					METHODOBJECT,
					INTERFACEMETHODOBJECT,
					STATICINTERFACEMETHODOBJECT,
					
					// Used by Semantic Analysis to create a 'dummy'
					// value to test if types can be cast.
					CASTDUMMYOBJECT
				};
				
				/**
				 * \brief Zero Initialise
				 * 
				 * Zero initialise a data value.
				 */
				static Value ZeroInitialise(const Type* type);
				
				/**
				 * \brief Copy Memory
				 * 
				 * Copy memory from the reference given.
				 */
				static Value MemCopy(Value operand, const Type* type);
				
				/**
				 * \brief Self
				 * 
				 * A reference within a method to the parent object.
				 */
				static Value Self(const Type* type);
				
				/**
				 * \brief This
				 * 
				 * A pointer within a method to the parent object.
				 */
				static Value This(const Type* type);
				
				/**
				 * \brief Constant
				 * 
				 * A constant value (e.g. 0, 1.2, true etc.).
				 */
				static Value Constant(Constant constant, const Type* type);
				
				/**
				 * \brief Predicate
				 * 
				 * A predicate value.
				 */
				static Value PredicateExpr(Predicate value, const Type* boolType);
				
				/**
				 * \brief Local Variable
				 * 
				 * A reference to a local variable.
				 */
				static Value LocalVar(const Var& var, const Type* type);
				
				/**
				 * \brief Union Tag
				 * 
				 * The tag value of a union, which is basically an enum
				 * value that identifies which variant of a union
				 * datatype a value contains.
				 */
				static Value UnionTag(Value operand, const Type* type);
				
				/**
				 * \brief Size Of
				 * 
				 * The size of the given type.
				 */
				static Value SizeOf(const Type* targetType, const Type* sizeType);
				
				/**
				 * \brief Union data offset
				 * 
				 * The offset of the union data (usually expected to be after the tag).
				 */
				static Value UnionDataOffset(const TypeInstance* typeInstance, const Type* sizeType);
				
				/**
				 * \brief Member variable offset
				 * 
				 * The offset of a member variable within an object.
				 */
				static Value MemberOffset(const TypeInstance* typeInstance, size_t memberIndex, const Type* sizeType);
				
				/**
				 * \brief Reinterpret a value as a type
				 * 
				 * Reinterprets the given value as if it had the given type.
				 */
				static Value Reinterpret(Value operand, const Type* type);
				
				/**
				 * \brief Dereference
				 * 
				 * Removes one or more layers of extra references from a value.
				 * For example 'T&&' could be dereferenced to 'T&'.
				 */
				static Value DerefReference(Value operand);
				
				/**
				 * \brief Ternary
				 * 
				 * Evaluates the condition to a boolean result and then evaluates
				 * either the ifTrue or ifFalse values depending on that outcome.
				 * 
				 * Note that ifTrue will ONLY be evaluated if the condition is true;
				 * similarly ifFalse will ONLY be evaluated if the condition is false.
				 */
				static Value Ternary(Value condition, Value ifTrue, Value ifFalse);
				
				/**
				 * \brief Cast
				 * 
				 * Casts the given value to the given type (usually a no-op that
				 * exists for type-safety purposes).
				 */
				static Value Cast(const Type* targetType, Value operand);
				
				/**
				 * \brief Polymorphic Cast
				 * 
				 * Casts the given value polymorphically (via a reference) to
				 * the target type. For example, 'Class&' could be polymorphically
				 * cast to 'Interface&'.
				 */
				static Value PolyCast(const Type* targetType, Value operand);
				
				/**
				 * \brief Add Lval Type Tag
				 * 
				 * Turns a value into an lvalue type. This is a no-op, but
				 * it modifies the type of the value and this affects the
				 * semantics when the value is used (e.g. methods like
				 * 'dissolve' are automatically invoked).
				 */
				static Value Lval(const Type* targetType, Value operand);
				
				/**
				 * \brief Remove Lval Type Tag
				 * 
				 * Removes the 'lval' tag from the value's type, which
				 * disables special automatic lvalue operations.
				 */
				static Value NoLval(Value operand);
				
				/**
				 * \brief Add Ref Type Tag
				 * 
				 * Turns a value into a ref type. This is a no-op, but
				 * it modifies the type of the value and this affects the
				 * semantics when the value is used (e.g. methods like
				 * 'deref' are automatically invoked).
				 */
				static Value Ref(const Type* targetType, Value operand);
				
				/**
				 * \brief Remove Ref Type Tag
				 * 
				 * Removes the 'ref' tag from the value's type, which
				 * disables special automatic ref operations.
				 */
				static Value NoRef(Value operand);
				
				/**
				 * \brief Add StaticRef Type Tag
				 * 
				 * Turns a value into a staticref type. This is a no-op, but
				 * it modifies the type of the value and this affects the
				 * semantics when the value is used (e.g. methods like
				 * 'deref' are automatically invoked).
				 */
				static Value StaticRef(const Type* targetType, Value operand);
				
				/**
				 * \brief Remove StaticRef Type Tag
				 * 
				 * Removes the 'staticref' tag from the value's type, which
				 * disables special automatic staticref operations.
				 */
				static Value NoStaticRef(Value operand);
				
				/**
				 * \brief Call internal constructor
				 * 
				 * This creates an instance of the parent object type using
				 * the 'internal constructor', a special auto-generated method.
				 */
				static Value InternalConstruct(const TypeInstance* typeInstance, HeapArray<Value> parameters);
				
				/**
				 * \brief Access member variable
				 * 
				 * Accesses a member variable of the given object.
				 * This will return a reference to an lvalue type.
				 */
				static Value MemberAccess(Value object, const Var& var, const Type* type);
				
				/**
				 * \brief Bind Value Reference
				 * 
				 * Automatically generates an lvalue in which to store the given
				 * rvalue and returns a reference to the value in that lvalue.
				 */
				static Value BindReference(Value operand, const Type* type);
				
				/**
				 * \brief Type Reference
				 * 
				 * A 'typename_t' value that contains type information
				 * for the given type.
				 */
				static Value TypeRef(const Type* targetType, const Type* type);
				
				/**
				 * \brief Template Variable Reference
				 * 
				 * A reference to a template variable.
				 */
				static Value TemplateVarRef(const TemplateVar* targetVar, const Type* type);
				
				/**
				 * \brief Call
				 * 
				 * Calls a (callable) value with the given parameters.
				 */
				static Value Call(Value functionValue, HeapArray<Value> parameters);
				
				/**
				 * \brief Function Reference
				 * 
				 * Refers to the given function with the provided template arguments.
				 * The function could be a method hence the parent type should be
				 * set, otherwise it should be NULL.
				 */
				static Value FunctionRef(const Type* parentType, Function* function, HeapArray<Value> templateArguments, const Type* const type);
				
				/**
				 * \brief Template Function Reference
				 * 
				 * Refers to a static method of a templated type with the given name.
				 */
				static Value TemplateFunctionRef(const Type* parentType, const String& name, const Type* functionType);
				
				/**
				 * \brief Method Reference
				 * 
				 * Refers to a dynamic method by providing the method function
				 * reference and the parent object instance value (must be
				 * a reference to the object).
				 */
				static Value MethodObject(Value method, Value methodOwner);
				
				/**
				 * \brief Interface Method Reference
				 * 
				 * Refers to a dynamic interface method by providing the method function
				 * reference and the parent interface instance value (must be
				 * a reference to the interface).
				 */
				static Value InterfaceMethodObject(Value method, Value methodOwner);
				
				/**
				 * \brief Static Interface Method Reference
				 * 
				 * Refers to a static interface method by providing the method function
				 * reference and the parent type reference value.
				 */
				static Value StaticInterfaceMethodObject(Value method, Value typeRef);
				
				/**
				 * \brief Cast Dummy
				 * 
				 * A placeholder value for cast operations in the Semantic Analysis stage.
				 */
				static Value CastDummy(const Type* type);
				
				Value();
				
				Value(Value&&) = default;
				Value& operator=(Value&&) = default;
				
				Kind kind() const;
				
				Value copy() const;
				
				const Type* type() const;
				
				ExitStates exitStates() const;
				
				bool isZeroInitialise() const;
				
				bool isMemCopy() const;
				const Value& memCopyOperand() const;
				
				bool isSelf() const;
				bool isThis() const;
				
				bool isConstant() const;
				const locic::Constant& constant() const;
				
				bool isPredicate() const;
				const Predicate& predicate() const;
				
				bool isLocalVarRef() const;
				const Var& localVar() const;
				
				bool isUnionTag() const;
				const Value& unionTagOperand() const;
				
				bool isSizeOf() const;
				const Type* sizeOfType() const;
				
				bool isUnionDataOffset() const;
				const TypeInstance* unionDataOffsetTypeInstance() const;
				
				bool isMemberOffset() const;
				const TypeInstance* memberOffsetTypeInstance() const;
				size_t memberOffsetMemberIndex() const;
				
				bool isReinterpret() const;
				const Value& reinterpretOperand() const;
				
				bool isDeref() const;
				const Value& derefOperand() const;
				
				bool isTernary() const;
				const Value& ternaryCondition() const;
				const Value& ternaryIfTrue() const;
				const Value& ternaryIfFalse() const;
				
				bool isCast() const;
				const Type* castTargetType() const;
				const Value& castOperand() const;
				
				bool isPolyCast() const;
				const Type* polyCastTargetType() const;
				const Value& polyCastOperand() const;
				
				bool isMakeLval() const;
				const Type* makeLvalTargetType() const;
				const Value& makeLvalOperand() const;
				
				bool isMakeNoLval() const;
				const Value& makeNoLvalOperand() const;
				
				bool isMakeRef() const;
				const Type* makeRefTargetType() const;
				const Value& makeRefOperand() const;
				
				bool isMakeNoRef() const;
				const Value& makeNoRefOperand() const;
				
				bool isMakeStaticRef() const;
				const Type* makeStaticRefTargetType() const;
				const Value& makeStaticRefOperand() const;
				
				bool isMakeNoStaticRef() const;
				const Value& makeNoStaticRefOperand() const;
				
				bool isInternalConstruct() const;
				const HeapArray<Value>& internalConstructParameters() const;
				
				bool isMemberAccess() const;
				const Value& memberAccessObject() const;
				const Var& memberAccessVar() const;
				
				bool isBindReference() const;
				const Value& bindReferenceOperand() const;
				
				bool isTypeRef() const;
				const Type* typeRefType() const;
				
				bool isTemplateVarRef() const;
				const TemplateVar* templateVar() const;
				
				bool isCall() const;
				const Value& callValue() const;
				const HeapArray<Value>& callParameters() const;
				
				bool isFunctionRef() const;
				const Type* functionRefParentType() const;
				Function* functionRefFunction() const;
				const HeapArray<Value>& functionRefTemplateArguments() const;
				
				bool isTemplateFunctionRef() const;
				const Type* templateFunctionRefParentType() const;
				const String& templateFunctionRefName() const;
				const Type* templateFunctionRefFunctionType() const;
				
				bool isMethodObject() const;
				const Value& methodObject() const;
				const Value& methodOwner() const;
				
				bool isInterfaceMethodObject() const;
				const Value& interfaceMethodObject() const;
				const Value& interfaceMethodOwner() const;
				
				bool isStaticInterfaceMethodObject() const;
				const Value& staticInterfaceMethodObject() const;
				const Value& staticInterfaceMethodOwner() const;
				
				void setDebugInfo(Debug::ValueInfo debugInfo);
				Optional<Debug::ValueInfo> debugInfo() const;
				
				size_t hash() const;
				
				bool operator==(const Value& value) const;
				bool operator!=(const Value& value) const {
					return !(*this == value);
				}
				
				bool dependsOn(const TemplateVar* const templateVar) const;
				bool dependsOnAny(const TemplateVarArray& array) const;
				bool dependsOnOnly(const TemplateVarArray& array) const;
				
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
				
				std::unique_ptr<Value> value0_, value1_, value2_;
				Optional<Predicate> predicate_;
				TypeArray typeArray_;
				HeapArray<Value> valueArray_;
				
				union {
					locic::Constant constant_;
					
					struct {
						const Var* var;
					} localVar_;
					
					struct {
						const Type* targetType;
					} sizeOf_;
					
					struct {
						const TypeInstance* typeInstance;
					} unionDataOffset_;
					
					struct {
						const TypeInstance* typeInstance;
						size_t memberIndex;
					} memberOffset_;
					
					struct {
						const Type* targetType;
					} cast_;
					
					struct {
						const Type* targetType;
					} polyCast_;
					
					struct {
						const Type* targetType;
					} makeLval_;
					
					struct {
						const Type* targetType;
					} makeRef_;
					
					struct {
						const Type* targetType;
					} makeStaticRef_;
					
					struct {
						const Var* memberVar;
					} memberAccess_;
					
					struct {
						const Type* targetType;
					} typeRef_;
					
					struct {
						const TemplateVar* templateVar;
					} templateVarRef_;
					
					struct {
						const Type* parentType;
						Function* function;
					} functionRef_;
					
					struct {
						const Type* parentType;
						String name;
						const Type* functionType;
					} templateFunctionRef_;
				} union_;
				
		};
		
	}
	
}

#endif
