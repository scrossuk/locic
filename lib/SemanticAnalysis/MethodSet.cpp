#include <assert.h>

#include <algorithm>
#include <map>
#include <string>

#include <boost/functional/hash.hpp>

#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/MethodSet.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		MethodSetElement::MethodSetElement(
				SEM::TemplateVarArray argTemplateVariables,
				SEM::Predicate argConstPredicate,
				const bool argIsNoExcept,
				const bool argIsStatic,
				const SEM::Type* const argReturnType,
				SEM::TypeArray argParameterTypes
			)
			: templateVariables_(std::move(argTemplateVariables)),
			constPredicate_(std::move(argConstPredicate)),
			isNoExcept_(argIsNoExcept),
			isStatic_(argIsStatic),
			returnType_(argReturnType),
			parameterTypes_(std::move(argParameterTypes)) { }
		
		MethodSetElement MethodSetElement::copy() const {
			return MethodSetElement(templateVariables().copy(), constPredicate().copy(), isNoExcept(), isStatic(), returnType(), parameterTypes().copy());
		}
		
		const SEM::TemplateVarArray& MethodSetElement::templateVariables() const {
			return templateVariables_;
		}
		
		const SEM::Predicate& MethodSetElement::constPredicate() const {
			return constPredicate_;
		}
		
		bool MethodSetElement::isNoExcept() const {
			return isNoExcept_;
		}
		
		bool MethodSetElement::isStatic() const {
			return isStatic_;
		}
		
		const SEM::Type* MethodSetElement::returnType() const {
			return returnType_;
		}
		
		const SEM::TypeArray& MethodSetElement::parameterTypes() const {
			return parameterTypes_;
		}
		
		const SEM::Type* MethodSetElement::createFunctionType(const bool isTemplated) const {
			const bool isVarArg = false;
			const bool isMethod = !isStatic();
			return SEM::Type::Function(isVarArg, isMethod, isTemplated, isNoExcept(), returnType(), parameterTypes().copy());
		}
		
		std::size_t MethodSetElement::hash() const {
			std::size_t seed = 0;
			
			boost::hash_combine(seed, templateVariables().hash());
			boost::hash_combine(seed, constPredicate().hash());
			boost::hash_combine(seed, isNoExcept());
			boost::hash_combine(seed, isStatic());
			boost::hash_combine(seed, returnType());
			
			for (const auto& parameterType: parameterTypes()) {
				boost::hash_combine(seed, parameterType);
			}
			
			return seed;
		}
		
		bool MethodSetElement::operator==(const MethodSetElement& methodSetElement) const {
			return templateVariables() == methodSetElement.templateVariables() &&
				constPredicate() == methodSetElement.constPredicate() &&
				returnType() == methodSetElement.returnType() &&
				parameterTypes() == methodSetElement.parameterTypes() &&
				isNoExcept() == methodSetElement.isNoExcept() &&
				isStatic() == methodSetElement.isStatic();
		}
		
		const MethodSet* MethodSet::getEmpty(const Context& context) {
			return MethodSet::get(context, {}, {});
		}
		
		const MethodSet* MethodSet::get(const Context& context, ElementSet elements, FilterSet filters) {
			return context.getMethodSet(MethodSet(context, std::move(elements), std::move(filters)));
		}
		
		MethodSet::MethodSet(const Context& pContext, ElementSet elements, FilterSet filters)
			: context_(pContext), elements_(std::move(elements)), filters_(std::move(filters)) { }
		
		const Context& MethodSet::context() const {
			return context_;
		}
		
		MethodSet::iterator MethodSet::begin() const {
			return elements_.begin();
		}
		
		MethodSet::iterator MethodSet::end() const {
			return elements_.end();
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
		
		const MethodSet::FilterSet& MethodSet::filterSet() const {
			return filters_;
		}
		
		MethodSet::FilterReason MethodSet::getFilterReason(const String& name) const {
			if (hasMethod(name)) {
				return NotFiltered;
			}
			
			const auto result = pairBinarySearch(filters_.begin(), filters_.end(), name);
			if (result != filters_.end()) {
				return result->second;
			}
			
			return NotFound;
		}
		
		/*const MethodSet* MethodSet::substitute(const SEM::TemplateVarMap& templateAssignments) const {
			ElementSet elements;
			elements.reserve(size());
			
			for (const auto& elementPair: *this) {
				const auto& elementName = elementPair.first;
				const auto& element = elementPair.second;
				elements.push_back(std::make_pair(elementName, element.substitute(templateAssignments)));
			}
			
			return MethodSet::get(context(), std::move(elements));
		}*/
		
		std::size_t MethodSet::hash() const {
			std::size_t seed = 0;
			
			for (const auto& filter: filters_) {
				boost::hash_combine(seed, filter.first.hash());
				boost::hash_combine(seed, filter.second);
			}
			
			for (const auto& element: elements_) {
				boost::hash_combine(seed, element.first.hash());
				boost::hash_combine(seed, element.second.hash());
			}
			
			return seed;
		}
		
		bool MethodSet::operator==(const MethodSet& methodSet) const {
			return filters_ == methodSet.filters_ && elements_ == methodSet.elements_;
		}
		
		const MethodSet* getMethodSetForRequiresPredicate(Context& context, const SEM::TemplateVar* const templateVar, const SEM::Predicate& requiresPredicate) {
			switch (requiresPredicate.kind()) {
				case SEM::Predicate::TRUE:
				case SEM::Predicate::FALSE:
				case SEM::Predicate::VARIABLE:
				{
					return MethodSet::getEmpty(context);
				}
				case SEM::Predicate::AND:
				{
					const auto leftMethodSet = getMethodSetForRequiresPredicate(context, templateVar, requiresPredicate.andLeft());
					const auto rightMethodSet = getMethodSetForRequiresPredicate(context, templateVar, requiresPredicate.andRight());
					return unionMethodSets(leftMethodSet, rightMethodSet);
				}
				case SEM::Predicate::OR:
				{
					const auto leftMethodSet = getMethodSetForRequiresPredicate(context, templateVar, requiresPredicate.orLeft());
					const auto rightMethodSet = getMethodSetForRequiresPredicate(context, templateVar, requiresPredicate.orRight());
					return intersectMethodSets(leftMethodSet, rightMethodSet);
				}
				case SEM::Predicate::SATISFIES:
				{
					if (requiresPredicate.satisfiesTemplateVar() != templateVar) {
						return MethodSet::getEmpty(context);
					}
					
					return getTypeMethodSet(context, requiresPredicate.satisfiesRequirement());
				}
			}
			
			throw std::logic_error("Unknown predicate kind.");
		}
		
		namespace {
			
			template <typename PairType>
			bool comparePairKeys(const PairType& a, const PairType& b) {
				return a.first < b.first;
			}
			
		}
		
		const MethodSet* getMethodSetForObjectType(Context& context, const SEM::Type* const objectType) {
			assert(objectType->isObject());
			
			const auto existingMethodSet = context.findMethodSet(objectType);
			if (existingMethodSet != nullptr) {
				return existingMethodSet;
			}
			
			MethodSet::ElementSet elements;
			MethodSet::FilterSet filters;
			
			const auto typeInstance = objectType->getObjectType();
			const auto templateVarMap = objectType->generateTemplateVarMap();
			
			// Conservatively assume object type is const if result is undetermined.
			const bool isConstObjectDefault = true;
			
			const bool isConstObject = evaluatePredicateWithDefault(context, objectType->constPredicate(), templateVarMap, isConstObjectDefault);
			
			for (const auto& functionPair: typeInstance->functions()) {
				const auto& functionName = functionPair.first;
				const auto& function = functionPair.second;
				
				// Conservatively assume methods are not const if result is undetermined.
				const bool isConstMethodDefault = false;
				
				auto constPredicate = function->constPredicate().substitute(templateVarMap).reduceToDependencies(function->templateVariables(), isConstMethodDefault);
				assert(constPredicate.dependsOnOnly(function->templateVariables()));
				
				// Evaluate the const predicate; this may not be possible if the
				// relevant template variables aren't instantiated.
				const Optional<bool> isConstMethodOptional = evaluatePredicate(context, constPredicate, templateVarMap);
				
				// If the method is already known to NOT be const, and the
				// parent object is const, then we can eliminate this method.
				if (isConstObject && isConstMethodOptional && !(*isConstMethodOptional) && !function->isStaticMethod()) {
					// Filter out this function.
					filters.push_back(std::make_pair(functionName, MethodSet::IsMutator));
					continue;
				}
				
				// TODO: also skip unsatisfied requirement specifiers.
				
				const auto functionType = function->type()->substitute(templateVarMap);
				
				const bool isNoExcept = functionType->isFunctionNoExcept();
				const bool isStatic = function->isStaticMethod();
				MethodSetElement functionElement(
					function->templateVariables().copy(),
					std::move(constPredicate),
					isNoExcept, isStatic,
					functionType->getFunctionReturnType(),
					functionType->getFunctionParameterTypes().copy());
				
				elements.push_back(std::make_pair(functionName, std::move(functionElement)));
			}
			
			// Sort the elements.
			std::sort(elements.begin(), elements.end(), comparePairKeys<MethodSet::Element>);
			std::sort(filters.begin(), filters.end(), comparePairKeys<MethodSet::Filter>);
			
			const auto result = MethodSet::get(context, std::move(elements), std::move(filters));
			context.addMethodSet(objectType, result);
			return result;
		}
		
		const MethodSet* getTypeMethodSet(Context& context, const SEM::Type* const rawType) {
			const auto type = rawType->resolveAliases();
			assert(type->isObject() || type->isTemplateVar());
			if (type->isObject()) {
				return getMethodSetForObjectType(context, type);
			} else {
				const auto& requiresPredicate = lookupRequiresPredicate(context.scopeStack());
				
				// TODO: need to filter out non-const methods!
				return getMethodSetForRequiresPredicate(context, type->getTemplateVar(), requiresPredicate);
			}
		}
		
		const MethodSet* intersectMethodSets(const MethodSet* setA, const MethodSet* setB) {
			assert(&(setA->context()) == &(setB->context()));
			
			auto iteratorA = setA->begin();
			auto iteratorB = setB->begin();
			
			MethodSet::ElementSet elements;
			elements.reserve(std::max<size_t>(setA->size(), setB->size()));
			
			while (iteratorA != setA->end() && iteratorB != setB->end()) {
				const auto& nameA = iteratorA->first;
				const auto& nameB = iteratorB->first;
				
				if (nameA == nameB) {
					// Merge methods!
					throw std::runtime_error("Merging methods not supported!");
				}
			}
			
			// Ignore any methods left over, since we're only concerned
			// about methods that exist in both sets.
			
			return MethodSet::get(setA->context(), std::move(elements), {});
		}
		
		const MethodSet* unionMethodSets(const MethodSet* setA, const MethodSet* setB) {
			assert(&(setA->context()) == &(setB->context()));
			
			auto iteratorA = setA->begin();
			auto iteratorB = setB->begin();
			
			MethodSet::ElementSet elements;
			elements.reserve(std::max<size_t>(setA->size(), setB->size()));
			
			while (iteratorA != setA->end() && iteratorB != setB->end()) {
				const auto& nameA = iteratorA->first;
				const auto& nameB = iteratorB->first;
				
				if (nameA < nameB) {
					elements.push_back(std::make_pair(nameA, iteratorA->second.copy()));
					++iteratorA;
				} else if (nameB < nameA) {
					elements.push_back(std::make_pair(nameB, iteratorB->second.copy()));
					++iteratorB;
				} else {
					// Merge methods!
					throw std::runtime_error("Merging methods not supported!");
				}
			}
			
			// Add any methods left over in one of the sets.
			auto addIterator = (iteratorA != setA->end()) ? iteratorA : iteratorB;
			const auto endIterator = (iteratorA != setA->end()) ? setA->end() : setB->end();
			
			while (addIterator != endIterator) {
				elements.push_back(std::make_pair(addIterator->first, addIterator->second.copy()));
				++addIterator;
			}
			
			// TODO: merge these properly!
			MethodSet::FilterSet filters;
			filters.reserve(setA->filterSet().size() + setB->filterSet().size());
			for (const auto& filter: setA->filterSet()) {
				filters.push_back(filter);
			}
			for (const auto& filter: setB->filterSet()) {
				filters.push_back(filter);
			}
			std::sort(filters.begin(), filters.end(), comparePairKeys<MethodSet::Filter>);
			
			return MethodSet::get(setA->context(), std::move(elements), std::move(filters));
		}
		
		SEM::TemplateVarMap generateSatisfyTemplateVarMap(const MethodSetElement& checkElement, const MethodSetElement& requireElement) {
			SEM::TemplateVarMap templateVarMap;
			
			// Very basic template deduction.
			for (const auto& templateVar: checkElement.templateVariables()) {
				auto selfRefValue = templateVar->selfRefValue();
				
				if (checkElement.constPredicate().isVariable() && checkElement.constPredicate().variableTemplateVar() == templateVar &&
					(requireElement.constPredicate().isTrue() || requireElement.constPredicate().isFalse())
				) {
					const bool chosenValue = requireElement.constPredicate().isTrue() ? true : false;
					const auto chosenConstant = chosenValue ? Constant::True() : Constant::False();
					auto value = SEM::Value::Constant(chosenConstant, selfRefValue.type());
					templateVarMap.insert(std::make_pair(templateVar, std::move(value)));
				} else {
					templateVarMap.insert(std::make_pair(templateVar, std::move(selfRefValue)));
				}
			}
			
			return templateVarMap;
		}
		
		bool methodSetSatisfiesRequirement(const MethodSet* const checkSet, const MethodSet* const requireSet) {
			auto checkIterator = checkSet->begin();
			auto requireIterator = requireSet->begin();
			
			for (; requireIterator != requireSet->end(); ++checkIterator) {
				const auto& requireFunctionName = requireIterator->first;
				const auto& requireFunctionElement = requireIterator->second;
				
				if (checkIterator == checkSet->end()) {
					// If all our methods have been considered, but
					// there's still an required method to consider, then
					// that method must NOT be present in our set.
					printf("\n\nMethod not found:\n\n%s\n\n",
						requireFunctionName.c_str());
					return false;
				}
				
				const auto& checkFunctionName = checkIterator->first;
				const auto& checkFunctionElement = checkIterator->second;
				
				if (checkFunctionName != requireFunctionName) {
					continue;
				}
				
				const auto satisfyTemplateVarMap = generateSatisfyTemplateVarMap(checkFunctionElement, requireFunctionElement);
				
				// The requirement method's const predicate needs to imply the
				// const predicate of the provided method (e.g. if the requirement
				// method is const, then the provided method must also be, but not
				// vice versa).
				if (!requireFunctionElement.constPredicate().implies(checkFunctionElement.constPredicate().substitute(satisfyTemplateVarMap))) {
					printf("\n\nNot const-compatible:\n\n%s : %s\n\n%s : %s\n\n",
						checkFunctionName.c_str(),
						checkFunctionElement.constPredicate().substitute(satisfyTemplateVarMap).toString().c_str(),
						requireFunctionName.c_str(),
						requireFunctionElement.constPredicate().toString().c_str());
					return false;
				}
				
				// Can't cast throwing method to noexcept method.
				if (!checkFunctionElement.isNoExcept() && requireFunctionElement.isNoExcept()) {
					printf("\n\nNot noexcept-compatible:\n\n%s\n\n%s\n\n",
						checkFunctionName.c_str(),
						requireFunctionName.c_str());
					return false;
				}
				
				// Can't cast between static/non-static methods.
				if (checkFunctionElement.isStatic() != requireFunctionElement.isStatic()) {
					printf("\n\nNot static-compatible:\n\n%s\n\n%s\n\n",
						checkFunctionName.c_str(),
						requireFunctionName.c_str());
					return false;
				}
				
				const auto& firstList = checkFunctionElement.parameterTypes();
				const auto& secondList = requireFunctionElement.parameterTypes();
				
				for (size_t i = 0; i < firstList.size(); i++) {
					if (firstList.at(i)->substitute(satisfyTemplateVarMap) != secondList.at(i)) {
						printf("\n\nParameter type not compatible:\n\n%s : %s\n\n%s : %s\n\n",
							checkFunctionName.c_str(), firstList.at(i)->substitute(satisfyTemplateVarMap)->toString().c_str(),
							requireFunctionName.c_str(), secondList.at(i)->toString().c_str());
						return false;
					}
				}
				
				const auto castReturnType =
					ImplicitCastTypeFormatOnly(
						checkFunctionElement.returnType()->substitute(satisfyTemplateVarMap),
						requireFunctionElement.returnType(),
						Debug::SourceLocation::Null()
					);
				
				if (castReturnType == nullptr) {
					printf("\n\nReturn type not compatible:\n\n%s : %s\n\n%s : %s\n\n",
						checkFunctionName.c_str(),
						checkFunctionElement.returnType()->substitute(satisfyTemplateVarMap)->toString().c_str(),
						requireFunctionName.c_str(),
						requireFunctionElement.returnType()->toString().c_str()
      					);
					return false;
				}
				
				++requireIterator;
			}
			
			return true;
		}
		
	}
	
}

