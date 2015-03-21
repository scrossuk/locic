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
				SEM::Predicate argNoexceptPredicate,
				SEM::Predicate argRequirePredicate,
				const bool argIsStatic,
				const SEM::Type* const argReturnType,
				SEM::TypeArray argParameterTypes
			)
			: templateVariables_(std::move(argTemplateVariables)),
			constPredicate_(std::move(argConstPredicate)),
			noexceptPredicate_(std::move(argNoexceptPredicate)),
			requirePredicate_(std::move(argRequirePredicate)),
			isStatic_(argIsStatic),
			returnType_(argReturnType),
			parameterTypes_(std::move(argParameterTypes)) { }
		
		MethodSetElement MethodSetElement::copy() const {
			return MethodSetElement(templateVariables().copy(), constPredicate().copy(),
				noexceptPredicate().copy(),
				requirePredicate().copy(),
				isStatic(), returnType(), parameterTypes().copy());
		}
		
		const SEM::TemplateVarArray& MethodSetElement::templateVariables() const {
			return templateVariables_;
		}
		
		const SEM::Predicate& MethodSetElement::constPredicate() const {
			return constPredicate_;
		}
		
		const SEM::Predicate& MethodSetElement::noexceptPredicate() const {
			return noexceptPredicate_;
		}
		
		const SEM::Predicate& MethodSetElement::requirePredicate() const {
			return requirePredicate_;
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
			return SEM::Type::Function(isVarArg, isMethod, isTemplated, noexceptPredicate().copy(), returnType(), parameterTypes().copy());
		}
		
		std::size_t MethodSetElement::hash() const {
			std::size_t seed = 0;
			
			boost::hash_combine(seed, templateVariables().hash());
			boost::hash_combine(seed, constPredicate().hash());
			boost::hash_combine(seed, noexceptPredicate().hash());
			boost::hash_combine(seed, requirePredicate().hash());
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
				noexceptPredicate() == methodSetElement.noexceptPredicate() &&
				requirePredicate() == methodSetElement.requirePredicate() &&
				returnType() == methodSetElement.returnType() &&
				parameterTypes() == methodSetElement.parameterTypes() &&
				isStatic() == methodSetElement.isStatic();
		}
		
		std::string MethodSetElement::toString() const {
			return makeString("MethodSetElement(constPredicate: %s, noexceptPredicate: %s, requirePredicate: %s, returnType: %s, ...)",
				constPredicate().toString().c_str(),
				noexceptPredicate().toString().c_str(),
				requirePredicate().toString().c_str(),
				returnType()->toString().c_str());
		}
		
		const MethodSet* MethodSet::getEmpty(const Context& context) {
			return MethodSet::get(context, {});
		}
		
		const MethodSet* MethodSet::get(const Context& context, ElementSet elements) {
			return context.getMethodSet(MethodSet(context, std::move(elements)));
		}
		
		MethodSet::MethodSet(const Context& pContext, ElementSet elements)
			: context_(pContext), elements_(std::move(elements)) { }
		
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
		
		std::size_t MethodSet::hash() const {
			std::size_t seed = 0;
			
			for (const auto& element: elements_) {
				boost::hash_combine(seed, element.first.hash());
				boost::hash_combine(seed, element.second.hash());
			}
			
			return seed;
		}
		
		bool MethodSet::operator==(const MethodSet& methodSet) const {
			return elements_ == methodSet.elements_;
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
			
			return makeString("MethodSet(elements: { %s })",
				elementsString.c_str());
		}
		
		SEM::Predicate getPredicateForSatisfyRequirements(Context& /*context*/, const MethodSet* const checkSet, const MethodSet* const requireSet) {
			auto predicate = SEM::Predicate::True();
			
			auto checkIterator = checkSet->begin();
			auto requireIterator = requireSet->begin();
			
			for (; requireIterator != requireSet->end(); ++checkIterator) {
				const auto& requireFunctionName = requireIterator->first;
				
				if (checkIterator == checkSet->end()) {
					break;
				}
				
				const auto& checkFunctionName = checkIterator->first;
				const auto& checkFunctionElement = checkIterator->second;
				
				if (checkFunctionName != requireFunctionName) {
					continue;
				}
				
				// TODO: this probably needs to consider whether the requirement
				// method set element is actually satisfied.
				predicate = SEM::Predicate::And(std::move(predicate), checkFunctionElement.requirePredicate().copy());
				
				++requireIterator;
			}
			
			return predicate;
		}
		
		class PushComputingMethodSet {
		public:
			PushComputingMethodSet(Context& context, const SEM::TemplateVar* const templateVar, const SEM::Predicate& predicate)
			: context_(context) {
				context_.pushComputingMethodSet(templateVar, predicate);
			}
			
			~PushComputingMethodSet() {
				context_.popComputingMethodSet();
			}
			
		private:
			Context& context_;
			
		};
		
		const MethodSet* getMethodSetForRequiresPredicate(Context& context, const SEM::TemplateVar* const templateVar, const SEM::Predicate& requiresPredicate) {
			// Avoid cycles such as:
			// 
			// template <typename A, typename B, typename C>
			// require(A : B and B : C and C : A)
			// interface SomeType { }
			// 
			// This would cause the requirements to be repeatedly
			// queried for the types in order of the cycle; using
			// a stack of method sets currently being computed
			// provides cycle detection.
			if (context.isComputingMethodSet(templateVar, requiresPredicate)) {
				// Return empty set since the template variable extending
				// itself doesn't provide any new information.
				return MethodSet::getEmpty(context);
			}
			
			PushComputingMethodSet pushComputingMethodSet(context, templateVar, requiresPredicate);
			
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
					if (requiresPredicate.satisfiesType()->isTemplateVar()) {
						if (requiresPredicate.satisfiesType()->getTemplateVar() == templateVar) {
							return getTypeMethodSet(context, requiresPredicate.satisfiesRequirement());
						} else {
							return MethodSet::getEmpty(context);
						}
					} else {
						const auto sourceSet = getMethodSetForObjectType(context, requiresPredicate.satisfiesType());
						const auto requireSet = getMethodSetForObjectType(context, requiresPredicate.satisfiesRequirement());
						
						const auto nextLevelRequires = getPredicateForSatisfyRequirements(context, sourceSet, requireSet);
						return getMethodSetForRequiresPredicate(context, templateVar, nextLevelRequires);
					}
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
			
			const auto typeInstance = objectType->getObjectType();
			const auto templateVarMap = objectType->generateTemplateVarMap();
			
			// Conservatively assume object type is const if result is undetermined.
			const bool isConstObjectDefault = true;
			
			// TODO: also push this into the method set so that no predicate evaluation is performed in this function.
			const bool isConstObject = evaluatePredicateWithDefault(context, objectType->constPredicate(), templateVarMap, isConstObjectDefault);
			
			for (const auto& functionPair: typeInstance->functions()) {
				const auto& functionName = functionPair.first;
				const auto& function = functionPair.second;
				
				auto constPredicate = function->constPredicate().substitute(templateVarMap);
				auto noexceptPredicate = function->type()->functionNoExceptPredicate().substitute(templateVarMap);
				auto requirePredicate = function->requiresPredicate().substitute(templateVarMap);
				
				if (isConstObject && !function->isStaticMethod()) {
					// Method needs to be const.
					requirePredicate = SEM::Predicate::And(std::move(requirePredicate), constPredicate.copy());
				}
				
				const auto functionType = function->type()->substitute(templateVarMap);
				
				const bool isStatic = function->isStaticMethod();
				MethodSetElement functionElement(
					function->templateVariables().copy(),
					std::move(constPredicate),
					std::move(noexceptPredicate),
					std::move(requirePredicate),
					isStatic,
					functionType->getFunctionReturnType(),
					functionType->getFunctionParameterTypes().copy());
				
				elements.push_back(std::make_pair(functionName, std::move(functionElement)));
			}
			
			// Sort the elements.
			std::sort(elements.begin(), elements.end(), comparePairKeys<MethodSet::Element>);
			
			const auto result = MethodSet::get(context, std::move(elements));
			context.addMethodSet(objectType, result);
			return result;
		}
		
		const MethodSet* getTypeMethodSet(Context& context, const SEM::Type* const rawType) {
			assert(context.methodSetsComplete());
			
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
			
			return MethodSet::get(setA->context(), std::move(elements));
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
					// TODO: Check both methods to ensure they're compatible.
					elements.push_back(std::make_pair(nameA, iteratorA->second.copy()));
					++iteratorA;
					++iteratorB;
				}
			}
			
			// Add any methods left over in one of the sets.
			auto addIterator = (iteratorA != setA->end()) ? iteratorA : iteratorB;
			const auto endIterator = (iteratorA != setA->end()) ? setA->end() : setB->end();
			
			while (addIterator != endIterator) {
				elements.push_back(std::make_pair(addIterator->first, addIterator->second.copy()));
				++addIterator;
			}
			
			return MethodSet::get(setA->context(), std::move(elements));
		}
		
		SEM::TemplateVarMap generateSatisfyTemplateVarMap(const MethodSetElement& checkElement, const MethodSetElement& requireElement) {
			SEM::TemplateVarMap templateVarMap;
			
			// Very basic template deduction.
			for (const auto& templateVar: checkElement.templateVariables()) {
				auto selfRefValue = templateVar->selfRefValue();
				
				if (checkElement.constPredicate().isVariable() && checkElement.constPredicate().variableTemplateVar() == templateVar) {
					if (requireElement.constPredicate().isTrivialBool()) {
						const bool chosenValue = requireElement.constPredicate().isTrue() ? true : false;
						const auto chosenConstant = chosenValue ? Constant::True() : Constant::False();
						auto value = SEM::Value::Constant(chosenConstant, selfRefValue.type());
						templateVarMap.insert(std::make_pair(templateVar, std::move(value)));
					} else if (requireElement.constPredicate().isVariable()) {
						auto value = SEM::Value::PredicateExpr(SEM::Predicate::False(), selfRefValue.type());
						templateVarMap.insert(std::make_pair(templateVar, std::move(value)));
					} else {
						auto value = SEM::Value::PredicateExpr(requireElement.constPredicate().copy(), selfRefValue.type());
						templateVarMap.insert(std::make_pair(templateVar, std::move(value)));
					}
				} else {
					templateVarMap.insert(std::make_pair(templateVar, std::move(selfRefValue)));
				}
			}
			
			return templateVarMap;
		}
		
		constexpr bool DEBUG_METHOD_SET_ELEMENT = false;
		constexpr bool DEBUG_METHOD_SET = false;
		
		bool methodSetElementSatisfiesRequirement(Context& context, const String& functionName, const MethodSetElement& checkFunctionElement, const MethodSetElement& requireFunctionElement) {
			const auto satisfyTemplateVarMap = generateSatisfyTemplateVarMap(checkFunctionElement, requireFunctionElement);
			
			const auto reducedConstPredicate = reducePredicate(context, checkFunctionElement.constPredicate().substitute(satisfyTemplateVarMap));
			
			// The requirement method's const predicate needs to imply the
			// const predicate of the provided method (e.g. if the requirement
			// method is const, then the provided method must also be, but not
			// vice versa).
			if (!requireFunctionElement.constPredicate().implies(reducedConstPredicate)) {
				if (DEBUG_METHOD_SET_ELEMENT) {
					printf("\nConst predicate implication failed for '%s'.\n    Source: %s\n    Require: %s\n\n",
					       functionName.c_str(),
					       reducedConstPredicate.toString().c_str(),
					       requireFunctionElement.constPredicate().toString().c_str()
					);
				}
				return false;
			}
			
			const auto reducedRequirePredicate = reducePredicate(context, checkFunctionElement.requirePredicate().substitute(satisfyTemplateVarMap));
			
			// The requirement method's require predicate needs to imply the
			// require predicate of the provided method.
			if (!requireFunctionElement.requirePredicate().implies(reducedRequirePredicate)) {
				if (DEBUG_METHOD_SET_ELEMENT) {
					printf("\nRequire predicate implication failed for '%s'.\n    Source: %s\n    Require: %s\n\n",
					       functionName.c_str(),
					       reducedRequirePredicate.toString().c_str(),
					       requireFunctionElement.requirePredicate().toString().c_str()
					);
				}
				return false;
			}
			
			const auto reducedNoexceptPredicate = reducePredicate(context, checkFunctionElement.noexceptPredicate().substitute(satisfyTemplateVarMap));
			
			// Can't cast throwing method to noexcept method.
			if (!requireFunctionElement.noexceptPredicate().implies(reducedNoexceptPredicate)) {
				if (DEBUG_METHOD_SET_ELEMENT) {
					printf("\nNoexcept predicate implication failed for '%s'.\n    Source: %s\n    Require: %s\n\n",
					       functionName.c_str(),
					       reducedNoexceptPredicate.toString().c_str(),
					       requireFunctionElement.noexceptPredicate().toString().c_str()
					);
				}
				return false;
			}
			
			// Can't cast between static/non-static methods.
			if (checkFunctionElement.isStatic() != requireFunctionElement.isStatic()) {
				if (DEBUG_METHOD_SET_ELEMENT) {
					printf("\nStatic-ness doesn't match for '%s'.\n    Source: %s\n    Require: %s\n\n",
					       functionName.c_str(),
					       checkFunctionElement.isStatic() ? "static" : "not static",
					       requireFunctionElement.isStatic() ? "static" : "not static"
					);
				}
				return false;
			}
			
			const auto& firstList = checkFunctionElement.parameterTypes();
			const auto& secondList = requireFunctionElement.parameterTypes();
			
			if (firstList.size() != secondList.size()) {
				if (DEBUG_METHOD_SET_ELEMENT) {
					printf("\nDifferent number of parameters for '%s'.\n    Source: %llu\n    Require: %llu\n\n",
					       functionName.c_str(),
					       (unsigned long long) firstList.size(),
					       (unsigned long long) secondList.size()
					);
				}
				return false;
			}
			
			for (size_t i = 0; i < firstList.size(); i++) {
				if (firstList.at(i)->substitute(satisfyTemplateVarMap) != secondList.at(i)) {
					if (DEBUG_METHOD_SET_ELEMENT) {
						printf("\nParameter types don't match for '%s' (param %llu).\n    Source: %s\n    Require: %s\n\n",
						       functionName.c_str(),
						       (unsigned long long) i,
						       firstList.at(i)->substitute(satisfyTemplateVarMap)->toString().c_str(),
						       secondList.at(i)->toString().c_str()
						);
					}
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
				if (DEBUG_METHOD_SET_ELEMENT) {
					printf("\nReturn type doesn't match for '%s'.\n    Source: %s\n    Require: %s\n\n",
					       functionName.c_str(),
					       checkFunctionElement.returnType()->substitute(satisfyTemplateVarMap)->toString().c_str(),
					       requireFunctionElement.returnType()->toString().c_str()
					);
				}
				return false;
			}
			
			return true;
		}
		
		bool methodSetSatisfiesRequirement(Context& context, const MethodSet* const checkSet, const MethodSet* const requireSet) {
			auto checkIterator = checkSet->begin();
			auto requireIterator = requireSet->begin();
			
			for (; requireIterator != requireSet->end(); ++checkIterator) {
				const auto& requireFunctionName = requireIterator->first;
				const auto& requireFunctionElement = requireIterator->second;
				
				if (checkIterator == checkSet->end()) {
					// If all our methods have been considered, but
					// there's still an required method to consider, then
					// that method must NOT be present in our set.
					if (DEBUG_METHOD_SET_ELEMENT) {
						printf("\nMethod not found: %s\n\n", requireFunctionName.c_str());
					}
					
					if (DEBUG_METHOD_SET) {
						printf("\n...in methodSetSatisfiesRequirement:\n    Source: %s\n    Require: %s\n\n",
							formatMessage(checkSet->toString()).c_str(),
							formatMessage(requireSet->toString()).c_str());
					}
					return false;
				}
				
				const auto& checkFunctionName = checkIterator->first;
				const auto& checkFunctionElement = checkIterator->second;
				
				if (checkFunctionName != requireFunctionName) {
					continue;
				}
				
				if (!methodSetElementSatisfiesRequirement(context, checkFunctionName, checkFunctionElement, requireFunctionElement)) {
					if (DEBUG_METHOD_SET) {
						printf("\n...in methodSetSatisfiesRequirement:\n    Source: %s\n    Require: %s\n\n",
							formatMessage(checkSet->toString()).c_str(),
							formatMessage(requireSet->toString()).c_str());
					}
					return false;
				}
				
				++requireIterator;
			}
			
			return true;
		}
		
	}
	
}

