#ifndef LOCIC_SEMANTICANALYSIS_METHODSET_HPP
#define LOCIC_SEMANTICANALYSIS_METHODSET_HPP

#include <string>

#include <locic/Support/Array.hpp>
#include <locic/SEM.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		class Context;
		
		class MethodSetElement {
			public:
				MethodSetElement(Array<SEM::TemplateVar*, 8> templateVariables,
					SEM::Predicate constPredicate, SEM::Predicate noexceptPredicate, SEM::Predicate requirePredicate,
					bool isStatic, const SEM::Type* returnType, SEM::TypeArray parameterTypes);
				
				MethodSetElement(MethodSetElement&&) = default;
				MethodSetElement& operator=(MethodSetElement&&) = default;
				
				MethodSetElement copy() const;
				
				MethodSetElement withRequirement(SEM::Predicate requirement) const;
				
				const SEM::TemplateVarArray& templateVariables() const;
				const SEM::Predicate& constPredicate() const;
				const SEM::Predicate& noexceptPredicate() const;
				const SEM::Predicate& requirePredicate() const;
				
				bool isStatic() const;
				const SEM::Type* returnType() const;
				const SEM::TypeArray& parameterTypes() const;
				
				const SEM::Type* createFunctionType(bool isTemplated) const;
				
				std::size_t hash() const;
				
				bool operator==(const MethodSetElement& methodSetElement) const;
				
				std::string toString() const;
				
			private:
				SEM::TemplateVarArray templateVariables_;
				SEM::Predicate constPredicate_;
				SEM::Predicate noexceptPredicate_;
				SEM::Predicate requirePredicate_;
				bool isStatic_;
				const SEM::Type* returnType_;
				SEM::TypeArray parameterTypes_;
				
		};
		
		constexpr size_t MethodSetElementBaseSize = 32;
		
		class MethodSet {
			public:
				typedef std::pair<String, MethodSetElement> Element;
				typedef Array<Element, MethodSetElementBaseSize> ElementSet;
				typedef ElementSet::const_iterator iterator;
				
				static const MethodSet* getEmpty(const Context& context);
				
				static const MethodSet* get(const Context& context, SEM::Predicate constPredicate, ElementSet elements);
				
				MethodSet(MethodSet&&) = default;
				MethodSet& operator=(MethodSet&&) = default;
				
				const Context& context() const;
				
				const MethodSet* withConstPredicate(SEM::Predicate constPredicate) const;
				const MethodSet* withRequirement(SEM::Predicate requirement) const;
				
				const SEM::Predicate& constPredicate() const;
				
				iterator begin() const;
				iterator end() const;
				
				size_t size() const;
				
				iterator find(const String& name) const;
				
				bool hasMethod(const String& name) const;
				
				std::size_t hash() const;
				
				bool operator==(const MethodSet& methodSet) const;
				
				std::string toString() const;
				
			private:
				// Non-copyable.
				MethodSet(const MethodSet&) = delete;
				MethodSet& operator=(const MethodSet&) = delete;
				
				MethodSet(const Context& context, SEM::Predicate constPredicate, ElementSet elements);
				
				const Context& context_;
				SEM::Predicate constPredicate_;
				ElementSet elements_;
				
		};
		
		const MethodSet* getMethodSetForRequiresPredicate(SEM::TemplateVar* templateVar, const SEM::Predicate& requiresPredicate);
		
		const MethodSet* getMethodSetForObjectType(Context& context, const SEM::Type* objectType);
		
		const MethodSet* getTypeMethodSet(Context& context, const SEM::Type* type);
		
		const MethodSet* intersectMethodSets(const MethodSet* setA, const MethodSet* setB);
		
		const MethodSet* unionMethodSets(const MethodSet* setA, const MethodSet* setB);
		
		bool methodSetSatisfiesRequirement(Context& context, const MethodSet* checkSet, const MethodSet* requireSet);
		
	}
	
}

namespace std {
	
	template <> struct hash<locic::SemanticAnalysis::MethodSet>
	{
		size_t operator()(const locic::SemanticAnalysis::MethodSet& value) const
		{
			return value.hash();
		}
	};
	
}

#endif
