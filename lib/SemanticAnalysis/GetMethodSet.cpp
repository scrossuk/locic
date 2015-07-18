#include <assert.h>

#include <algorithm>
#include <map>
#include <string>

#include <boost/functional/hash.hpp>

#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/GetMethodSet.hpp>
#include <locic/SemanticAnalysis/MethodSet.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
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
 						// from which we can then calculate a method set.
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
		
	}
	
}

