#ifndef LOCIC_SEMANTICANALYSIS_METHODSETELEMENT_HPP
#define LOCIC_SEMANTICANALYSIS_METHODSETELEMENT_HPP

#include <string>

#include <locic/AST/TemplateVarArray.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/TypeArray.hpp>
#include <locic/Support/Array.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace SEM {
		
		class FunctionType;
		
	}
	
	namespace SemanticAnalysis {
		
		class MethodSetElement {
			public:
				MethodSetElement(Array<AST::TemplateVar*, 8> templateVariables,
					SEM::Predicate constPredicate, SEM::Predicate noexceptPredicate, SEM::Predicate requirePredicate,
					bool isStatic, const SEM::Type* returnType, SEM::TypeArray parameterTypes);
				
				MethodSetElement(MethodSetElement&&) = default;
				MethodSetElement& operator=(MethodSetElement&&) = default;
				
				MethodSetElement copy() const;
				
				MethodSetElement withRequirement(SEM::Predicate requirement) const;
				MethodSetElement withNoExceptPredicate(SEM::Predicate newNoExceptPredicate) const;
				
				const AST::TemplateVarArray& templateVariables() const;
				const SEM::Predicate& constPredicate() const;
				const SEM::Predicate& noexceptPredicate() const;
				const SEM::Predicate& requirePredicate() const;
				
				bool isStatic() const;
				const SEM::Type* returnType() const;
				const SEM::TypeArray& parameterTypes() const;
				
				SEM::FunctionType createFunctionType(bool isTemplated) const;
				
				std::size_t hash() const;
				
				bool operator==(const MethodSetElement& methodSetElement) const;
				
				std::string toString() const;
				
			private:
				AST::TemplateVarArray templateVariables_;
				SEM::Predicate constPredicate_;
				SEM::Predicate noexceptPredicate_;
				SEM::Predicate requirePredicate_;
				bool isStatic_;
				const SEM::Type* returnType_;
				SEM::TypeArray parameterTypes_;
				
		};
		
	}
	
}

#endif
