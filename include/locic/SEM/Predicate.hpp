#ifndef LOCIC_SEM_PREDICATE_HPP
#define LOCIC_SEM_PREDICATE_HPP

#include <memory>

namespace locic {
	
	namespace SEM {
		
		class TemplateVar;
		class TemplateVarMap;
		class Type;
		
		class Predicate {
			public:
				enum Kind {
					TRUE,
					FALSE,
					AND,
					SATISFIES,
					VARIABLE
				};
				
				static Predicate True();
				
				static Predicate False();
				
				static Predicate And(Predicate left, Predicate right);
				
				static Predicate Satisfies(TemplateVar* templateVar, const Type* requirement);
				
				static Predicate Variable(TemplateVar* templateVar);
				
				Predicate(Predicate&&) = default;
				Predicate& operator=(Predicate&&) = default;
				
				Predicate copy() const;
				Predicate substitute(const TemplateVarMap&) const;
				
				Kind kind() const;
				
				bool isTrue() const;
				bool isFalse() const;
				bool isAnd() const;
				bool isSatisfies() const;
				bool isVariable() const;
				
				const Predicate& andLeft() const;
				const Predicate& andRight() const;
				
				TemplateVar* satisfiesTemplateVar() const;
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
				const Type* requirement_;
				
		};
		
	}
	
}

#endif
