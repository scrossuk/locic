#ifndef LOCIC_AST_TYPEINSTANCE_HPP
#define LOCIC_AST_TYPEINSTANCE_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <locic/AST/ExceptionInitializer.hpp>
#include <locic/AST/GlobalStructure.hpp>
#include <locic/AST/ModuleScope.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/RequireSpecifier.hpp>
#include <locic/AST/StringList.hpp>
#include <locic/AST/TemplatedObject.hpp>
#include <locic/AST/TemplateVarArray.hpp>
#include <locic/AST/TypeArray.hpp>
#include <locic/AST/Value.hpp>
#include <locic/AST/ValueArray.hpp>

#include <locic/Debug/TypeInstanceInfo.hpp>

#include <locic/AST/Predicate.hpp>

#include <locic/Support/FastMap.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/Optional.hpp>
#include <locic/Support/PrimitiveID.hpp>
#include <locic/Support/String.hpp>

#include <locic/Support/FastMap.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/Optional.hpp>
#include <locic/Support/PrimitiveID.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		class Context;
		struct ExceptionInitializer;
		
		class Function;
		typedef std::vector<Node<Function>> FunctionList;
		
		class TemplateVar;
		typedef std::vector<Node<TemplateVar>> TemplateVarList;
		
		class TypeInstance;
		typedef std::vector<Node<TypeInstance>> TypeInstanceList;
		
		class Var;
		typedef std::vector<Node<Var>> VarList;
		
		/**
		 * \brief Type Instance
		 * 
		 * A type instance is a type structure, such
		 * as a class, interface, datatype etc.
		 * 
		 * All type structures have essentially the
		 * same properties and hence are managed by
		 * this common class:
		 * 
		 * - A name
		 * - Template variables
		 * - Static/dynamic methods
		 * - Member variables
		 * - Require predicates
		 * - In some cases variants and parent
		 */
		class TypeInstance final: public TemplatedObject {
			public:
				enum Kind {
					PRIMITIVE,
					ENUM,
					STRUCT,
					OPAQUE_STRUCT,
					UNION,
					CLASSDECL,
					CLASSDEF,
					DATATYPE,
					UNION_DATATYPE,
					INTERFACE,
					EXCEPTION
				};
				
				TypeInstance(String name, Kind kind);
				~TypeInstance();
				
				Kind kind() const;
				String name() const;
				
				// TODO: turn these into getters/setters.
				Node<StringList> constructors;
				Node<TemplateVarList> templateVariableDecls;
				Node<TypeInstanceList> variantDecls;
				Node<VarList> variableDecls;
				Node<FunctionList> functionDecls;
				Node<ExceptionInitializer> initializer;
				Node<RequireSpecifier> moveSpecifier;
				Node<RequireSpecifier> requireSpecifier;
				Node<StringList> noTagSetDecl;
				
				/**
				 * \brief Get/set context.
				 * 
				 * The context is primarily for 'uniquifying'
				 * SEM objects and hence known by the type
				 * instance so that it can built such objects.
				 * 
				 * \return The SEM context.
				 */
				Context& context() const;
				void setContext(Context& context);
				
				GlobalStructure& parent();
				const GlobalStructure& parent() const;
				void setParent(GlobalStructure parent);
				
				Namespace& nameSpace();
				const Namespace& nameSpace() const;
				
				const Name& fullName() const;
				void setFullName(Name fullName);
				
				/**
				 * \brief Get module scope.
				 * 
				 * Type instances exist in module scopes.
				 * For example:
				 * 
				 * import some.module 1.0.0 {
				 *     class C { }
				 * }
				 * 
				 * In this case the class exists within
				 * an imported module scope.
				 * 
				 * \return The type instance's module scope.
				 */
				const ModuleScope& moduleScope() const;
				void setModuleScope(ModuleScope moduleScope);
				
				bool isPrimitive() const;
				
				void setPrimitiveID(PrimitiveID primitiveID);
				
				PrimitiveID primitiveID() const;
				
				bool isEnum() const;
				
				bool isStruct() const;
				
				bool isOpaqueStruct() const;
				
				bool isUnion() const;
				
				bool isClassDecl() const;
				
				bool isClassDef() const;
				
				bool isClass() const;
				
				bool isDatatype() const;
				
				bool isUnionDatatype() const;
				
				bool isInterface() const;
				
				bool isException() const;
				
				/**
				 * \brief Get type of 'self'.
				 * 
				 * This creates an object type with template
				 * arguments that refer to the type instance's
				 * own template variables.
				 * 
				 * For example, given:
				 * 
				 *     template <typename A, typename B>
				 *     class SomeType { ... }
				 * 
				 * ...this function will return:
				 * 
				 *     SomeType<A, B>
				 * 
				 */
				const Type* selfType() const;
				
				/**
				 * \brief Get self template arguments.
				 * 
				 * Gets the template arguments of the self
				 * type, which is an array of values that
				 * refer to the type instance's template
				 * variables.
				 * 
				 * \return Template argument array.
				 */
				ValueArray selfTemplateArgs() const;
				
				/**
				 * \brief Get variants.
				 * 
				 * Some type instances (e.g. union datatypes)
				 * can have multiple 'variants', which are
				 * type instances that are effectively children
				 * of this type instance.
				 * 
				 * \return Variant type instance array.
				 */
				std::vector<TypeInstance*>& variants();
				const std::vector<TypeInstance*>& variants() const;
				
				/**
				 * \brief Get template variables.
				 * 
				 * Type instances can have zero or more template
				 * variables.
				 * 
				 * \return Template variable array.
				 */
				TemplateVarArray& templateVariables();
				const TemplateVarArray& templateVariables() const;
				
				/**
				 * \brief Get name to template variable mapping.
				 * 
				 * This map supports looking up template variables
				 * based on names.
				 * 
				 * \return Name to template variable mapping.
				 */
				FastMap<String, TemplateVar*>& namedTemplateVariables();
				const FastMap<String, TemplateVar*>& namedTemplateVariables() const;
				
				/**
				 * \brief Get (optional) move predicate.
				 * 
				 * A type instance can have a move predicate that
				 * specifies the requirements that must be satisfied
				 * for the type to be movable. If no predicate
				 * is provided then the compiler will compute a
				 * default predicate (i.e. that all the member
				 * variables and variants must be movable).
				 * 
				 * \return Move predicate if available or none.
				 */
				const Optional<Predicate>& movePredicate() const;
				void setMovePredicate(Predicate predicate);
				
				/**
				 * \brief Get require predicate.
				 * 
				 * A type instance can have a require predicate that
				 * specifies the requirements for its template
				 * arguments, which must always be true and is
				 * hence also a requirement for all its methods.
				 * 
				 * \return Require predicate.
				 */
				const Predicate& requiresPredicate() const;
				void setRequiresPredicate(Predicate predicate);
				
				/**
				 * \brief Get noexcept predicate.
				 * 
				 * This currently doesn't mean anything for
				 * type instances, but is planned for future. In
				 * the meantime it's needed to comply with the
				 * TemplatedObject interface.
				 * 
				 * \return Noexcept predicate.
				 */
				const Predicate& noexceptPredicate() const;
				
				/**
				 * \brief Get name to member variable mapping.
				 * 
				 * This map supports looking up member variables
				 * based on names.
				 * 
				 * \return Name to member variable mapping.
				 */
				FastMap<String, Var*>& namedVariables();
				const FastMap<String, Var*>& namedVariables() const;
				
				/**
				 * \brief Get member variables.
				 * 
				 * Type instances can have zero or more member
				 * variables.
				 * 
				 * \return Member variable array.
				 */
				const std::vector<Var*>& variables() const;
				
				void attachVariable(Var& var);
				
				/**
				 * \brief Get methods.
				 * 
				 * This is a map from canonicalised method
				 * names to the methods of this type instance.
				 * 
				 * \return Method map.
				 */
				Array<Function*, 8>& functions();
				const Array<Function*, 8>& functions() const;
				
				/**
				 * \brief Attach method.
				 * 
				 * Note that multiple methods can be attached with the
				 * same name; this allows SemanticAnalysis to continue
				 * to process code that has duplicate methods.
				 */
				void attachFunction(Function& function);
				void attachFunction(std::unique_ptr<Function> function);
				
				/**
				 * \brief Check if type already has method.
				 */
				bool hasFunction(String canonicalName) const;
				
				/**
				 * \brief Find method (if it exists).
				 * 
				 * \return Function if found, NULL otherwise.
				 */
				Function* findFunction(String canonicalName);
				const Function* findFunction(String canonicalName) const;
				
				/**
				 * \brief Get method.
				 * 
				 * This relies on the type already being known to
				 * have the given method (it will assert this).
				 */
				Function& getFunction(String canonicalName);
				const Function& getFunction(String canonicalName) const;
				
				/**
				 * \brief Compute construct types.
				 * 
				 * This computes the types of the values that
				 * must be passed to the internal constructor
				 * of this type instance. They are the types
				 * of the member variables without their lval
				 * wrapper.
				 * 
				 * \return Construct types.
				 */
				TypeArray constructTypes() const;
				
				/**
				 * \brief Get/set parent.
				 * 
				 * Some type instances can have a parent type instance.
				 * 
				 * This property needs to be stored here to efficiently
				 * verify the correctness of datatype-switch statements.
				 * 
				 * \return Parent type instance.
				 */
				const TypeInstance* parentTypeInstance() const;
				void setParentTypeInstance(const TypeInstance* parent);
				
				const Type* parentType() const;
				void setParentType(const Type* parent);
				
				/**
				 * \brief Get/set notag() set.
				 * 
				 * When using the notag() on an object type
				 * this removes type tags from the type itself
				 * and, depending on the object, some of the
				 * template arguments of the type.
				 * 
				 * This set defines which template arguments
				 * (based on their associated template variable)
				 * should have the notag() added to them.
				 */
				const TemplateVarArray& noTagSet() const;
				void setNoTagSet(TemplateVarArray noTagSet);
				
				/**
				 * \brief Get/set debugging info.
				 * 
				 * A type instance can have associated debugging
				 * information that describes its properties
				 * and location in the original source code.
				 * 
				 * \return Association debugging information.
				 */
				Optional<Debug::TypeInstanceInfo> debugInfo() const;
				void setDebugInfo(Debug::TypeInstanceInfo debugInfo);
				
				std::string refToString() const;
				
				std::string toString() const;
				
			private:
				String name_;
				Kind kind_;
				Context* context_;
				Optional<GlobalStructure> parent_;
				Name fullName_;
				ModuleScope moduleScope_;
				Optional<PrimitiveID> primitiveID_;
				Optional<Debug::TypeInstanceInfo> debugInfo_;
				
				const TypeInstance* parentTypeInstance_;
				const Type* parentType_;
				
				std::vector<TypeInstance*> variants_;
				
				TemplateVarArray templateVariables_;
				FastMap<String, TemplateVar*> namedTemplateVariables_;
				Optional<Predicate> movePredicate_;
				Predicate requiresPredicate_;
				Predicate noexceptPredicate_;
				
				std::vector<Var*> variables_;
				FastMap<String, Var*> namedVariables_;
				
				Array<Function*, 8> functions_;
				
				TemplateVarArray noTagSet_;
				mutable const Type* cachedSelfType_;
				
		};
		
	}
	
}

#endif
