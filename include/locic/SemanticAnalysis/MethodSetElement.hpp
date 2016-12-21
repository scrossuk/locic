#ifndef LOCIC_SEMANTICANALYSIS_METHODSETELEMENT_HPP
#define LOCIC_SEMANTICANALYSIS_METHODSETELEMENT_HPP

#include <string>

#include <locic/AST/TemplateVarArray.hpp>
#include <locic/AST/TypeArray.hpp>
#include <locic/AST/Predicate.hpp>
#include <locic/Support/Array.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		class FunctionType;
		
	}
	
	namespace SemanticAnalysis {
		
		class MethodSetElement {
			public:
				MethodSetElement(Array<AST::TemplateVar*, 8> templateVariables,
					AST::Predicate constPredicate, AST::Predicate noexceptPredicate, AST::Predicate requirePredicate,
					bool isStatic, const AST::Type* returnType, AST::TypeArray parameterTypes);
				
				MethodSetElement(MethodSetElement&&) = default;
				MethodSetElement& operator=(MethodSetElement&&) = default;
				
				MethodSetElement copy() const;
				
				MethodSetElement withRequirement(AST::Predicate requirement) const;
				MethodSetElement withNoExceptPredicate(AST::Predicate newNoExceptPredicate) const;
				
				const AST::TemplateVarArray& templateVariables() const;
				const AST::Predicate& constPredicate() const;
				const AST::Predicate& noexceptPredicate() const;
				const AST::Predicate& requirePredicate() const;
				
				bool isStatic() const;
				const AST::Type* returnType() const;
				const AST::TypeArray& parameterTypes() const;
				
				AST::FunctionType createFunctionType(bool isTemplated) const;
				
				std::size_t hash() const;
				
				bool operator==(const MethodSetElement& methodSetElement) const;
				
				std::string toString() const;
				
			private:
				AST::TemplateVarArray templateVariables_;
				AST::Predicate constPredicate_;
				AST::Predicate noexceptPredicate_;
				AST::Predicate requirePredicate_;
				bool isStatic_;
				const AST::Type* returnType_;
				AST::TypeArray parameterTypes_;
				
		};
		
	}
	
}

#endif
