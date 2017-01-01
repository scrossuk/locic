#include <locic/SemanticAnalysis/GetMethodSet.hpp>

#include <algorithm>
#include <cassert>
#include <map>
#include <string>

#include <boost/functional/hash.hpp>

#include <locic/AST/MethodSet.hpp>
#include <locic/AST/Type.hpp>

#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
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
		AST::Predicate
		getPredicateForSatisfyRequirements(Context& /*context*/,
		                                   const AST::MethodSet* const checkSet,
		                                   const AST::MethodSet* const requireSet) {
			auto predicate = AST::Predicate::True();
			
			auto checkIterator = checkSet->begin();
			auto requireIterator = requireSet->begin();
			
			for (; requireIterator != requireSet->end(); ++checkIterator) {
				const auto& requireFunctionName = requireIterator->first;
				
				if (checkIterator == checkSet->end()) {
					// Method set can't be satisfied (method is missing!), so can't obtain any information from this.
					return AST::Predicate::False();
				}
				
				const auto& checkFunctionName = checkIterator->first;
				const auto& checkFunctionElement = checkIterator->second;
				
				if (checkFunctionName != requireFunctionName) {
					continue;
				}
				
				predicate = AST::Predicate::And(std::move(predicate), checkFunctionElement.requirePredicate().copy());
				
				++requireIterator;
			}
			
			return predicate;
		}
		
		class PushComputingMethodSet {
		public:
			PushComputingMethodSet(Context& context, const AST::TemplateVar* const templateVar, const AST::Predicate& predicate)
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
		const AST::MethodSet*
		getMethodSetForRequiresPredicate(Context& context, const AST::TemplateVar* const templateVar, const AST::Predicate& requiresPredicate) {
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
				return AST::MethodSet::getEmpty(context.astContext());
			}
			
			PushComputingMethodSet pushComputingMethodSet(context, templateVar, requiresPredicate);
			
			switch (requiresPredicate.kind()) {
				case AST::Predicate::TRUE:
				case AST::Predicate::FALSE:
				case AST::Predicate::VARIABLE:
				{
					return AST::MethodSet::getEmpty(context.astContext());
				}
				case AST::Predicate::AND:
				{
					const auto leftMethodSet = getMethodSetForRequiresPredicate(context, templateVar, requiresPredicate.andLeft());
					const auto rightMethodSet = getMethodSetForRequiresPredicate(context, templateVar, requiresPredicate.andRight());
					return unionMethodSets(leftMethodSet, rightMethodSet);
				}
				case AST::Predicate::OR:
				{
					const auto leftMethodSet = getMethodSetForRequiresPredicate(context, templateVar, requiresPredicate.orLeft());
					const auto rightMethodSet = getMethodSetForRequiresPredicate(context, templateVar, requiresPredicate.orRight());
					return intersectMethodSets(leftMethodSet, rightMethodSet);
				}
				case AST::Predicate::SATISFIES:
				{
					if (requiresPredicate.satisfiesType()->isTemplateVar()) {
						// The case of 'T : SomeRequirement', where T is a template
						// variable. Hence we just get the method set for the required
						// type since T must support that.
						if (requiresPredicate.satisfiesType()->getTemplateVar() == templateVar) {
							return getTypeMethodSet(context, requiresPredicate.satisfiesRequirement());
						} else {
							// It's not the template variable we were looking for, so ignore.
							return AST::MethodSet::getEmpty(context.astContext());
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
			
			locic_unreachable("Unknown predicate kind.");
		}
		
		/**
		 * \brief Add noexcepts to a method set as needed.
		 * 
		 * The outer requires predicate dictates what a method set must
		 * contain, but the outer noexcept predicate can also specify
		 * when methods are noexcept. For example, consider:
		 * 
		 * interface NormalCase {
		 *     void method();
		 * }
		 * 
		 * interface NoexceptCase {
		 *     void method() noexcept;
		 * }
		 * 
		 * template <typename T>
		 * require(T : NormalCase)
		 * void function(T& object) noexcept(T : NoexceptCase) {
		 *     object.method();
		 * }
		 * 
		 * In this case we know 'object' always has the method, but when
		 * the outer noexcept predicate ('T : NoexceptCase') is true the
		 * method is also noexcept. So the correct noexcept predicate
		 * for the method in this context is 'T : NoexceptCase' since
		 * the method is only guaranteed to be noexcept if that
		 * predicate is true.
		 * 
		 * Note that we ignore any methods added by the noexcept
		 * predicate. For example:
		 * 
		 * interface NormalCase {
		 *     void method();
		 * }
		 * 
		 * interface NoexceptCase {
		 *     void method() noexcept;
		 *     void method2() noexcept;
		 * }
		 * 
		 * template <typename T>
		 * require(T : NormalCase)
		 * void function(T& object) noexcept(T : NoexceptCase) {
		 *     object.method();
		 *     object.method2();
		 * }
		 * 
		 * The code is clearly wrong because 'function' can be
		 * instantiated with a type that doesn't have both methods since
		 * it only needs to comply with the requires predicate.
		 */
		const AST::MethodSet*
		getMethodSetWithNoExceptSet(const AST::MethodSet* methodSet,
		                            const AST::MethodSet* noexceptMethodSet,
		                            const AST::Predicate& outerNoexceptPredicate) {
			if (noexceptMethodSet->empty()) {
				// Try to avoid doing unnecessary work in common
				// cases such as noexcept(true|false).
				return methodSet;
			}
			
			auto iterator = methodSet->begin();
			auto noexceptIterator = noexceptMethodSet->begin();
			
			AST::MethodSet::ElementSet elements;
			
			while (iterator != methodSet->end() && noexceptIterator != noexceptMethodSet->end()) {
				const auto& element = iterator->second;
				if (iterator->first == noexceptIterator->first) {
					// TODO: check methods for compatibility!
					
					// Create expression:
					//     method_normal_noexcept OR
					//     (outer_noexcept AND method_noexcept_noexcept)
					//
					// In other words, the method as obtained from
					// the outer requires predicate could be noexcept
					// or the method obtained from the outer noexcept
					// predicate could be noexcept. If we use the
					// latter we also have to require that the
					// outer noexcept predicate is true.
					auto ifNoexceptPredicate = AST::Predicate::And(outerNoexceptPredicate.copy(),
					                                               noexceptIterator->second.noexceptPredicate().copy());
					auto newPredicate = AST::Predicate::Or(element.noexceptPredicate().copy(),
					                                       std::move(ifNoexceptPredicate));
					auto newElement = element.withNoExceptPredicate(std::move(newPredicate));
					elements.push_back(std::make_pair(iterator->first, std::move(newElement)));
					++iterator;
					++noexceptIterator;
				} else if (iterator->first < noexceptIterator->first) {
					elements.push_back(std::make_pair(iterator->first, element.copy()));
					++iterator;
				} else {
					++noexceptIterator;
				}
			}
			
			while (iterator != methodSet->end()) {
				const auto& element = iterator->second;
				elements.push_back(std::make_pair(iterator->first, element.copy()));
				++iterator;
			}
			
			return AST::MethodSet::get(methodSet->context(),
			                           methodSet->constPredicate().copy(),
			                           std::move(elements));
		}
		
		const AST::MethodSet*
		getMethodSetForTemplateVarType(Context& context, const AST::Type* const templateVarType, const AST::TemplatedObject& templatedObject) {
			assert(templateVarType->isTemplateVar());
			
			// Look in the require() predicate to see what methods this
			// template variable supports.
			const auto methodSet = getMethodSetForRequiresPredicate(context, templateVarType->getTemplateVar(), templatedObject.requiresPredicate());
			
			const auto noexceptMethodSet = getMethodSetForRequiresPredicate(context, templateVarType->getTemplateVar(), templatedObject.noexceptPredicate());
			
			// The template variable may be const, in which case add its predicate to
			// the method set's const predicate.
			return getMethodSetWithNoExceptSet(methodSet,
			                                   noexceptMethodSet,
			                                   templatedObject.noexceptPredicate())->withConstPredicate(templateVarType->constPredicate().copy());
		}
		
		namespace {
			
			template <typename PairType>
			bool comparePairKeys(const PairType& a, const PairType& b) {
				return a.first < b.first;
			}
			
		}
		
		const AST::MethodSet*
		getMethodSetForObjectType(Context& context, const AST::Type* const objectType) {
			assert(objectType->isObject());
			
			AST::MethodSet::ElementSet elements;
			
			const auto typeInstance = objectType->getObjectType();
			const auto templateVarMap = objectType->generateTemplateVarMap();
			
			for (const auto& function: typeInstance->functions()) {
				auto constPredicate = function->constPredicate().substitute(templateVarMap);
				auto noexceptPredicate = function->type().attributes().noExceptPredicate().substitute(templateVarMap);
				auto requirePredicate = function->requiresPredicate().substitute(templateVarMap);
				const auto functionType = function->type().substitute(templateVarMap);
				const bool isStatic = function->isStaticMethod();
				
				AST::MethodSetElement functionElement(
					function->templateVariables().copy(),
					std::move(constPredicate),
					std::move(noexceptPredicate),
					std::move(requirePredicate),
					isStatic,
					functionType.returnType(),
					functionType.parameterTypes().copy());
				
				elements.push_back(std::make_pair(function->canonicalName(), std::move(functionElement)));
			}
			
			// Sort the elements.
			std::sort(elements.begin(), elements.end(), comparePairKeys<AST::MethodSet::Element>);
			
			auto constObjectPredicate = objectType->constPredicate().substitute(templateVarMap);
			
			return AST::MethodSet::get(context.astContext(), std::move(constObjectPredicate), std::move(elements));
		}
		
		const AST::MethodSet*
		getTypeMethodSet(Context& context, const AST::Type* const rawType) {
			assert(context.methodSetsComplete());
			
			const auto type = rawType->resolveAliases();
			assert(type->isObject() || type->isTemplateVar());
			
			const auto templatedObject = lookupTemplatedObject(context.scopeStack());
			
			const auto existingMethodSet = context.findMethodSet(templatedObject, type);
			if (existingMethodSet != nullptr) {
				return existingMethodSet;
			}
			
			// In static asserts we won't have a templated object (which would
			// normally be the containing alias, function or type instance), so
			// in this case we do NOT expect to see template variables.
			assert(type->isObject() || templatedObject != nullptr);
			
			const auto methodSet =
				type->isObject() ?
					getMethodSetForObjectType(context, type) :
					getMethodSetForTemplateVarType(context,
					                               type,
					                               *templatedObject);
			
			context.addMethodSet(templatedObject, type, methodSet);
			return methodSet;
		}
		
		const AST::MethodSet*
		intersectMethodSets(const AST::MethodSet* setA, const AST::MethodSet* setB) {
			assert(&(setA->context()) == &(setB->context()));
			
			auto iteratorA = setA->begin();
			auto iteratorB = setB->begin();
			
			AST::MethodSet::ElementSet elements;
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
			
			auto constObjectPredicate = AST::Predicate::Or(setA->constPredicate().copy(), setB->constPredicate().copy());
			return AST::MethodSet::get(setA->context(), std::move(constObjectPredicate), std::move(elements));
		}
		
		const AST::MethodSet*
		unionMethodSets(const AST::MethodSet* setA, const AST::MethodSet* setB) {
			assert(&(setA->context()) == &(setB->context()));
			
			auto iteratorA = setA->begin();
			auto iteratorB = setB->begin();
			
			AST::MethodSet::ElementSet elements;
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
			
			auto constObjectPredicate = AST::Predicate::Or(setA->constPredicate().copy(), setB->constPredicate().copy());
			return AST::MethodSet::get(setA->context(), std::move(constObjectPredicate), std::move(elements));
		}
		
	}
	
}

