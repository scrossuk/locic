#ifndef LOCIC_AST_VALUE_HPP
#define LOCIC_AST_VALUE_HPP

#include <memory>

#include <locic/AST/TemplateVarArray.hpp>
#include <locic/AST/ValueArray.hpp>
#include <locic/Support/HeapArray.hpp>
#include <locic/Support/Map.hpp>
#include <locic/Support/Optional.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	class Constant;
	
	namespace Debug {
		
		class ValueInfo;
		
	}
	
	namespace AST {
		
		class Alias;
		class ExitStates;
		class Function;
		class Predicate;
		class TemplateVar;
		class TemplateVarMap;
		class Type;
		class TypeInstance;
		class Var;
		
		class Value {
			public:
				enum Kind {
					SELF,
					THIS,
					CONSTANT,
					ALIAS,
					PREDICATE,
					LOCALVAR,
					REINTERPRET,
					DEREF_REFERENCE,
					TERNARY,
					CAST,
					POLYCAST,
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
					CAPABILITYTEST,
					ARRAYLITERAL,
					NEW,
					
					// Used by Semantic Analysis to create a 'dummy'
					// value to test if types can be cast.
					CASTDUMMYOBJECT
				};
				
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
				 * \brief Alias
				 * 
				 * An alias value.
				 */
				static Value Alias(const AST::Alias& alias,
				                   ValueArray templateArguments,
				                   const Type* type);
				
				/**
				 * \brief Predicate
				 * 
				 * A predicate value.
				 */
				static Value PredicateExpr(Predicate value,
				                           const Type* boolType);
				
				/**
				 * \brief Local Variable
				 * 
				 * A reference to a local variable.
				 */
				static Value LocalVar(const Var& var, const Type* type);
				
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
				 * \brief Call internal constructor
				 * 
				 * This creates an instance of the parent object type using
				 * the 'internal constructor', a special auto-generated method.
				 */
				static Value InternalConstruct(const Type* parentType, ValueArray parameters);
				
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
				static Value Call(Value functionValue, ValueArray parameters, const Type* type);
				
				/**
				 * \brief Function Reference
				 * 
				 * Refers to the given function with the provided template arguments.
				 * The function could be a method hence the parent type should be
				 * set, otherwise it should be NULL.
				 */
				static Value FunctionRef(const Type* parentType, const Function& function,
				                         ValueArray templateArguments, const Type* const type);
				
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
				static Value MethodObject(Value method, Value methodOwner, const Type* const type);
				
				/**
				 * \brief Interface Method Reference
				 * 
				 * Refers to a dynamic interface method by providing the method function
				 * reference and the parent interface instance value (must be
				 * a reference to the interface).
				 */
				static Value InterfaceMethodObject(Value method, Value methodOwner, const Type* const type);
				
				/**
				 * \brief Static Interface Method Reference
				 * 
				 * Refers to a static interface method by providing the method function
				 * reference and the parent type reference value.
				 */
				static Value StaticInterfaceMethodObject(Value method, Value typeRef, const Type* const type);
				
				/**
				 * \brief Capability Test
				 * 
				 * Query whether a type has the capabilities (methods) specified by
				 * another type.
				 */
				static Value CapabilityTest(const Type* const checkType,
				                            const Type* const capabilityType,
				                            const Type* const boolType);
				
				/**
				 * \brief Array literal.
				 * 
				 * An array literal, e.g. { 1, 2, 3 }.
				 */
				static Value ArrayLiteral(const Type* arrayType,
				                          ValueArray values);
				
				/**
				 * \brief New expression.
				 */
				static Value New(Value placementArg, Value operand,
				                 const Type* voidType);
				
				/**
				 * \brief Cast Dummy
				 * 
				 * A placeholder value for cast operations in the Semantic Analysis stage.
				 */
				static Value CastDummy(const Type* type);
				
				Value();
				~Value();
				
				Value(Value&&) = default;
				Value& operator=(Value&&) = default;
				
				Kind kind() const;
				
				Value copy() const;
				
				const Type* type() const;
				
				ExitStates exitStates() const;
				
				bool isSelf() const;
				bool isThis() const;
				
				bool isConstant() const;
				const locic::Constant& constant() const;
				
				bool isAlias() const;
				const AST::Alias& alias() const;
				const ValueArray& aliasTemplateArguments() const;
				
				bool isPredicate() const;
				const Predicate& predicate() const;
				
				bool isLocalVarRef() const;
				const Var& localVar() const;
				
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
				
				bool isInternalConstruct() const;
				const ValueArray& internalConstructParameters() const;
				
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
				const ValueArray& callParameters() const;
				
				bool isFunctionRef() const;
				const Type* functionRefParentType() const;
				const Function& functionRefFunction() const;
				const ValueArray& functionRefTemplateArguments() const;
				
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
				
				bool isCapabilityTest() const;
				const Type* capabilityTestCheckType() const;
				const Type* capabilityTestCapabilityType() const;
				
				bool isArrayLiteral() const;
				const ValueArray& arrayLiteralValues() const;
				
				bool isNew() const;
				const Value& newPlacementArg() const;
				const Value& newOperand() const;
				
				void setDebugInfo(Debug::ValueInfo debugInfo);
				const Optional<Debug::ValueInfo>& debugInfo() const;
				
				size_t hash() const;
				
				bool operator==(const Value& value) const;
				bool operator!=(const Value& value) const {
					return !(*this == value);
				}
				
				bool dependsOn(const TemplateVar* const templateVar) const;
				bool dependsOnAny(const TemplateVarArray& array) const;
				bool dependsOnOnly(const TemplateVarArray& array) const;
				
				Value substitute(const TemplateVarMap& templateVarMap,
				                 const Predicate& selfconst) const;
				Predicate makePredicate() const;
				
				std::string toString() const;
				std::string toDiagString() const;
				
			private:
				Value(Kind kind,
				      const Type* type,
				      ExitStates exitStates);
				
				// Non-copyable.
				Value(const Value&) = delete;
				Value& operator=(const Value&) = delete;
				
				std::shared_ptr<class ValueImpl> impl_;
				
		};
		
	}
	
}

#endif
