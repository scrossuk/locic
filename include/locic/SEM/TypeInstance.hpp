#ifndef LOCIC_SEM_TYPEINSTANCE_HPP
#define LOCIC_SEM_TYPEINSTANCE_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <locic/Debug/TypeInstanceInfo.hpp>
#include <locic/SEM/ModuleScope.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TemplatedObject.hpp>
#include <locic/SEM/TemplateVarArray.hpp>
#include <locic/SEM/TypeArray.hpp>
#include <locic/SEM/ValueArray.hpp>
#include <locic/Support/FastMap.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/Optional.hpp>
#include <locic/Support/PrimitiveID.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace SEM {
	
		class Context;
		class Function;
		class TemplateVar;
		class Var;
		
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
				
				TypeInstance(Context& c, Name name, Kind k, ModuleScope m);
				
				/**
				 * \brief Get context.
				 * 
				 * The context is primarily for 'uniquifying'
				 * SEM objects and hence known by the type
				 * instance so that it can built such objects.
				 * 
				 * \return The SEM context.
				 */
				Context& context() const;
				
				const Name& name() const;
				
				Kind kind() const;
				
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
				std::vector<Var*>& variables();
				const std::vector<Var*>& variables() const;
				
				/**
				 * \brief Get methods.
				 * 
				 * This is a map from canonicalised method
				 * names to the methods of this type instance.
				 * 
				 * \return Method map.
				 */
				FastMap<String, std::unique_ptr<Function>>& functions();
				const FastMap<String, std::unique_ptr<Function>>& functions() const;
				
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
				const TypeInstance* parent() const;
				void setParent(const TypeInstance* parent);
				
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
				Context& context_;
				Name name_;
				Kind kind_;
				ModuleScope moduleScope_;
				Optional<PrimitiveID> primitiveID_;
				Optional<Debug::TypeInstanceInfo> debugInfo_;
				
				const TypeInstance* parent_;
				const Type* parentType_;
				
				std::vector<TypeInstance*> variants_;
				
				TemplateVarArray templateVariables_;
				FastMap<String, TemplateVar*> namedTemplateVariables_;
				Optional<Predicate> movePredicate_;
				Predicate requiresPredicate_;
				Predicate noexceptPredicate_;
				
				std::vector<Var*> variables_;
				FastMap<String, Var*> namedVariables_;
				
				FastMap<String, std::unique_ptr<Function>> functions_;
				
				TemplateVarArray noTagSet_;
				
		};
		
	}
	
}

#endif
