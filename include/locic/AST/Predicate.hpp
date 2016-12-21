#ifndef LOCIC_AST_PREDICATE_HPP
#define LOCIC_AST_PREDICATE_HPP

#include <memory>

#include <locic/AST/TemplateVarArray.hpp>

namespace locic {
	
	namespace AST {
		
		class TemplateVar;
		class TemplateVarMap;
		class Type;
		
		/**
		 * \brief Predicate
		 * 
		 * A predicate is a compile-time expression that evaluates to either
		 * true or false.
		 * 
		 * Predicates are used to indicate when methods are const, when
		 * methods don't throw, when methods are available (e.g. because a
		 * template type supports a required method) etc.
		 */
		class Predicate {
		public:
			enum Kind {
				TRUE,
				FALSE,
				AND,
				OR,
				SATISFIES,
				VARIABLE
			};
			
			static Predicate FromBool(bool boolValue);
			
			static Predicate True();
			
			static Predicate False();
			
			static Predicate And(Predicate left, Predicate right);
			
			static Predicate Or(Predicate left, Predicate right);
			
			static Predicate Satisfies(const Type* type,
			                           const Type* requirement);
			
			static Predicate Variable(TemplateVar* templateVar);
			
			Predicate(Predicate&&) = default;
			Predicate& operator=(Predicate&&) = default;
			
			Predicate copy() const;
			Predicate substitute(const TemplateVarMap& templateVarMap) const;
			
			/*bool isEvaluatable() const;
			Optional<bool> evaluate() const;
			bool evaluateWithDefault(bool defaultValue) const;*/
			
			bool dependsOn(const TemplateVar* templateVar) const;
			
			/**
			 * \brief Depends on any of given template variables
			 * 
			 * Checks whether the predicate depends on any of the given
			 * template variables.
			 */
			bool dependsOnAny(const TemplateVarArray& array) const;
			
			/**
			 * \brief Depends only on given template variables
			 * 
			 * Checks whether the predicate only depends on the given template
			 * variables and no other template variables are required to
			 * evaluate it.
			 */
			bool dependsOnOnly(const TemplateVarArray& array) const;
			
			/**
			 * \brief Reduce to dependencies
			 * 
			 * Reduce the predicate to only depend on the given template
			 * variables. Any expressions that depend on other template
			 * variables will be reduced to the conservative default given.
			 */
			Predicate reduceToDependencies(const TemplateVarArray& array, bool conservativeDefault) const;
			
			/**
			 * \brief Implies
			 * 
			 * Tries to prove that this predicate implies the provided
			 * predicate, returning true if this is possible and false
			 * otherwise.
			 */
			bool implies(const AST::Predicate& other) const;
			
			Kind kind() const;
			
			bool isTrivialBool() const;
			bool isTrue() const;
			bool isFalse() const;
			bool isAnd() const;
			bool isOr() const;
			bool isSatisfies() const;
			bool isVariable() const;
			
			const Predicate& andLeft() const;
			const Predicate& andRight() const;
			
			const Predicate& orLeft() const;
			const Predicate& orRight() const;
			
			const Type* satisfiesType() const;
			const Type* satisfiesRequirement() const;
			
			TemplateVar* variableTemplateVar() const;
			
			bool operator==(const Predicate& other) const;
			bool operator!=(const Predicate& other) const {
				return !(*this == other);
			}
			
			size_t hash() const;
			
			std::string toString() const;
			
		private:
			Predicate(Kind k);
			
			Kind kind_;
			std::unique_ptr<Predicate> left_, right_;
			
			TemplateVar* templateVar_;
			const Type* type_;
			const Type* requirement_;
			
		};
		
	}
	
}

#endif
