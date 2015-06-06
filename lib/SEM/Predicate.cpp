#include <assert.h>

#include <memory>
#include <stdexcept>
#include <string>

#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/TemplateVarMap.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/Value.hpp>
#include <locic/Support/Hasher.hpp>
#include <locic/Support/MakeString.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace SEM {
		
		Predicate Predicate::FromBool(const bool boolValue) {
			return boolValue ? True() : False();
		}
		
		Predicate Predicate::True() {
			return Predicate(TRUE);
		}
		
		Predicate Predicate::False() {
			return Predicate(FALSE);
		}
		
		Predicate Predicate::And(Predicate left, Predicate right) {
			if (left.isFalse() || right.isFalse()) {
				return Predicate::False();
			} else if (left.isTrue()) {
				return right;
			} else if (right.isTrue()) {
				return left;
			} else if (left == right) {
				return left;
			} else if (left.isAnd() && (left.andLeft() == right || left.andRight() == right)) {
				return left;
			} else if (right.isAnd() && (right.andLeft() == left || right.andRight() == left)) {
				return right;
			}
			
			Predicate predicate(AND);
			predicate.left_ = std::unique_ptr<Predicate>(new Predicate(std::move(left)));
			predicate.right_ = std::unique_ptr<Predicate>(new Predicate(std::move(right)));
			return predicate;
		}
		
		Predicate Predicate::Or(Predicate left, Predicate right) {
			if (left.isTrue() || right.isTrue()) {
				return Predicate::True();
			} else if (left.isFalse()) {
				return right;
			} else if (right.isFalse()) {
				return left;
			} else if (left == right) {
				return left;
			} else if (left.isOr() && (left.orLeft() == right || left.orRight() == right)) {
				return left;
			} else if (right.isOr() && (right.orLeft() == left || right.orRight() == left)) {
				return right;
			}
			
			Predicate predicate(OR);
			predicate.left_ = std::unique_ptr<Predicate>(new Predicate(std::move(left)));
			predicate.right_ = std::unique_ptr<Predicate>(new Predicate(std::move(right)));
			return predicate;
		}
		
		Predicate Predicate::Satisfies(const Type* const type, const Type* const requirement) {
			Predicate predicate(SATISFIES);
			predicate.type_ = type;
			predicate.requirement_ = requirement;
			return predicate;
		}
		
		Predicate Predicate::Variable(TemplateVar* templateVar) {
			Predicate predicate(VARIABLE);
			predicate.templateVar_ = templateVar;
			return predicate;
		}
		
		Predicate::Predicate(Kind k) : kind_(k) { }
		
		Predicate Predicate::copy() const {
			switch (kind()) {
				case TRUE:
				{
					return Predicate::True();
				}
				case FALSE:
				{
					return Predicate::False();
				}
				case AND:
				{
					return Predicate::And(andLeft().copy(), andRight().copy());
				}
				case OR:
				{
					return Predicate::Or(orLeft().copy(), orRight().copy());
				}
				case SATISFIES:
				{
					return Predicate::Satisfies(satisfiesType(), satisfiesRequirement());
				}
				case VARIABLE:
				{
					return Predicate::Variable(variableTemplateVar());
				}
			}
			
			throw std::logic_error("Unknown predicate kind.");
		}
		
		Predicate Predicate::substitute(const TemplateVarMap& templateVarMap) const {
			switch (kind()) {
				case TRUE:
				{
					return Predicate::True();
				}
				case FALSE:
				{
					return Predicate::False();
				}
				case AND:
				{
					return Predicate::And(andLeft().substitute(templateVarMap), andRight().substitute(templateVarMap));
				}
				case OR:
				{
					return Predicate::Or(orLeft().substitute(templateVarMap), orRight().substitute(templateVarMap));
				}
				case SATISFIES:
				{
					return Predicate::Satisfies(satisfiesType()->substitute(templateVarMap), satisfiesRequirement()->substitute(templateVarMap));
				}
				case VARIABLE:
				{
					const auto iterator = templateVarMap.find(variableTemplateVar());
					
					if (iterator == templateVarMap.end()) {
						return Predicate::Variable(variableTemplateVar());
					}
					
					const auto& templateValue = iterator->second;
					return templateValue.makePredicate();
				}
			}
			
			throw std::logic_error("Unknown predicate kind.");
		}
		
		/*bool Predicate::isEvaluatable() const {
			
		}
		
		Optional<bool> Predicate::evaluate() const {
			switch (kind()) {
				case TRUE:
				{
					return make_optional(true);
				}
				case FALSE:
				{
					return make_optional(false);
				}
				case AND:
				{
					const auto leftIsTrue = predicate.andLeft().evaluate();
					if (!leftIsTrue) {
						return None;
					}
					
					const auto rightIsTrue = evaluatePredicate(context, predicate.andRight(), variableAssignments);
					if (!rightIsTrue) {
						return None;
					}
					
					return make_optional(*leftIsTrue && *rightIsTrue);
				}
				case OR:
				{
					const auto leftIsTrue = evaluatePredicate(context, predicate.orLeft(), variableAssignments);
					if (!leftIsTrue) {
						return None;
					}
					
					const auto rightIsTrue = evaluatePredicate(context, predicate.orRight(), variableAssignments);
					if (!rightIsTrue) {
						return None;
					}
					
					return make_optional(*leftIsTrue || *rightIsTrue);
				}
				case SATISFIES:
				{
					const auto templateVar = predicate.satisfiesType();
					const auto requireType = predicate.satisfiesRequirement();
					
					const auto templateValue = variableAssignments.at(templateVar).typeRefType()->resolveAliases();
					if (templateValue->isAuto()) {
						// Presumably this will work.
						return make_optional(true);
					}
					
					// Some of the requirements can depend on the template values provided.
					const auto substitutedRequireType = requireType->substitute(variableAssignments);
					
					const auto sourceMethodSet = getTypeMethodSet(context, templateValue);
					const auto requireMethodSet = getTypeMethodSet(context, substitutedRequireType);
					
					return make_optional(methodSetSatisfiesRequirement(sourceMethodSet, requireMethodSet));
				}
				case VARIABLE:
				{
					const auto templateVar = predicate.variableTemplateVar();
					const auto iterator = variableAssignments.find(templateVar);
					
					if (iterator == variableAssignments.end()) {
						// Unknown result since we don't know the variable's value.
						return None;
					}
					
					const auto& templateValue = iterator->second;
					
					if (templateValue.isConstant()) {
						assert(templateValue.constant().kind() == Constant::BOOLEAN);
						return make_optional(templateValue.constant().boolValue());
					} else {
						// Unknown result since we don't know the variable's value.
						return None;
					}
				}
			}
		}
		
		bool Predicate::evaluateWithDefault(const bool defaultValue) const {
			
		}*/
		
		bool Predicate::dependsOn(const TemplateVar* const templateVar) const {
			switch (kind()) {
				case TRUE:
				case FALSE:
				{
					return false;
				}
				case AND:
				{
					return andLeft().dependsOn(templateVar) || andRight().dependsOn(templateVar);
				}
				case OR:
				{
					return orLeft().dependsOn(templateVar) || orRight().dependsOn(templateVar);
				}
				case SATISFIES:
				{
					return satisfiesType()->dependsOn(templateVar) || satisfiesRequirement()->dependsOn(templateVar);
				}
				case VARIABLE:
				{
					return templateVar == variableTemplateVar();
				}
			}
			
			throw std::logic_error("Unknown predicate kind.");
		}
		
		bool Predicate::dependsOnAny(const TemplateVarArray& array) const {
			for (const auto& templateVar: array) {
				if (dependsOn(templateVar)) {
					return true;
				}
			}
			return false;
		}
		
		bool Predicate::dependsOnOnly(const TemplateVarArray& array) const {
			switch (kind()) {
				case TRUE:
				case FALSE:
				{
					return true;
				}
				case AND:
				{
					return andLeft().dependsOnOnly(array) && andRight().dependsOnOnly(array);
				}
				case OR:
				{
					return orLeft().dependsOnOnly(array) && orRight().dependsOnOnly(array);
				}
				case SATISFIES:
				{
					return satisfiesType()->dependsOnOnly(array) && satisfiesRequirement()->dependsOnOnly(array);
				}
				case VARIABLE:
				{
					return array.contains(variableTemplateVar());
				}
			}
			
			throw std::logic_error("Unknown predicate kind.");
		}
		
		Predicate Predicate::reduceToDependencies(const TemplateVarArray& array, const bool conservativeDefault) const {
			switch (kind()) {
				case TRUE:
				case FALSE:
				{
					return copy();
				}
				case AND:
				{
					return And(andLeft().reduceToDependencies(array, conservativeDefault), andRight().reduceToDependencies(array, conservativeDefault));
				}
				case OR:
				{
					return Or(orLeft().reduceToDependencies(array, conservativeDefault), orRight().reduceToDependencies(array, conservativeDefault));
				}
				case SATISFIES:
				{
					if (satisfiesType()->dependsOnOnly(array) && satisfiesRequirement()->dependsOnOnly(array)) {
						return copy();
					} else {
						return Predicate::FromBool(conservativeDefault);
					}
				}
				case VARIABLE:
				{
					if (array.contains(variableTemplateVar())) {
						return copy();
					} else {
						return Predicate::FromBool(conservativeDefault);
					}
				}
			}
			
			throw std::logic_error("Unknown predicate kind.");
		}
		
		bool Predicate::implies(const SEM::Predicate& other) const {
			switch (kind()) {
				case TRUE:
					// Drop through.
					break;
				case FALSE:
					return true;
				case AND:
					// (A and B) => C is true iff ((A => C) or (B => C)).
					return andLeft().implies(other) || andRight().implies(other);
				case OR:
					// (A or B) => C is true iff ((A => C) and (B => C)).
					return orLeft().implies(other) && orRight().implies(other);
				case SATISFIES:
				case VARIABLE:
					// Drop through.
					break;
			}
			
			switch (other.kind()) {
				case TRUE:
					return true;
				case FALSE:
					return false;
				case AND:
					// A => (B and C) is true iff ((A => B) and (A => C)).
					return implies(other.andLeft()) && implies(other.andRight());
				case OR:
					// A => (B or C) is true iff ((A => B) or (A => C)).
					return implies(other.orLeft()) || implies(other.orRight());
				case SATISFIES:
				case VARIABLE:
					// Drop through.
					break;
			}
			
			if (kind() != other.kind()) {
				return false;
			}
			
			switch (kind()) {
				case SATISFIES:
					// TODO: this needs to be improved to allow covariant
					// treatment of the requirement type (e.g. T : copyable_and_movable<T>
					// implies T : movable).
					return satisfiesType() == other.satisfiesType() && satisfiesRequirement() == other.satisfiesRequirement();
				case VARIABLE:
					return variableTemplateVar() == other.variableTemplateVar();
				default:
					throw std::logic_error("Reached unreachable block in 'implies'.");
			}
		}
		
		Predicate::Kind Predicate::kind() const {
			return kind_;
		}
		
		bool Predicate::isTrivialBool() const {
			return isTrue() || isFalse();
		}
		
		bool Predicate::isTrue() const {
			return kind() == TRUE;
		}
		
		bool Predicate::isFalse() const {
			return kind() == FALSE;
		}
		
		bool Predicate::isAnd() const {
			return kind() == AND;
		}
		
		bool Predicate::isOr() const {
			return kind() == OR;
		}
		
		bool Predicate::isSatisfies() const {
			return kind() == SATISFIES;
		}
		
		bool Predicate::isVariable() const {
			return kind() == VARIABLE;
		}
		
		const Predicate& Predicate::andLeft() const {
			assert(isAnd());
			return *left_;
		}
		
		const Predicate& Predicate::andRight() const {
			assert(isAnd());
			return *right_;
		}
		
		const Predicate& Predicate::orLeft() const {
			assert(isOr());
			return *left_;
		}
		
		const Predicate& Predicate::orRight() const {
			assert(isOr());
			return *right_;
		}
		
		const Type* Predicate::satisfiesType() const {
			assert(isSatisfies());
			return type_;
		}
		
		const Type* Predicate::satisfiesRequirement() const {
			assert(isSatisfies());
			return requirement_;
		}
		
		TemplateVar* Predicate::variableTemplateVar() const {
			assert(isVariable());
			return templateVar_;
		}
		
		bool Predicate::operator==(const Predicate& other) const {
			if (kind() != other.kind()) {
				return false;
			}
			
			switch (kind()) {
				case TRUE:
				case FALSE:
				{
					return true;
				}
				case AND:
				{
					return andLeft() == other.andLeft() && andRight() == other.andRight();
				}
				case OR:
				{
					return orLeft() == other.orLeft() && orRight() == other.orRight();
				}
				case SATISFIES:
				{
					return satisfiesType() == other.satisfiesType() &&
						satisfiesRequirement() == other.satisfiesRequirement();
				}
				case VARIABLE:
				{
					return variableTemplateVar() == other.variableTemplateVar();
				}
			}
			
			throw std::logic_error("Unknown predicate kind.");
		}
		
		size_t Predicate::hash() const {
			Hasher hasher;
			hasher.add(kind());
			
			switch (kind()) {
				case TRUE:
				case FALSE:
				{
					break;
				}
				case AND:
				{
					hasher.add(andLeft());
					hasher.add(andRight());
					break;
				}
				case OR:
				{
					hasher.add(orLeft());
					hasher.add(orRight());
					break;
				}
				case SATISFIES:
				{
					hasher.add(satisfiesType());
					hasher.add(satisfiesRequirement());
					break;
				}
				case VARIABLE:
				{
					hasher.add(variableTemplateVar());
					break;
				}
			}
			
			return hasher.get();
		}
		
		std::string Predicate::toString() const {
			switch (kind()) {
				case TRUE:
				{
					return "true";
				}
				case FALSE:
				{
					return "false";
				}
				case AND:
				{
					return makeString("and(%s, %s)",
						andLeft().toString().c_str(),
						andRight().toString().c_str());
				}
				case OR:
				{
					return makeString("or(%s, %s)",
						orLeft().toString().c_str(),
						orRight().toString().c_str());
				}
				case SATISFIES:
				{
					return makeString("satisfies(type: %s, requirement: %s)",
						satisfiesType()->toString().c_str(),
						satisfiesRequirement()->toString().c_str());
				}
				case VARIABLE:
				{
					return makeString("variable(templateVar: %s)",
						variableTemplateVar()->toString().c_str());
				}
			}
			
			throw std::logic_error("Unknown predicate kind.");
		}
		
	}
	
}

