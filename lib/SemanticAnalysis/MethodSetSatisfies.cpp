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
#include <locic/SemanticAnalysis/MethodSetSatisfies.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
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

