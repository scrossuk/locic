#include <locic/SemanticAnalysis/SatisfyChecker.hpp>

#include <locic/AST/MethodSet.hpp>
#include <locic/AST/MethodSetElement.hpp>
#include <locic/AST/Predicate.hpp>
#include <locic/AST/TemplateVar.hpp>
#include <locic/AST/TemplateVarMap.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/ValueDecl.hpp>

#include <locic/Frontend/OptionalDiag.hpp>

#include <locic/SemanticAnalysis/CastGenerator.hpp>
#include <locic/SemanticAnalysis/CastSequence.hpp>
#include <locic/SemanticAnalysis/GetMethodSet.hpp>
#include <locic/SemanticAnalysis/Unifier.hpp>

#include <locic/Constant.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		constexpr size_t MAX_STACK_DEPTH = 20;
		
		SatisfyChecker::SatisfyChecker(Context& context, Unifier& unifier)
		: context_(context), unifier_(unifier) { }
		
		Diag
		StackMaxDepthExceededDiag(const AST::Type* const sourceType,
		                          const AST::Type* const destType) {
			return Error("max stack depth exceeded evaluating '%s' : '%s'",
			             sourceType->toDiagString().c_str(),
			             destType->toDiagString().c_str());
		}
		
		class PushStack {
		public:
			PushStack(SatisfyChecker::Stack& stack,
			          const AST::Type* const checkType,
			          const AST::Type* const requireType)
			: stack_(stack) {
				stack_.push_back(std::make_pair(checkType, requireType));
			}
			
			~PushStack() {
				stack_.pop_back();
			}
			
		private:
			SatisfyChecker::Stack& stack_;
			
		};
		
		ResultOrDiag<const AST::Type*>
		SatisfyChecker::satisfies(const AST::Type* checkType,
		                          const AST::Type* requireType) {
			checkType = checkType->resolveAliases();
			requireType = requireType->resolveAliases();
			
			// Avoid cycles such as:
			// 
			// template <typename T : SomeType<T>>
			// interface SomeType { }
			// 
			// This is done by first checking if T : SomeType<T>,
			// which itself will check that T : SomeType<T> (since T
			// is an argument to 'SomeType') but on the second (nested)
			// check simply assuming that the result is true (since
			// this is being used to compute whether it is itself true
			// and a cyclic dependency like this is acceptable).
			const auto pair = std::make_pair(checkType, requireType);
			for (const auto& checkPair: satisfyCheckStack_) {
				if (pair == checkPair) {
					return requireType;
				}
			}
			
			if (satisfyCheckStack_.size() > MAX_STACK_DEPTH) {
				printf("Depth exceeded!\n");
				for (const auto& checkPair: satisfyCheckStack_) {
					printf("    %s : %s\n",
					       checkPair.first->toDiagString().c_str(),
					       checkPair.second->toDiagString().c_str());
				}
				return StackMaxDepthExceededDiag(checkType,
				                                 requireType);
			}
			
			PushStack stackElement(satisfyCheckStack_, checkType, requireType);
			return typeSatisfies(checkType, requireType);
		}
		
		Diag
		CannotMatchIncompatibleTypesDiag(const AST::Type* const sourceType,
		                                 const AST::Type* const destType) {
			return Error("cannot match incompatible types '%s' and '%s'",
			             sourceType->toDiagString().c_str(),
			             destType->toDiagString().c_str());
		}
		
		Diag
		ConstNotImpliedDiag(const AST::Type* const sourceType,
		                    const AST::Type* const destType) {
			if (sourceType->hasConst() && !destType->hasConst()) {
				return Error("const type '%s' cannot satisfy non-const type '%s'",
				             sourceType->toDiagString().c_str(),
				             destType->toDiagString().c_str());
			} else {
				return Error("const predicate of '%s' does not imply const predicate of '%s'",
				             sourceType->toDiagString().c_str(),
				             destType->toDiagString().c_str());
			}
		}
		
		Diag
		RefCannotMatchAutoDiag(const AST::Type* const sourceType,
		                       const AST::Type* const destType) {
			return Error("reference type '%s' cannot satisfy auto type '%s'",
			             sourceType->toDiagString().c_str(),
			             destType->toDiagString().c_str());
		}
		
		Diag
		AutoCannotMatchRefDiag(const AST::Type* const sourceType,
		                       const AST::Type* const destType) {
			return Error("auto type '%s' cannot satisfy reference type '%s'",
			             sourceType->toDiagString().c_str(),
			             destType->toDiagString().c_str());
		}
		
		ResultOrDiag<const AST::Type*>
		SatisfyChecker::typeSatisfies(const AST::Type* const checkType,
		                              const AST::Type* const requireType) {
			assert(!checkType->isAlias() && !requireType->isAlias());
			
			if (requireType->isAuto()) {
				if (checkType->isRef()) {
					return RefCannotMatchAutoDiag(checkType,
					                              requireType);
				}
				
				// Note that 'const T : auto' is always TRUE because
				// we resolve 'auto' to be 'const T' itself.
				//
				// Note also that 'const auto : auto' is always
				// TRUE because the two auto types are completely
				// distinct, so the second 'auto' is resolved to
				// being const(first auto).
				return checkType->applyConst(requireType->constPredicate().copy());
			}
			
			if (checkType->isAuto()) {
				if (requireType->isRef()) {
					return AutoCannotMatchRefDiag(checkType,
					                              requireType);
				}
				
				// If we have 'const auto : T' we resolve 'auto' to
				// 'T' and so we have to check 'const T : T'
				// (which can be true for interface types).
				return satisfies(requireType->applyConst(checkType->constPredicate().copy()),
				                 requireType);
			}
			
			if (requireType->isInterface()) {
				// For 'T : Interface' we want to check the method sets, since
				// we know we'll have a complete method set for the interface
				// type.
				//
				// Note that 'const T : Interface' can be true if the interface
				// only contains methods marked 'const'.
				const auto sourceMethodSet = getTypeMethodSet(context_, checkType);
				const auto requireMethodSet = getTypeMethodSet(context_, requireType);
				
				auto result = methodSetSatisfies(sourceMethodSet, requireMethodSet);
				// TODO: chain diagnostics here.
				if (result.failed()) return result.extractDiag();
				
				return requireType;
			}
			
			if (checkType->kind() != requireType->kind()) {
				return CannotMatchIncompatibleTypesDiag(checkType,
				                                        requireType);
			}
			
			if (requireType->isObject()) {
				if (checkType->getObjectType() != requireType->getObjectType()) {
					return CannotMatchIncompatibleTypesDiag(checkType,
					                                        requireType);
				}
			} else {
				assert(requireType->isTemplateVar());
				if (checkType->getTemplateVar() != requireType->getTemplateVar()) {
					return CannotMatchIncompatibleTypesDiag(checkType,
					                                        requireType);
				}
			}
			
			if (!checkType->constPredicate().implies(requireType->constPredicate())) {
				// 'const T : T' is always FALSE for non-interface
				// types because, in general, we don't necessarily know
				// the complete method set. So 'T' could contain a
				// non-const method that we can't see.
				return ConstNotImpliedDiag(checkType, requireType);
			}
			
			
			auto result = unifier_.unifyTypes(checkType->stripConst(),
			                                  requireType->stripConst());
			if (result.failed()) {
				return result;
			}
			
			return result.value()->applyConst(requireType->constPredicate().copy());
		}
		
		OptionalDiag
		SatisfyChecker::methodSetElementTypeCast(const AST::Type* const sourceType,
		                                         const AST::Type* const destType) {
			if (sourceType->hasConst() || destType->hasConst()) {
				// TODO: This shouldn't be needed.
				return methodSetElementTypeCast(sourceType->stripConst(),
				                                destType->stripConst());
			}
			
			// All casts in method set elements (return type or argument type)
			// must be NOOP, since there's no ability for Code Generation to
			// insert any instructions.
			CastGenerator castGenerator(context_, *this);
			return castGenerator.implicitCastNoop(sourceType, destType);
		}
		
		AST::TemplateVarMap
		SatisfyChecker::generateSatisfyTemplateVarMap(const AST::MethodSetElement& checkElement,
		                                              const AST::MethodSetElement& requireElement) {
			AST::TemplateVarMap templateVarMap;
			
			// Very basic template deduction.
			for (const auto& templateVar: checkElement.templateVariables()) {
				auto selfRefValue = templateVar->selfRefValue();
				
				if (checkElement.constPredicate().isVariable() && checkElement.constPredicate().variableTemplateVar() == templateVar) {
					if (requireElement.constPredicate().isTrivialBool()) {
						const bool chosenValue = requireElement.constPredicate().isTrue() ? true : false;
						const auto chosenConstant = chosenValue ? Constant::True() : Constant::False();
						auto value = AST::Value::Constant(chosenConstant, selfRefValue.type());
						templateVarMap.insert(std::make_pair(templateVar, std::move(value)));
					} else if (requireElement.constPredicate().isVariable()) {
						auto value = AST::Value::PredicateExpr(AST::Predicate::False(), selfRefValue.type());
						templateVarMap.insert(std::make_pair(templateVar, std::move(value)));
					} else {
						auto value = AST::Value::PredicateExpr(requireElement.constPredicate().copy(), selfRefValue.type());
						templateVarMap.insert(std::make_pair(templateVar, std::move(value)));
					}
				} else {
					templateVarMap.insert(std::make_pair(templateVar, std::move(selfRefValue)));
				}
			}
			
			return templateVarMap;
		}
		
		Diag
		MismatchingStaticDiag(const String name, bool sourceIsStatic,
		                      bool requireIsStatic) {
			assert(sourceIsStatic != requireIsStatic);
			return Error("method '%s' is %s in source type but %s in required type",
			             name.c_str(), sourceIsStatic ? "static" : "non-static",
			             requireIsStatic ? "static" : "non-static");
		}
		
		Diag
		ParentIsConstMethodIsNotDiag(const String name) {
			return Error("mutator method '%s' is required but type is const",
			             name.c_str());
		}
		
		Diag
		ParentConstPredicateImplicationFailedDiag(const String name, const AST::Predicate& parentPredicate,
		                                          const AST::Predicate& methodPredicate) {
			return Error("const predicate '%s' in type doesn't imply const "
			             "predicate '%s' in method '%s'", parentPredicate.toString().c_str(),
			             methodPredicate.toString().c_str(), name.c_str());
		}
		
		Diag
		ConstPredicateImplicationFailedDiag(const String name, const AST::Predicate& requirePredicate,
		                                    const AST::Predicate& sourcePredicate) {
			return Error("method '%s' has const predicate '%s' in required type "
			             "which doesn't imply const predicate '%s' in source type",
			             name.c_str(), requirePredicate.toString().c_str(),
			             sourcePredicate.toString().c_str());
		}
		
		Diag
		RequirePredicateImplicationFailedDiag(const String name, const AST::Predicate& requirePredicate,
		                                      const AST::Predicate& sourcePredicate) {
			return Error("method '%s' has require predicate '%s' in required type "
			             "which doesn't imply require predicate '%s' in source type",
			             name.c_str(), requirePredicate.toString().c_str(),
			             sourcePredicate.toString().c_str());
		}
		
		Diag
		NoexceptPredicateImplicationFailedDiag(const String name, const AST::Predicate& requirePredicate,
		                                       const AST::Predicate& sourcePredicate) {
			return Error("method '%s' has noexcept predicate '%s' in required type "
			             "which doesn't imply noexcept predicate '%s' in source type",
			             name.c_str(), requirePredicate.toString().c_str(),
			             sourcePredicate.toString().c_str());
		}
		
		Diag
		ParamCountMismatchDiag(const String name, const size_t sourceParamCount,
		                       const size_t requireParamCount) {
			return Error("method '%s' has %zu parameter(s) in source type "
			             "but %zu parameter(s) in required type",
			             name.c_str(), sourceParamCount, requireParamCount);
		}
		
		Diag
		ParamTypeMismatchDiag(const String name, const size_t index,
		                      const AST::Type* const sourceType,
		                      const AST::Type* const requireType) {
			return Error("cannot cast type '%s' to '%s' for parameter %zu in method '%s'",
			             requireType->toDiagString().c_str(), sourceType->toDiagString().c_str(),
			             index, name.c_str());
		}
		
		Diag
		ReturnTypeMismatchDiag(const String name, const AST::Type* const sourceType,
		                       const AST::Type* const requireType) {
			return Error("return type in method '%s' has type %s in source "
			             "but type %s in requirement", name.c_str(),
			             sourceType->toDiagString().c_str(),
			             requireType->toDiagString().c_str());
		}
		
		constexpr bool DEBUG_METHOD_SET_ELEMENT = false;
		constexpr bool DEBUG_METHOD_SET = false;
		
		OptionalDiag
		SatisfyChecker::methodSatisfies(const AST::Predicate& checkSelfConst,
		                                const AST::Predicate& requireSelfConst,
		                                const String& functionName,
		                                const AST::MethodSetElement& checkFunctionElement,
		                                const AST::MethodSetElement& requireFunctionElement) {
			if (!requireFunctionElement.templateVariables().empty()) {
				// FIXME: We shouldn't have to skip over these.
				return SUCCESS;
			}
			
			const auto requireConst = reducePredicate(requireFunctionElement.constPredicate().substitute(AST::TemplateVarMap(),
			                                                                                             /*selfconst=*/requireSelfConst));
			
			if (!requireFunctionElement.isStatic() && !requireSelfConst.implies(requireConst)) {
				// Skip because required method is non-const
				// inside const parent.
				return SUCCESS;
			}
			
			const auto satisfyTemplateVarMap = generateSatisfyTemplateVarMap(checkFunctionElement, requireFunctionElement);
			
			// Can't cast between static/non-static methods.
			if (checkFunctionElement.isStatic() != requireFunctionElement.isStatic()) {
				if (DEBUG_METHOD_SET_ELEMENT) {
					printf("\nStatic-ness doesn't match for '%s'.\n    Source: %s\n    Require: %s\n\n",
					       functionName.c_str(),
					       checkFunctionElement.isStatic() ? "static" : "not static",
					       requireFunctionElement.isStatic() ? "static" : "not static"
					);
				}
				return MismatchingStaticDiag(functionName,
				                             checkFunctionElement.isStatic(),
				                             requireFunctionElement.isStatic());
			}
			
			const auto checkConst = reducePredicate(checkFunctionElement.constPredicate().substitute(satisfyTemplateVarMap,
			                                                                                         /*selfconst=*/checkSelfConst));
			
			// The method set's const predicate needs to imply the method's
			// const predicate.
			if (!checkFunctionElement.isStatic() && !checkSelfConst.implies(checkConst)) {
				if (DEBUG_METHOD_SET_ELEMENT) {
					printf("\nConst parent predicate implication failed for '%s'.\n    Parent: %s\n    Method: %s\n\n",
					       functionName.c_str(),
					       checkSelfConst.toString().c_str(),
					       checkConst.toString().c_str()
					);
				}
				if (checkSelfConst.isTrue() && checkConst.isFalse()) {
					return ParentIsConstMethodIsNotDiag(functionName);
				}
				
				return ParentConstPredicateImplicationFailedDiag(functionName, checkSelfConst,
				                                                 checkSelfConst);
			}
			
			// The requirement method's const predicate needs to imply the
			// const predicate of the provided method (e.g. if the requirement
			// method is const, then the provided method must also be, but not
			// vice versa).
			if (!requireConst.implies(checkConst)) {
				if (DEBUG_METHOD_SET_ELEMENT) {
					printf("\nConst predicate implication failed for '%s'.\n    Source: %s\n    Require: %s\n\n",
					       functionName.c_str(),
					       checkConst.toString().c_str(),
					       requireConst.toString().c_str()
					);
				}
				return ConstPredicateImplicationFailedDiag(functionName,
				                                           requireConst,
				                                           checkConst);
			}
			
			const auto checkRequire = reducePredicate(checkFunctionElement.requirePredicate().substitute(satisfyTemplateVarMap,
			                                                                                             /*selfconst=*/checkSelfConst));
			const auto requireRequire = reducePredicate(requireFunctionElement.requirePredicate().substitute(AST::TemplateVarMap(),
			                                                                                                 /*selfconst=*/requireSelfConst));
			
			// The requirement method's require predicate needs to imply the
			// require predicate of the provided method.
			if (!requireRequire.implies(checkRequire)) {
				if (DEBUG_METHOD_SET_ELEMENT) {
					printf("\nRequire predicate implication failed for '%s'.\n    Source: %s\n    Require: %s\n\n",
					       functionName.c_str(),
					       checkRequire.toString().c_str(),
					       requireRequire.toString().c_str()
					);
				}
				return RequirePredicateImplicationFailedDiag(functionName,
				                                             requireRequire,
				                                             checkRequire);
			}
			
			const auto checkNoexcept = reducePredicate(checkFunctionElement.noexceptPredicate().substitute(satisfyTemplateVarMap,
			                                                                                               /*selfconst=*/checkSelfConst));
			const auto requireNoexcept = reducePredicate(requireFunctionElement.noexceptPredicate().substitute(AST::TemplateVarMap(),
			                                                                                                   /*selfconst=*/requireSelfConst));
			
			// Can't cast throwing method to noexcept method.
			if (!requireNoexcept.implies(checkNoexcept)) {
				if (DEBUG_METHOD_SET_ELEMENT) {
					printf("\nNoexcept predicate implication failed for '%s'.\n    Source: %s\n    Require: %s\n\n",
					       functionName.c_str(),
					       checkNoexcept.toString().c_str(),
					       requireNoexcept.toString().c_str()
					);
				}
				return NoexceptPredicateImplicationFailedDiag(functionName,
				                                              requireNoexcept,
				                                              checkNoexcept);
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
				return ParamCountMismatchDiag(functionName, firstList.size(),
				                              secondList.size());
			}
			
			for (size_t i = 0; i < firstList.size(); i++) {
				const auto checkParamType = firstList.at(i)->substitute(satisfyTemplateVarMap,
				                                                         /*selfconst=*/checkSelfConst);
				const auto requireParamType = secondList.at(i)->substitute(AST::TemplateVarMap(),
				                                                           /*selfconst=*/requireSelfConst);
				auto result = methodSetElementTypeCast(requireParamType,
				                                       checkParamType);
				if (result.failed()) {
					if (DEBUG_METHOD_SET_ELEMENT) {
						printf("\nParameter types don't match for '%s' (param %llu).\n    Source: %s\n    Require: %s\n\n",
						       functionName.c_str(),
						       (unsigned long long) i,
						       checkParamType->toString().c_str(),
						       requireParamType->toString().c_str()
						);
					}
					return ParamTypeMismatchDiag(functionName, i, checkParamType,
					                             requireParamType);
				}
			}
			
			{
				const auto checkReturnType =
				    checkFunctionElement.returnType()->substitute(satisfyTemplateVarMap,
				                                                  /*selfconst=*/checkSelfConst);
				const auto requireReturnType =
				    requireFunctionElement.returnType()->substitute(AST::TemplateVarMap(),
				                                                    /*selfconst=*/requireSelfConst);
				
				auto result = methodSetElementTypeCast(checkReturnType,
				                                       requireReturnType);
				if (result.failed()) {
					if (DEBUG_METHOD_SET_ELEMENT) {
						printf("\nReturn type doesn't match for '%s'.\n    Source: %s\n    Require: %s\n\n",
						       functionName.c_str(),
						       checkReturnType->toString().c_str(),
						       requireReturnType->toString().c_str()
						);
					}
					return ReturnTypeMismatchDiag(functionName, checkReturnType,
					                              requireReturnType);
				}
			}
			
			return SUCCESS;
		}
		
		Diag
		MethodNotFoundDiag(const String name) {
			return Error("method '%s' not found", name.c_str());
		}
		
		OptionalDiag
		SatisfyChecker::methodSetSatisfies(const AST::MethodSet* const checkSet,
		                                   const AST::MethodSet* const requireSet) {
			auto checkIterator = checkSet->begin();
			auto requireIterator = requireSet->begin();
			
			const auto checkSelfConst = reducePredicate(checkSet->constPredicate().copy());
			const auto requireSelfConst = reducePredicate(requireSet->constPredicate().copy());
			
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
					return MethodNotFoundDiag(requireFunctionName);
				}
				
				const auto& checkFunctionName = checkIterator->first;
				const auto& checkFunctionElement = checkIterator->second;
				
				if (checkFunctionName != requireFunctionName) {
					continue;
				}
				
				auto result = methodSatisfies(checkSelfConst,
				                              requireSelfConst,
				                              checkFunctionName, checkFunctionElement,
				                              requireFunctionElement);
				if (result.failed()) {
					if (DEBUG_METHOD_SET) {
						printf("\n...in methodSetSatisfiesRequirement:\n    Source: %s\n    Require: %s\n\n",
							formatMessage(checkSet->toString()).c_str(),
							formatMessage(requireSet->toString()).c_str());
					}
					return result;
				}
				
				++requireIterator;
			}
			
			return SUCCESS;
		}
		
		AST::Predicate
		SatisfyChecker::reducePredicate(AST::Predicate predicate) {
			switch (predicate.kind()) {
				case AST::Predicate::TRUE:
				case AST::Predicate::FALSE:
				case AST::Predicate::SELFCONST:
				{
					return predicate;
				}
				case AST::Predicate::AND:
				{
					auto left = reducePredicate(predicate.andLeft().copy());
					auto right = reducePredicate(predicate.andRight().copy());
					return AST::Predicate::And(std::move(left), std::move(right));
				}
				case AST::Predicate::OR:
				{
					auto left = reducePredicate(predicate.orLeft().copy());
					auto right = reducePredicate(predicate.orRight().copy());
					return AST::Predicate::Or(std::move(left), std::move(right));
				}
				case AST::Predicate::SATISFIES:
				{
					const auto checkType = predicate.satisfiesType();
					const auto requireType = predicate.satisfiesRequirement();
					
					if (checkType->isAuto()) {
						// Presumably this is OK...
						// TODO: remove auto from here.
						return AST::Predicate::True();
					}
					
					const auto result = satisfies(checkType, requireType);
					if (result.success()) {
						// If the result is true then we
						// know for sure that the check
						// type satisfies the requirement,
						// but a false result might just
						// be a lack of information.
						return AST::Predicate::True();
					}
					
					if (!checkType->dependsOnOnly({}) || !requireType->dependsOnOnly({})) {
						// Types still depend on some template variables, so can't reduce.
						return predicate;
					}
					
					return AST::Predicate::False();
				}
				case AST::Predicate::VARIABLE:
				{
					return predicate;
				}
			}
			
			locic_unreachable("Unknown predicate kind.");
		}
		
	}
	
}
