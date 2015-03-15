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
		
		Predicate Predicate::True() {
			return Predicate(TRUE);
		}
		
		Predicate Predicate::False() {
			return Predicate(FALSE);
		}
		
		Predicate Predicate::And(Predicate left, Predicate right) {
			Predicate predicate(AND);
			predicate.left_ = std::unique_ptr<Predicate>(new Predicate(std::move(left)));
			predicate.right_ = std::unique_ptr<Predicate>(new Predicate(std::move(right)));
			return predicate;
		}
		
		Predicate Predicate::Satisfies(TemplateVar* templateVar, const Type* requirement) {
			Predicate predicate(SATISFIES);
			predicate.templateVar_ = templateVar;
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
				case SATISFIES:
				{
					return Predicate::Satisfies(satisfiesTemplateVar(), satisfiesRequirement());
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
				case SATISFIES:
				{
					return Predicate::Satisfies(satisfiesTemplateVar(), satisfiesRequirement()->substitute(templateVarMap));
				}
				case VARIABLE:
				{
					const auto iterator = templateVarMap.find(variableTemplateVar());
					
					if (iterator == templateVarMap.end()) {
						return Predicate::Variable(variableTemplateVar());
					}
					
					const auto& templateValue = iterator->second;
					
					if (templateValue.isConstant()) {
						assert(templateValue.constant().kind() == Constant::BOOLEAN);
						return templateValue.constant().boolValue() ? Predicate::True() : Predicate::False();
					} else {
						return Predicate::Variable(variableTemplateVar());
					}
				}
			}
			
			throw std::logic_error("Unknown predicate kind.");
		}
		
		Predicate::Kind Predicate::kind() const {
			return kind_;
		}
		
		bool Predicate::isTrue() const {
			return kind_ == TRUE;
		}
		
		bool Predicate::isFalse() const {
			return kind_ == FALSE;
		}
		
		bool Predicate::isAnd() const {
			return kind_ == AND;
		}
		
		bool Predicate::isSatisfies() const {
			return kind_ == SATISFIES;
		}
		
		bool Predicate::isVariable() const {
			return kind_ == VARIABLE;
		}
		
		const Predicate& Predicate::andLeft() const {
			assert(isAnd());
			return *left_;
		}
		
		const Predicate& Predicate::andRight() const {
			assert(isAnd());
			return *right_;
		}
		
		TemplateVar* Predicate::satisfiesTemplateVar() const {
			assert(isSatisfies());
			return templateVar_;
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
				case SATISFIES:
				{
					return satisfiesTemplateVar() == other.satisfiesTemplateVar() &&
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
				case SATISFIES:
				{
					hasher.add(satisfiesTemplateVar());
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
				case SATISFIES:
				{
					return makeString("satisfies(templateVar: %s, requirement: %s)",
						satisfiesTemplateVar()->toString().c_str(),
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

