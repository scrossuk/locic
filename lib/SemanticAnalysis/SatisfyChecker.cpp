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
#include <locic/SemanticAnalysis/GetMethodSet.hpp>

#include <locic/Constant.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		constexpr size_t MAX_STACK_DEPTH = 20;
		
		SatisfyChecker::SatisfyChecker(Context& context)
		: context_(context) { }
		
		class StackMaxDepthExceededDiag: public Error {
		public:
			StackMaxDepthExceededDiag(const AST::Type* const sourceType,
			                          const AST::Type* const destType)
			: sourceType_(sourceType), destType_(destType) { }

			std::string toString() const {
				return makeString("max stack depth exceeded evaluating '%s' : '%s'",
				                  sourceType_->toDiagString().c_str(),
				                  destType_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* sourceType_;
			const AST::Type* destType_;
			
		};
		
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
		
		OptionalDiag
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
					return SUCCESS;
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
		
		class CannotMatchIncompatibleTypesDiag: public Error {
		public:
			CannotMatchIncompatibleTypesDiag(const AST::Type* const sourceType,
			                                 const AST::Type* const destType)
			: sourceType_(sourceType), destType_(destType) { }

			std::string toString() const {
				return makeString("cannot match incompatible types '%s' and '%s'",
				                  sourceType_->toDiagString().c_str(),
				                  destType_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* sourceType_;
			const AST::Type* destType_;
			
		};
		
		class ConstNotImpliedDiag: public Error {
		public:
			ConstNotImpliedDiag(const AST::Type* const sourceType,
			                    const AST::Type* const destType)
			: sourceType_(sourceType), destType_(destType) { }

			std::string toString() const {
				if (sourceType_->hasConst() && !destType_->hasConst()) {
					return makeString("const type '%s' cannot satisfy non-const type '%s'",
					                  sourceType_->toDiagString().c_str(),
					                  destType_->toDiagString().c_str());
				} else {
					return makeString("const predicate of '%s' does not imply const predicate of '%s'",
					                  sourceType_->toDiagString().c_str(),
					                  destType_->toDiagString().c_str());
				}
			}
			
		private:
			const AST::Type* sourceType_;
			const AST::Type* destType_;
			
		};
		
		class RefCannotMatchAutoDiag: public Error {
		public:
			RefCannotMatchAutoDiag(const AST::Type* const sourceType,
			                       const AST::Type* const destType)
			: sourceType_(sourceType), destType_(destType) { }

			std::string toString() const {
				return makeString("reference type '%s' cannot satisfy auto type '%s'",
				                  sourceType_->toDiagString().c_str(),
				                  destType_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* sourceType_;
			const AST::Type* destType_;
			
		};
		
		class AutoCannotMatchRefDiag: public Error {
		public:
			AutoCannotMatchRefDiag(const AST::Type* const sourceType,
			                       const AST::Type* const destType)
			: sourceType_(sourceType), destType_(destType) { }

			std::string toString() const {
				return makeString("auto type '%s' cannot satisfy reference type '%s'",
				                  sourceType_->toDiagString().c_str(),
				                  destType_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* sourceType_;
			const AST::Type* destType_;
			
		};
		
		OptionalDiag
		SatisfyChecker::typeSatisfies(const AST::Type* const checkType,
		                              const AST::Type* const requireType) {
			assert(!checkType->isAlias() && !requireType->isAlias());
			
			// Try to unify the types, which will handle things like
			// 'auto 'in template arguments.
			if (unifyTypes(checkType, requireType).success()) {
				return SUCCESS;
			}
			
			const auto constImplied = checkType->constPredicate().implies(requireType->constPredicate());
			
			if ((checkType->hasConst() || requireType->hasConst()) && constImplied) {
				// All types must be covariant in regards to 'const'
				// (i.e. T : const T for all T).
				return satisfies(checkType->stripConst(),
				                 requireType->stripConst());
			}
			
			if (checkType->isAuto() || requireType->isAuto()) {
				if (!constImplied) {
					return ConstNotImpliedDiag(checkType,
					                           requireType);
				}
				
				if (checkType->isRef()) {
					return RefCannotMatchAutoDiag(checkType,
					                              requireType);
				}
				
				if (requireType->isRef()) {
					return AutoCannotMatchRefDiag(checkType,
					                              requireType);
				}
				
				return SUCCESS;
			}
			
			if (!requireType->isInterface()) {
				// Don't allow casts between different object types or
				// template variables.
				
				if (checkType->kind() != requireType->kind()) {
					// Cannot mix object types and template vars.
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
				
				if (!constImplied) {
					// 'const T : T' is always FALSE for non-interface
					// types because we may NOT know the full method
					// set.
					return ConstNotImpliedDiag(checkType, requireType);
				}
			}
			
			const auto sourceMethodSet = getTypeMethodSet(context_, checkType);
			const auto requireMethodSet = getTypeMethodSet(context_, requireType);
			// TODO: chain diagnostics here.
			return methodSetSatisfies(sourceMethodSet, requireMethodSet);
		}
		
		class UnifyIncompatibleTypesDiag: public Error {
		public:
			UnifyIncompatibleTypesDiag(const AST::Type* const sourceType,
			                           const AST::Type* const destType)
			: sourceType_(sourceType), destType_(destType) { }

			std::string toString() const {
				return makeString("cannot unify incompatible types '%s' and '%s'",
				                  sourceType_->toDiagString().c_str(),
				                  destType_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* sourceType_;
			const AST::Type* destType_;
			
		};
		
		class UnifyAutoWithRefDiag: public Error {
		public:
			UnifyAutoWithRefDiag(const AST::Type* const refType)
			: refType_(refType) { }

			std::string toString() const {
				return makeString("cannot unify auto with reference type '%s'",
				                  refType_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* refType_;
			
		};
		
		class UnifyMismatchingTemplateArgDiag: public Error {
		public:
			UnifyMismatchingTemplateArgDiag(const AST::Value& first,
			                                const AST::Value& second)
			: str_(makeString("cannot unify template arguments '%s' and '%s'",
			                  first.toDiagString().c_str(),
			                  second.toDiagString().c_str())) { }

			std::string toString() const {
				return str_;
			}
			
		private:
			std::string str_;
			
		};
		
		class UnifyMismatchingConstPredicates: public Error {
		public:
			UnifyMismatchingConstPredicates(const AST::Predicate& first,
			                                const AST::Predicate& second)
			: str_(makeString("cannot unify const predicates '%s' and '%s'",
			                  first.toString().c_str(),
			                  second.toString().c_str())) { }

			std::string toString() const {
				return str_;
			}
			
		private:
			std::string str_;
			
		};
		
		OptionalDiag
		SatisfyChecker::unifyTypes(const AST::Type* first,
		                           const AST::Type* second) {
			if (first->isAlias() || second->isAlias()) {
				printf("ALIASES: %s or %s\n", first->toDiagString().c_str(),
				       second->toDiagString().c_str());
				first = first->resolveAliases();
				second = second->resolveAliases();
			}
			assert(!first->isAlias() && !second->isAlias());
			
			if (first == second) { return SUCCESS; }
			
			auto diag = unifyConstPredicates(first->constPredicate(),
			                                 second->constPredicate());
			if (diag.failed()) return diag;
			
			if (first->isAuto() || second->isAuto()) {
				const auto otherType = first->isAuto() ? second : first;
				
				if (!otherType->isRef()) {
					// Auto can unify with any non-reference
					// type.
					return SUCCESS;
				} else {
					return UnifyAutoWithRefDiag(otherType);
				}
			}
			
			if (first->kind() != second->kind()) {
				return UnifyIncompatibleTypesDiag(first, second);
			}
			
			if (first->isTemplateVar()) {
				assert(second->isTemplateVar());
				if (first->getTemplateVar() != second->getTemplateVar()) {
					return UnifyIncompatibleTypesDiag(first,
					                                  second);
				}
			}
			
			assert(first->isObject() && second->isObject());
			
			if (first->getObjectType() != second->getObjectType()) {
				return UnifyIncompatibleTypesDiag(first, second);
			}
			
			assert(first->templateArguments().size() == second->templateArguments().size());
			
			return unifyTemplateArgs(first->templateArguments(),
			                         second->templateArguments());
		}
		
		OptionalDiag
		SatisfyChecker::unifyTemplateArgs(const AST::ValueArray& first,
		                                  const AST::ValueArray& second) {
			assert(first.size() == second.size());
			
			for (size_t i = 0; i < first.size(); i++) {
				const auto& firstArg = first[i];
				const auto& secondArg = second[i];
				if (firstArg.isTypeRef() && secondArg.isTypeRef()) {
					auto diag = unifyTypes(firstArg.typeRefType(),
					                       secondArg.typeRefType());
					if (diag.failed()) return diag;
				} else if (firstArg != secondArg) {
					return UnifyMismatchingTemplateArgDiag(firstArg,
					                                       secondArg);
				}
			}
			
			return SUCCESS;
		}
		
		OptionalDiag
		SatisfyChecker::unifyConstPredicates(const AST::Predicate& first,
		                                     const AST::Predicate& second) {
			if (first != second) {
				return UnifyMismatchingConstPredicates(first, second);
			}
			
			return SUCCESS;
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
			CastGenerator castGenerator(context_, *this, sourceType,
			                            /*isNoop=*/true, /*canBind=*/false);
			return castGenerator.implicitCast(destType);
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
			ParentConstPredicateImplicationFailedDiag(const String name, const AST::Predicate& parentPredicate,
			                                          const AST::Predicate& methodPredicate)
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
			ConstPredicateImplicationFailedDiag(const String name, const AST::Predicate& requirePredicate,
			                                    const AST::Predicate& sourcePredicate)
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
			RequirePredicateImplicationFailedDiag(const String name, const AST::Predicate& requirePredicate,
			                                      const AST::Predicate& sourcePredicate)
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
			NoexceptPredicateImplicationFailedDiag(const String name, const AST::Predicate& requirePredicate,
			                                       const AST::Predicate& sourcePredicate)
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
