#include <assert.h>

#include <algorithm>
#include <map>
#include <string>

#include <boost/functional/hash.hpp>

#include <locic/AST/Type.hpp>

#include <locic/Frontend/OptionalDiag.hpp>

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
		
		AST::TemplateVarMap generateSatisfyTemplateVarMap(const MethodSetElement& checkElement, const MethodSetElement& requireElement) {
			AST::TemplateVarMap templateVarMap;
			
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
		
		class MismatchingStaticDiag: public Error {
		public:
			MismatchingStaticDiag(const String name, bool sourceIsStatic,
			                      bool requireIsStatic)
			: name_(name), sourceIsStatic_(sourceIsStatic),
			requireIsStatic_(requireIsStatic) {
				assert(sourceIsStatic != requireIsStatic);
			}
			
			std::string toString() const {
				return makeString("method '%s' is %s in source type but %s in required type",
				                  name_.c_str(), sourceIsStatic_ ? "static" : "non-static",
				                  requireIsStatic_ ? "static" : "non-static");
			}
			
		private:
			String name_;
			bool sourceIsStatic_;
			bool requireIsStatic_;
			
		};
		
		class ParentIsConstMethodIsNotDiag: public Error {
		public:
			ParentIsConstMethodIsNotDiag(const String name)
			: name_(name) { }
			
			std::string toString() const {
				return makeString("mutator method '%s' is required but type is const",
				                  name_.c_str());
			}
			
		private:
			String name_;
			
		};
		
		class ParentConstPredicateImplicationFailedDiag: public Error {
		public:
			ParentConstPredicateImplicationFailedDiag(const String name, const SEM::Predicate& parentPredicate,
			                                          const SEM::Predicate& methodPredicate)
			: name_(name), parentPredicateString_(parentPredicate.toString()),
			methodPredicateString_(methodPredicate.toString()) { }
			
			std::string toString() const {
				return makeString("const predicate '%s' in type doesn't imply const "
				                  "predicate '%s' in method '%s'", parentPredicateString_.c_str(),
				                  methodPredicateString_.c_str(), name_.c_str());
			}
			
		private:
			String name_;
			std::string parentPredicateString_;
			std::string methodPredicateString_;
			
		};
		
		class ConstPredicateImplicationFailedDiag: public Error {
		public:
			ConstPredicateImplicationFailedDiag(const String name, const SEM::Predicate& requirePredicate,
			                                    const SEM::Predicate& sourcePredicate)
			: name_(name), requirePredicateString_(requirePredicate.toString()),
			sourcePredicateString_(sourcePredicate.toString()) { }
			
			std::string toString() const {
				return makeString("method '%s' has const predicate '%s' in required type "
				                  "which doesn't imply const predicate '%s' in source type",
				                  name_.c_str(), requirePredicateString_.c_str(),
				                  sourcePredicateString_.c_str());
			}
			
		private:
			String name_;
			std::string requirePredicateString_;
			std::string sourcePredicateString_;
			
		};
		
		class RequirePredicateImplicationFailedDiag: public Error {
		public:
			RequirePredicateImplicationFailedDiag(const String name, const SEM::Predicate& requirePredicate,
			                                      const SEM::Predicate& sourcePredicate)
			: name_(name), requirePredicateString_(requirePredicate.toString()),
			sourcePredicateString_(sourcePredicate.toString()) { }
			
			std::string toString() const {
				return makeString("method '%s' has require predicate '%s' in required type "
				                  "which doesn't imply require predicate '%s' in source type",
				                  name_.c_str(), requirePredicateString_.c_str(),
				                  sourcePredicateString_.c_str());
			}
			
		private:
			String name_;
			std::string requirePredicateString_;
			std::string sourcePredicateString_;
			
		};
		
		class NoexceptPredicateImplicationFailedDiag: public Error {
		public:
			NoexceptPredicateImplicationFailedDiag(const String name, const SEM::Predicate& requirePredicate,
			                                       const SEM::Predicate& sourcePredicate)
			: name_(name), requirePredicateString_(requirePredicate.toString()),
			sourcePredicateString_(sourcePredicate.toString()) { }
			
			std::string toString() const {
				return makeString("method '%s' has noexcept predicate '%s' in required type "
				                  "which doesn't imply noexcept predicate '%s' in source type",
				                  name_.c_str(), requirePredicateString_.c_str(),
				                  sourcePredicateString_.c_str());
			}
			
		private:
			String name_;
			std::string requirePredicateString_;
			std::string sourcePredicateString_;
			
		};
		
		class ParamCountMismatchDiag: public Error {
		public:
			ParamCountMismatchDiag(const String name, const size_t sourceParamCount,
			                       const size_t requireParamCount)
			: name_(name), sourceParamCount_(sourceParamCount),
			requireParamCount_(requireParamCount) { }
			
			std::string toString() const {
				return makeString("method '%s' has %zu parameter(s) in source type "
				                  "but %zu parameter(s) in required type",
				                  name_.c_str(), sourceParamCount_, requireParamCount_);
			}
			
		private:
			String name_;
			size_t sourceParamCount_;
			size_t requireParamCount_;
			
		};
		
		class ParamTypeMismatchDiag: public Error {
		public:
			ParamTypeMismatchDiag(const String name, const size_t index,
			                      const AST::Type* sourceType, const AST::Type* requireType)
			: name_(name), index_(index), sourceType_(sourceType),
			requireType_(requireType) { }
			
			std::string toString() const {
				return makeString("cannot cast type '%s' to '%s' for parameter %zu in method '%s'",
				                  requireType_->toDiagString().c_str(), sourceType_->toDiagString().c_str(),
				                  index_, name_.c_str());
			}
			
		private:
			String name_;
			size_t index_;
			const AST::Type* sourceType_;
			const AST::Type* requireType_;
			
		};
		
		class ReturnTypeMismatchDiag: public Error {
		public:
			ReturnTypeMismatchDiag(const String name, const AST::Type* sourceType,
			                       const AST::Type* requireType)
			: name_(name), sourceType_(sourceType), requireType_(requireType) { }
			
			std::string toString() const {
				return makeString("return type in method '%s' has type %s in source "
				                  "but type %s in requirement", name_.c_str(),
				                  sourceType_->toDiagString().c_str(),
				                  requireType_->toDiagString().c_str());
			}
			
		private:
			String name_;
			const AST::Type* sourceType_;
			const AST::Type* requireType_;
			
		};
		
		constexpr bool DEBUG_METHOD_SET_ELEMENT = false;
		constexpr bool DEBUG_METHOD_SET = false;
		
		OptionalDiag
		methodSetElementSatisfiesRequirement(Context& context, const SEM::Predicate& checkConstPredicate,
		                                     const String& functionName, const MethodSetElement& checkFunctionElement,
		                                     const MethodSetElement& requireFunctionElement) {
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
				return OptionalDiag(MismatchingStaticDiag(functionName,
				                                          checkFunctionElement.isStatic(),
				                                          requireFunctionElement.isStatic()));
			}
			
			const auto reducedConstPredicate = reducePredicate(context, checkFunctionElement.constPredicate().substitute(satisfyTemplateVarMap));
			
			// The method set's const predicate needs to imply the method's
			// const predicate.
			if (!checkFunctionElement.isStatic() && !checkConstPredicate.implies(reducedConstPredicate)) {
				if (DEBUG_METHOD_SET_ELEMENT) {
					printf("\nConst parent predicate implication failed for '%s'.\n    Parent: %s\n    Method: %s\n\n",
					       functionName.c_str(),
					       checkConstPredicate.toString().c_str(),
					       reducedConstPredicate.toString().c_str()
					);
				}
				if (checkConstPredicate.isTrue() && reducedConstPredicate.isFalse()) {
					return OptionalDiag(ParentIsConstMethodIsNotDiag(functionName));
				}
				
				return OptionalDiag(ParentConstPredicateImplicationFailedDiag(functionName, checkConstPredicate,
				                                                              reducedConstPredicate));
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
				return OptionalDiag(ConstPredicateImplicationFailedDiag(functionName,
				                                                        requireFunctionElement.constPredicate(),
				                                                        reducedConstPredicate));
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
				return OptionalDiag(RequirePredicateImplicationFailedDiag(functionName,
				                                                          requireFunctionElement.requirePredicate(),
				                                                          reducedRequirePredicate));
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
				return OptionalDiag(NoexceptPredicateImplicationFailedDiag(functionName,
				                                                           requireFunctionElement.noexceptPredicate(),
				                                                           reducedNoexceptPredicate));
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
				return OptionalDiag(ParamCountMismatchDiag(functionName, firstList.size(),
				                                           secondList.size()));
			}
			
			for (size_t i = 0; i < firstList.size(); i++) {
				const auto sourceParamType = firstList.at(i)->substitute(satisfyTemplateVarMap);
				const auto requireParamType = secondList.at(i);
				const auto castParamType =
				    ImplicitCastTypeFormatOnly(context, requireParamType, sourceParamType,
				                               Debug::SourceLocation::Null());
				
				if (castParamType == nullptr) {
					if (DEBUG_METHOD_SET_ELEMENT) {
						printf("\nParameter types don't match for '%s' (param %llu).\n    Source: %s\n    Require: %s\n\n",
						       functionName.c_str(),
						       (unsigned long long) i,
						       sourceParamType->toString().c_str(),
						       requireParamType->toString().c_str()
						);
					}
					return OptionalDiag(ParamTypeMismatchDiag(functionName, i, sourceParamType,
					                                          requireParamType));
				}
			}

			const auto sourceReturnType =
			    checkFunctionElement.returnType()->substitute(satisfyTemplateVarMap);
			const auto requireReturnType = requireFunctionElement.returnType();
			const auto castReturnType =
			    ImplicitCastTypeFormatOnly(context, sourceReturnType, requireReturnType,
			                               Debug::SourceLocation::Null());

			if (castReturnType == nullptr) {
				if (DEBUG_METHOD_SET_ELEMENT) {
					printf("\nReturn type doesn't match for '%s'.\n    Source: %s\n    Require: %s\n\n",
					       functionName.c_str(),
					       sourceReturnType->toString().c_str(),
					       requireReturnType->toString().c_str()
					);
				}
				return OptionalDiag(ReturnTypeMismatchDiag(functionName, sourceReturnType,
				                                           requireReturnType));
			}
			
			return OptionalDiag();
		}
		
		class MethodNotFoundDiag: public Error {
		public:
			MethodNotFoundDiag(const String name)
			: name_(name) { }
			
			std::string toString() const {
				return makeString("method '%s' not found",
				                  name_.c_str());
			}
			
		private:
			String name_;
			
		};
		
		OptionalDiag
		methodSetSatisfiesRequirement(Context& context, const MethodSet* const checkSet,
		                              const MethodSet* const requireSet) {
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
					return OptionalDiag(MethodNotFoundDiag(requireFunctionName));
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
				
				auto optionalDiag =
				    methodSetElementSatisfiesRequirement(context, checkConstPredicate,
				                                         checkFunctionName, checkFunctionElement,
				                                         requireFunctionElement);
				if (!optionalDiag) {
					if (DEBUG_METHOD_SET) {
						printf("\n...in methodSetSatisfiesRequirement:\n    Source: %s\n    Require: %s\n\n",
							formatMessage(checkSet->toString()).c_str(),
							formatMessage(requireSet->toString()).c_str());
					}
					return optionalDiag;
				}
				
				++requireIterator;
			}
			
			return OptionalDiag();
		}
		
	}
	
}

