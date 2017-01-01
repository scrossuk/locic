#include <locic/AST/MethodSet.hpp>

#include <algorithm>
#include <cassert>
#include <map>
#include <string>

#include <boost/functional/hash.hpp>

#include <locic/AST/Context.hpp>
#include <locic/AST/MethodSetElement.hpp>

#include <locic/Support/MakeString.hpp>

namespace locic {
	
	namespace AST {
		
		const MethodSet* MethodSet::getEmpty(const Context& context) {
			return MethodSet::get(context, Predicate::False(), {});
		}
		
		const MethodSet* MethodSet::get(const Context& context, Predicate constPredicate, ElementSet elements) {
			return context.getMethodSet(MethodSet(context, std::move(constPredicate), std::move(elements)));
		}
		
		const MethodSet* MethodSet::withConstPredicate(Predicate addConstPredicate) const {
			auto newConstPredicate = Predicate::Or(constPredicate().copy(), std::move(addConstPredicate));
			if (constPredicate() == newConstPredicate) {
				return this;
			}
			
			return MethodSet::get(context(), std::move(newConstPredicate), elements_.copy());
		}
		
		const MethodSet* MethodSet::withRequirement(const Predicate requirement) const {
			ElementSet newElements;
			newElements.reserve(size());
			
			for (const auto& elementPair: *this) {
				const auto& element = elementPair.second;
				auto newElement = element.withRequirement(requirement.copy());
				newElements.push_back(std::make_pair(elementPair.first, std::move(newElement)));
			}
			
			return MethodSet::get(context(), constPredicate().copy(), std::move(newElements));
		}
		
		MethodSet::MethodSet(const Context& pContext, Predicate argConstPredicate, ElementSet argElements)
			: context_(pContext), constPredicate_(std::move(argConstPredicate)),
			elements_(std::move(argElements)) { }
		
		const Context& MethodSet::context() const {
			return context_;
		}
		
		const Predicate& MethodSet::constPredicate() const {
			return constPredicate_;
		}
		
		MethodSet::iterator MethodSet::begin() const {
			return elements_.begin();
		}
		
		MethodSet::iterator MethodSet::end() const {
			return elements_.end();
		}
		
		bool MethodSet::empty() const {
			return elements_.empty();
		}
		
		size_t MethodSet::size() const {
			return elements_.size();
		}
		
		namespace {
			
			template <typename It, typename Key>
			It pairBinarySearch(It begin, It end, const Key& key) {
				const It failureIterator = end;
				while (true) {
					const size_t distance = end - begin;
					if (distance == 0) {
						return failureIterator;
					}
					
					const It midPoint = begin + (distance / 2);
					assert(midPoint < end);
					const auto& pairKey = midPoint->first;
					if (key < pairKey) {
						end = midPoint;
					} else if (pairKey < key) {
						begin = midPoint + 1;
					} else {
						// Found.
						return midPoint;
					}
				}
			}
			
		}
		
		MethodSet::iterator MethodSet::find(const String& name) const {
			return pairBinarySearch(begin(), end(), name);
		}
		
		bool MethodSet::hasMethod(const String& name) const {
			return find(name) != end();
		}
		
		std::size_t MethodSet::hash() const {
			if (cachedHashValue_) {
				return *cachedHashValue_;
			}
			
			std::size_t seed = 0;
			
			boost::hash_combine(seed, constPredicate().hash());
			
			for (const auto& element: elements_) {
				boost::hash_combine(seed, element.first.hash());
				boost::hash_combine(seed, element.second.hash());
			}
			
			cachedHashValue_ = make_optional(seed);
			return seed;
		}
		
		bool MethodSet::operator==(const MethodSet& methodSet) const {
			return constPredicate() == methodSet.constPredicate() && elements_ == methodSet.elements_;
		}
		
		std::string MethodSet::toString() const {
			std::string elementsString;
			
			bool first = true;
			for (const auto& element: *this) {
				if (!first) {
					elementsString += ", ";
				}
				first = false;
				
				elementsString += makeString("%s: %s",
					element.first.c_str(),
					element.second.toString().c_str());
			}
			
			return makeString("MethodSet(constPredicate: %s, elements: { %s })",
				constPredicate().toString().c_str(),
				elementsString.c_str());
		}
		
	}
	
}

