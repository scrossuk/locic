#ifndef LOCIC_AST_METHODSET_HPP
#define LOCIC_AST_METHODSET_HPP

#include <string>

#include <locic/AST/MethodSetElement.hpp>
#include <locic/AST/TemplateVarArray.hpp>
#include <locic/AST/Predicate.hpp>

#include <locic/Support/Array.hpp>
#include <locic/Support/Optional.hpp>
#include <locic/Support/String.hpp>

namespace locic {
	
	namespace AST {
		
		class Context;
		
		constexpr size_t MethodSetElementBaseSize = 32;
		
		class MethodSet {
			public:
				typedef std::pair<String, MethodSetElement> Element;
				typedef Array<Element, MethodSetElementBaseSize> ElementSet;
				typedef ElementSet::const_iterator iterator;
				
				static const MethodSet* getEmpty(const Context& context);
				
				static const MethodSet* get(const Context& context, Predicate constPredicate, ElementSet elements);
				
				MethodSet(MethodSet&&) = default;
				MethodSet& operator=(MethodSet&&) = default;
				
				const Context& context() const;
				
				const MethodSet* withConstPredicate(Predicate constPredicate) const;
				const MethodSet* withRequirement(Predicate requirement) const;
				
				const Predicate& constPredicate() const;
				
				iterator begin() const;
				iterator end() const;
				
				bool empty() const;
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
				
				MethodSet(const Context& context, Predicate constPredicate, ElementSet elements);
				
				const Context& context_;
				Predicate constPredicate_;
				ElementSet elements_;
				mutable Optional<size_t> cachedHashValue_;
				
		};
		
	}
	
}

#endif
