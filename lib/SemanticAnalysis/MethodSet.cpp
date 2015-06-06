#include <assert.h>

#include <algorithm>
#include <map>
#include <string>

#include <boost/functional/hash.hpp>

#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Cast.hpp>
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
		
		MethodSetElement MethodSetElement::withRequirement(SEM::Predicate requirement) const {
			return MethodSetElement(templateVariables().copy(), constPredicate().copy(),
				noexceptPredicate().copy(),
				SEM::Predicate::And(requirePredicate().copy(), std::move(requirement)),
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
		
		SEM::FunctionType MethodSetElement::createFunctionType(const bool isTemplated) const {
			const bool isVarArg = false;
			const bool isMethod = !isStatic();
			SEM::FunctionAttributes attributes(isVarArg, isMethod, isTemplated, noexceptPredicate().copy());
			return SEM::FunctionType(std::move(attributes), returnType(), parameterTypes().copy());
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
			return MethodSet::get(context, SEM::Predicate::False(), {});
		}
		
		const MethodSet* MethodSet::get(const Context& context, SEM::Predicate constPredicate, ElementSet elements) {
			return context.getMethodSet(MethodSet(context, std::move(constPredicate), std::move(elements)));
		}
		
		const MethodSet* MethodSet::withConstPredicate(SEM::Predicate addConstPredicate) const {
			auto newConstPredicate = SEM::Predicate::Or(constPredicate().copy(), std::move(addConstPredicate));
			if (constPredicate() == newConstPredicate) {
				return this;
			}
			
			return MethodSet::get(context(), std::move(newConstPredicate), elements_.copy());
		}
		
		const MethodSet* MethodSet::withRequirement(const SEM::Predicate requirement) const {
			ElementSet newElements;
			newElements.reserve(size());
			
			for (const auto& elementPair: *this) {
				const auto& element = elementPair.second;
				auto newElement = element.withRequirement(requirement.copy());
				newElements.push_back(std::make_pair(elementPair.first, std::move(newElement)));
			}
			
			return MethodSet::get(context(), constPredicate().copy(), std::move(newElements));
		}
		
		MethodSet::MethodSet(const Context& pContext, SEM::Predicate argConstPredicate, ElementSet argElements)
			: context_(pContext), constPredicate_(std::move(argConstPredicate)),
			elements_(std::move(argElements)) { }
		
		const Context& MethodSet::context() const {
			return context_;
		}
		
		const SEM::Predicate& MethodSet::constPredicate() const {
			return constPredicate_;
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
		
		/**
		 * \brief Get predicate for satisfies requirement.
		 * 
		 * This function takes two method sets and computes
		 * what is required of the first method set to
		 * satisfy the second method set. This can then be
		 * used to generate a method set for a template
		 * variable.
		 * 
		 * For example:
		 * 
		 * template <typename T>
		 * require(pair<T, T> : movable)
		 * void function();
		 * 
		 * In this case we can get a method set for 'pair<T, T>'
		 * and for 'movable'. This will then identify that
		 * 'T : movable' must be true for 'pair<T, T> : movable'
		 * to be true, and hence we can conclude that T must
		 * be movable.
		 * 
		 * Note that it's possible that the given method set
		 * NEVER satisfies the requirement method set, and in
		 * that case no functionality can be assumed about the
		 * template variable (not handled here).
		 */
		SEM::Predicate getPredicateForSatisfyRequirements(Context& /*context*/, const MethodSet* const checkSet, const MethodSet* const requireSet) {
			auto predicate = SEM::Predicate::True();
			
			auto checkIterator = checkSet->begin();
			auto requireIterator = requireSet->begin();
			
			for (; requireIterator != requireSet->end(); ++checkIterator) {
				const auto& requireFunctionName = requireIterator->first;
				
				if (checkIterator == checkSet->end()) {
					// Method set can't be satisfied (method is missing!), so can't obtain any information from this.
					return SEM::Predicate::False();
				}
				
				const auto& checkFunctionName = checkIterator->first;
				const auto& checkFunctionElement = checkIterator->second;
				
				if (checkFunctionName != requireFunctionName) {
					continue;
				}
				
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
		
		/**
		 * \brief Extract method set from require() predicate.
		 * 
		 * This function looks at a predicate to determine the
		 * supported methods for a template variable.
		 * 
		 * For example:
		 * 
		 * interface HasCustomMethod {
		 *     void customMethod();
		 * }
		 * 
		 * template <typename T>
		 * require(T : HasCustomMethod)
		 * void function(T& object) {
		 *     object.customMethod();
		 * }
		 * 
		 * In this case we look at the predicate 'T : HasCustomMethod'
		 * and hence determine that template variable 'T' must
		 * have the method 'customMethod' inside the function.
		 */
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
						// The case of 'T : SomeRequirement', where T is a template
						// variable. Hence we just get the method set for the required
						// type since T must support that.
						if (requiresPredicate.satisfiesType()->getTemplateVar() == templateVar) {
							return getTypeMethodSet(context, requiresPredicate.satisfiesRequirement());
						} else {
							// It's not the template variable we were looking for, so ignore.
							return MethodSet::getEmpty(context);
						}
					} else {
						// This case is a little more complex; you can have something like:
						// 
						// template <typename T>
						// require(pair<T, T> : movable)
						// void function();
						// 
						// Hence we generate the method sets of both 'pair<T, T>' and
						// 'movable', and then determine what must be true of 'pair<T, T>'
						// to satisfy 'movable' by computing a new require() predicate,
// 						// from which we can then calculate a method set.
						const auto sourceSet = getTypeMethodSet(context, requiresPredicate.satisfiesType());
						const auto requireSet = getTypeMethodSet(context, requiresPredicate.satisfiesRequirement());
						
						// Compute what must be true in order for the source method set
						// to satisfy the required method set.
						const auto nextLevelRequires = getPredicateForSatisfyRequirements(context, sourceSet, requireSet);
						
						// Use the new set of requirements (as a predicate) to generate a
						// method set for our template variable.
						const auto satisfyMethodSet = getMethodSetForRequiresPredicate(context, templateVar, nextLevelRequires);
						
						// Add a requirement to every method in the set that this satisfy
						// expression evaluates to TRUE; this prevents a user specifying a
						// predicate that is always false and hence confusing us into
						// assuming the template variable has the capabilities specified.
						return satisfyMethodSet->withRequirement(requiresPredicate.copy());
					}
				}
			}
			
			throw std::logic_error("Unknown predicate kind.");
		}
		
		const MethodSet* getMethodSetForTemplateVarType(Context& context, const SEM::Type* const templateVarType, const SEM::TemplatedObject& templatedObject) {
			assert(templateVarType->isTemplateVar());
			
			// Look in the require() predicate to see what methods this
			// template variable supports.
			const auto methodSet = getMethodSetForRequiresPredicate(context, templateVarType->getTemplateVar(), templatedObject.requiresPredicate());
			
			// The template variable may be const, in which case add its predicate to
			// the method set's const predicate.
			return methodSet->withConstPredicate(templateVarType->constPredicate().copy());
		}
		
		namespace {
			
			template <typename PairType>
			bool comparePairKeys(const PairType& a, const PairType& b) {
				return a.first < b.first;
			}
			
		}
		
		const MethodSet* getMethodSetForObjectType(Context& context, const SEM::Type* const objectType) {
			assert(objectType->isObject());
			
			MethodSet::ElementSet elements;
			
			const auto typeInstance = objectType->getObjectType();
			const auto templateVarMap = objectType->generateTemplateVarMap();
			
			for (const auto& functionPair: typeInstance->functions()) {
				const auto& functionName = functionPair.first;
				const auto& function = functionPair.second;
				
				auto constPredicate = function->constPredicate().substitute(templateVarMap);
				auto noexceptPredicate = function->type().attributes().noExceptPredicate().substitute(templateVarMap);
				auto requirePredicate = function->requiresPredicate().substitute(templateVarMap);
				const auto functionType = function->type().substitute(templateVarMap);
				const bool isStatic = function->isStaticMethod();
				
				MethodSetElement functionElement(
					function->templateVariables().copy(),
					std::move(constPredicate),
					std::move(noexceptPredicate),
					std::move(requirePredicate),
					isStatic,
					functionType.returnType(),
					functionType.parameterTypes().copy());
				
				elements.push_back(std::make_pair(functionName, std::move(functionElement)));
			}
			
			// Sort the elements.
			std::sort(elements.begin(), elements.end(), comparePairKeys<MethodSet::Element>);
			
			auto constObjectPredicate = objectType->constPredicate().substitute(templateVarMap);
			
			return MethodSet::get(context, std::move(constObjectPredicate), std::move(elements));
		}
		
		const MethodSet* getTypeMethodSet(Context& context, const SEM::Type* const rawType) {
			assert(context.methodSetsComplete());
			
			const auto type = rawType->resolveAliases();
			assert(type->isObject() || type->isTemplateVar());
			
			const auto& templatedObject = lookupTemplatedObject(context.scopeStack());
			
			const auto existingMethodSet = context.findMethodSet(templatedObject, type);
			if (existingMethodSet != nullptr) {
				return existingMethodSet;
			}
			
			const auto methodSet =
				type->isObject() ?
					getMethodSetForObjectType(context, type) :
					getMethodSetForTemplateVarType(context, type, templatedObject);
			
			context.addMethodSet(templatedObject, type, methodSet);
			return methodSet;
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
			
			auto constObjectPredicate = SEM::Predicate::Or(setA->constPredicate().copy(), setB->constPredicate().copy());
			return MethodSet::get(setA->context(), std::move(constObjectPredicate), std::move(elements));
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
			
			auto constObjectPredicate = SEM::Predicate::Or(setA->constPredicate().copy(), setB->constPredicate().copy());
			return MethodSet::get(setA->context(), std::move(constObjectPredicate), std::move(elements));
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
		
		bool methodSetElementSatisfiesRequirement(Context& context, const SEM::Predicate& checkConstPredicate, const String& functionName,
				const MethodSetElement& checkFunctionElement, const MethodSetElement& requireFunctionElement) {
			const auto satisfyTemplateVarMap = generateSatisfyTemplateVarMap(checkFunctionElement, requireFunctionElement);
			
			const auto reducedConstPredicate = reducePredicate(context, checkFunctionElement.constPredicate().substitute(satisfyTemplateVarMap));
			
			// The method set's const predicate needs to imply the method's
			// const predicate.
			if (!checkConstPredicate.implies(reducedConstPredicate)) {
				if (DEBUG_METHOD_SET_ELEMENT) {
					printf("\nConst parent predicate implication failed for '%s'.\n    Parent: %s\n    Method: %s\n\n",
					       functionName.c_str(),
					       checkConstPredicate.toString().c_str(),
					       reducedConstPredicate.toString().c_str()
					);
				}
				return false;
			}
			
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
			
			const auto checkConstPredicate = reducePredicate(context, checkSet->constPredicate().copy());
			const auto requireConstPredicate = reducePredicate(context, requireSet->constPredicate().copy());
			
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
				
				const auto requireMethodConstPredicate = reducePredicate(context, requireFunctionElement.constPredicate().copy());
				
				if (!requireConstPredicate.implies(requireMethodConstPredicate)) {
					// Skip because required method is non-const inside
					// const parent.
					continue;
				}
				
				if (!methodSetElementSatisfiesRequirement(context, checkConstPredicate, checkFunctionName,
						checkFunctionElement, requireFunctionElement)) {
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

