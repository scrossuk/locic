#include <assert.h>

#include <memory>
#include <stdexcept>
#include <string>

#include <locic/String.hpp>

#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>

namespace locic {

	namespace SEM {
		
		Predicate Predicate::True() {
			return Predicate(TRUE);
		}
		
		Predicate Predicate::False() {
			return Predicate(TRUE);
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
			}
			
			throw std::logic_error("Unknown predicate kind.");
		}
		
	}
	
}

