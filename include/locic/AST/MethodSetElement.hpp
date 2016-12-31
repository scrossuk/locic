#ifndef LOCIC_AST_METHODSETELEMENT_HPP
#define LOCIC_AST_METHODSETELEMENT_HPP

#include <string>

#include <locic/AST/TemplateVarArray.hpp>
#include <locic/AST/TypeArray.hpp>
#include <locic/AST/Predicate.hpp>
#include <locic/Support/Array.hpp>

namespace locic {
	
	namespace AST {
		
		class FunctionType;
		
		class MethodSetElement {
			public:
				MethodSetElement(Array<TemplateVar*, 8> templateVariables,
					Predicate constPredicate, Predicate noexceptPredicate, Predicate requirePredicate,
					bool isStatic, const Type* returnType, TypeArray parameterTypes);
				
				MethodSetElement(MethodSetElement&&) = default;
				MethodSetElement& operator=(MethodSetElement&&) = default;
				
				MethodSetElement copy() const;
				
				MethodSetElement withRequirement(Predicate requirement) const;
				MethodSetElement withNoExceptPredicate(Predicate newNoExceptPredicate) const;
				
				const TemplateVarArray& templateVariables() const;
				const Predicate& constPredicate() const;
				const Predicate& noexceptPredicate() const;
				const Predicate& requirePredicate() const;
				
				bool isStatic() const;
				const Type* returnType() const;
				const TypeArray& parameterTypes() const;
				
				FunctionType createFunctionType(bool isTemplated) const;
				
				std::size_t hash() const;
				
				bool operator==(const MethodSetElement& methodSetElement) const;
				
				std::string toString() const;
				
			private:
				TemplateVarArray templateVariables_;
				Predicate constPredicate_;
				Predicate noexceptPredicate_;
				Predicate requirePredicate_;
				bool isStatic_;
				const Type* returnType_;
				TypeArray parameterTypes_;
				
		};
		
	}
	
}

#endif
