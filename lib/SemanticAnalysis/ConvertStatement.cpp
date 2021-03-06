#include <assert.h>

#include <set>
#include <stdexcept>
#include <string>

#include <locic/AST.hpp>
#include <locic/AST/Type.hpp>
#include <locic/Support/MakeArray.hpp>

#include <locic/SemanticAnalysis/CallValue.hpp>
#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertForLoop.hpp>
#include <locic/SemanticAnalysis/ConvertScope.hpp>
#include <locic/SemanticAnalysis/ConvertStatement.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/ConvertVar.hpp>
#include <locic/SemanticAnalysis/GetMethod.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>
#include <locic/SemanticAnalysis/TypeCapabilities.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		static AST::Value GetAssignValue(Context& context, AST::AssignKind assignKind, AST::Value varValue,
		                                 AST::Value operandValue, const Debug::SourceLocation& location) {
			switch (assignKind) {
				case AST::ASSIGN_DIRECT:
					return operandValue;
				case AST::ASSIGN_ADD: {
					auto opMethod = GetMethod(context, std::move(varValue), context.getCString("add"), location);
					return CallValue(context, std::move(opMethod), makeHeapArray( std::move(operandValue) ), location);
				}
				case AST::ASSIGN_SUB: {
					auto opMethod = GetMethod(context, std::move(varValue), context.getCString("subtract"), location);
					return CallValue(context, std::move(opMethod), makeHeapArray( std::move(operandValue) ), location);
				}
				case AST::ASSIGN_MUL: {
					auto opMethod = GetMethod(context, std::move(varValue), context.getCString("multiply"), location);
					return CallValue(context, std::move(opMethod), makeHeapArray( std::move(operandValue) ), location);
				}
				case AST::ASSIGN_DIV: {
					auto opMethod = GetMethod(context, std::move(varValue), context.getCString("divide"), location);
					return CallValue(context, std::move(opMethod), makeHeapArray( std::move(operandValue) ), location);
				}
				case AST::ASSIGN_MOD: {
					auto opMethod = GetMethod(context, std::move(varValue), context.getCString("modulo"), location);
					return CallValue(context, std::move(opMethod), makeHeapArray( std::move(operandValue) ), location);
				}
			}
			
			locic_unreachable("Unknown assign kind.");
		}
		
		class CannotAssignToNonLvalueDiag: public ErrorDiag {
		public:
			CannotAssignToNonLvalueDiag(const AST::Type* const type)
			: typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("cannot assign to non-lvalue type '%s'",
				                  typeString_.c_str());
			}
			
		private:
			std::string typeString_;
			
		};
		
		class CannotAssignNonMovableDiag: public ErrorDiag {
		public:
			CannotAssignNonMovableDiag(const AST::Type* const type)
			: typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("cannot assign non-movable type '%s'",
				                  typeString_.c_str());
			}
			
		private:
			std::string typeString_;
			
		};
		
		class ScopeActionCanThrowDiag: public ErrorDiag {
		public:
			ScopeActionCanThrowDiag(String scopeActionState)
			: scopeActionState_(std::move(scopeActionState)) { }
			
			std::string toString() const {
				return makeString("scope(%s) can throw",
				                  scopeActionState_.c_str());
			}
			
		private:
			String scopeActionState_;
			
		};
		
		class ThrowNonExceptionValueDiag: public ErrorDiag {
		public:
			ThrowNonExceptionValueDiag(std::string typeString)
			: typeString_(std::move(typeString)) { }
			
			std::string toString() const {
				return makeString("cannot throw non-exception value of type '%s'",
				                  typeString_.c_str());
			}
			
		private:
			std::string typeString_;
			
		};
		
		class RethrowInTryScopeDiag: public ErrorDiag {
		public:
			RethrowInTryScopeDiag() { }
			
			std::string toString() const {
				return "cannot re-throw caught exception inside try scope";
			}
			
		};
		
		class ThrowInAssertNoExceptDiag: public WarningDiag {
		public:
			ThrowInAssertNoExceptDiag() { }
			
			std::string toString() const {
				return "throw statement means assert noexcept is guaranteed to throw";
			}
			
		};
		
		class RethrowInAssertNoExceptDiag: public WarningDiag {
		public:
			RethrowInAssertNoExceptDiag() { }
			
			std::string toString() const {
				return "re-throw statement means assert noexcept is guaranteed to throw";
			}
			
		};
		
		class ThrowInScopeActionDiag: public ErrorDiag {
		public:
			ThrowInScopeActionDiag(String scopeActionState)
			: scopeActionState_(std::move(scopeActionState)) { }
			
			std::string toString() const {
				return makeString("cannot throw exception inside scope(%s)",
				                  scopeActionState_.c_str());
			}
			
		private:
			String scopeActionState_;
			
		};
		
		class RethrowInScopeActionDiag: public ErrorDiag {
		public:
			RethrowInScopeActionDiag(String scopeActionState)
			: scopeActionState_(std::move(scopeActionState)) { }
			
			std::string toString() const {
				return makeString("cannot re-throw caught exception inside scope(%s)",
				                  scopeActionState_.c_str());
			}
			
		private:
			String scopeActionState_;
			
		};
		
		class RethrowOutsideCatchDiag: public ErrorDiag {
		public:
			RethrowOutsideCatchDiag() { }
			
			std::string toString() const {
				return "cannot re-throw exception outside of catch clause";
			}
			
		};
		
		class AssertNoExceptAroundNoexceptScopeDiag: public WarningDiag {
		public:
			AssertNoExceptAroundNoexceptScopeDiag() { }
			
			std::string toString() const {
				return "assert noexcept is around scope that is guaranteed to never throw anyway";
			}
			
		};
		
		class BreakInScopeExitActionDiag: public ErrorDiag {
		public:
			BreakInScopeExitActionDiag(String scopeActionState)
			: scopeActionState_(std::move(scopeActionState)) { }
			
			std::string toString() const {
				return makeString("'break' statement cannot be used in scope(%s)",
				                  scopeActionState_.c_str());
			}
			
		private:
			String scopeActionState_;
			
		};
		
		class BreakNotInCorrectScopeDiag: public ErrorDiag {
		public:
			BreakNotInCorrectScopeDiag() { }
			
			std::string toString() const {
				return "'break' statement not in loop statement";
			}
			
		};
		
		class ContinueInScopeExitActionDiag: public ErrorDiag {
		public:
			ContinueInScopeExitActionDiag(String scopeActionState)
			: scopeActionState_(std::move(scopeActionState)) { }
			
			std::string toString() const {
				return makeString("'continue' statement cannot be used in scope(%s)",
				                  scopeActionState_.c_str());
			}
			
		private:
			String scopeActionState_;
			
		};
		
		class ContinueNotInCorrectScopeDiag: public ErrorDiag {
		public:
			ContinueNotInCorrectScopeDiag() { }
			
			std::string toString() const {
				return "'continue' statement not in loop statement";
			}
			
		};
		
		class DuplicateCaseDiag: public ErrorDiag {
		public:
			DuplicateCaseDiag(const AST::TypeInstance& typeInstance)
			: typeInstance_(typeInstance) { }
			
			std::string toString() const {
				return makeString("duplicate case for type '%s'",
				                  typeInstance_.fullName().toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			const AST::TypeInstance& typeInstance_;
			
		};
		
		class SwitchCaseTypeNotMemberOfDatatype: public ErrorDiag {
		public:
			SwitchCaseTypeNotMemberOfDatatype(const AST::TypeInstance& caseTypeInstance,
			                                  const AST::TypeInstance& switchTypeInstance)
			: caseTypeInstance_(caseTypeInstance),
			switchTypeInstance_(switchTypeInstance) { }
			
			std::string toString() const {
				return makeString("switch type '%s' is not variant of type '%s'",
				                  caseTypeInstance_.fullName().toString(/*addPrefix=*/false).c_str(),
				                  switchTypeInstance_.fullName().toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			const AST::TypeInstance& caseTypeInstance_;
			const AST::TypeInstance& switchTypeInstance_;
			
		};
		
		class SwitchTypeNotObjectDiag: public ErrorDiag {
		public:
			SwitchTypeNotObjectDiag(const AST::Type* type)
			: type_(type) { }
			
			std::string toString() const {
				return makeString("switch type '%s' is not an object",
				                  type_->toDiagString().c_str());
			}
			
		private:
			const AST::Type* type_;
			
		};
		
		constexpr auto MAX_DIAG_LIST_SIZE = 4;
		
		class SwitchCasesNotHandledDiag: public ErrorDiag {
		public:
			SwitchCasesNotHandledDiag(const Array<const AST::Type*, 8>& unhandledCases) {
				assert(!unhandledCases.empty());
				for (size_t i = 0; i < std::min<size_t>(unhandledCases.size(), MAX_DIAG_LIST_SIZE); i++) {
					if (i > 0) casesNotHandled_ += ", ";
					casesNotHandled_ += unhandledCases[i]->toDiagString();
				}
				if (unhandledCases.size() > MAX_DIAG_LIST_SIZE) {
					casesNotHandled_ += ", ...";
				}
			}
			
			std::string toString() const {
				return makeString("cases not handled in switch: %s",
				                  casesNotHandled_.c_str());
			}
			
		private:
			std::string casesNotHandled_;
			
		};
		
		class UnnecessaryDefaultCaseDiag: public WarningDiag {
		public:
			UnnecessaryDefaultCaseDiag() { }
			
			std::string toString() const {
				return "default case in switch which covers all possible cases";
			}
			
		};
		
		class VoidExplicitlyIgnoredDiag: public WarningDiag {
		public:
			VoidExplicitlyIgnoredDiag() { }
			
			std::string toString() const {
				return "void explicitly ignored in expression";
			}
			
		};
		
		class NonVoidNotExplicitlyIgnoredDiag: public WarningDiag {
		public:
			NonVoidNotExplicitlyIgnoredDiag() { }
			
			std::string toString() const {
				return "non-void value result ignored in expression";
			}
			
		};
		
		class TryWrapsScopeThatCannotThrowDiag: public WarningDiag {
		public:
			TryWrapsScopeThatCannotThrowDiag() { }
			
			std::string toString() const {
				return "try statement wraps scope that cannot throw";
			}
			
		};
		
		class CatchClauseCannotUsePatternMatchingDiag: public ErrorDiag {
		public:
			CatchClauseCannotUsePatternMatchingDiag() { }
			
			std::string toString() const {
				return "catch clause cannot use pattern matching";
			}
			
		};
		
		class CannotCatchNonExceptionTypeDiag: public ErrorDiag {
		public:
			CannotCatchNonExceptionTypeDiag(const AST::Type* const type)
			: typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("cannot catch non-exception type '%s'",
				                  typeString_.c_str());
			}
			
		private:
			std::string typeString_;
			
		};
		
		class InvalidScopeExitStateDiag: public ErrorDiag {
		public:
			InvalidScopeExitStateDiag(const String exitState)
			: exitState_(exitState) { }
			
			std::string toString() const {
				return makeString("invalid scope exit state '%s'",
				                  exitState_.c_str());
			}
			
		private:
			String exitState_;
			
		};
		
		class CannotReturnVoidInNonVoidFunctionDiag: public ErrorDiag {
		public:
			CannotReturnVoidInNonVoidFunctionDiag(const Name& functionName)
			: functionNameString_(functionName.toString(/*addPrefix=*/false)) { }
			
			std::string toString() const {
				return makeString("cannot return void in function '%s' with non-void return type",
				                  functionNameString_.c_str());
			}
			
		private:
			std::string functionNameString_;
			
		};
		
		class CannotReturnNonVoidInVoidFunctionDiag: public ErrorDiag {
		public:
			CannotReturnNonVoidInVoidFunctionDiag(const Name& functionName)
			: functionNameString_(functionName.toString(/*addPrefix=*/false)) { }
			
			std::string toString() const {
				return makeString("cannot return non-void value in function '%s' with void return type",
				                  functionNameString_.c_str());
			}
			
		private:
			std::string functionNameString_;
			
		};
		
		class CannotReturnInScopeActionDiag: public ErrorDiag {
		public:
			CannotReturnInScopeActionDiag() { }
			
			std::string toString() const {
				return "cannot return in scope action";
			}
			
		};
		
		static AST::Statement ConvertStatementData(Context& context, const AST::Node<AST::StatementDecl>& statement) {
			const auto& location = statement.location();
			
			switch (statement->kind()) {
				case AST::StatementDecl::VALUE: {
					auto value = ConvertValue(context, statement->value());
					if (statement->isUnusedResultValue()) {
						if (value.type()->isBuiltInVoid()) {
							context.issueDiag(VoidExplicitlyIgnoredDiag(),
							                  location);
						}
						const auto voidType = context.typeBuilder().getVoidType();
						return AST::Statement::ValueStmt(AST::Value::Cast(voidType, std::move(value)));
					} else {
						if (!value.type()->isBuiltInVoid()) {
							context.issueDiag(NonVoidNotExplicitlyIgnoredDiag(),
							                  location);
						}
						return AST::Statement::ValueStmt(std::move(value));
					}
				}
				case AST::StatementDecl::SCOPE: {
					ConvertScope(context, statement->scope());
					return AST::Statement::ScopeStmt(std::move(statement->scope()));
				}
				case AST::StatementDecl::IF: {
					const auto boolType = context.typeBuilder().getBoolType();
					
					std::vector<AST::IfClause*> clauseList;
					for (const auto& ifClauseNode: *(statement->ifClauseList())) {
						auto condition = ConvertValue(context, ifClauseNode->conditionDecl());
						auto boolValue = ImplicitCast(context, std::move(condition), boolType, location);
						ifClauseNode->setCondition(std::move(boolValue));
						
						ConvertScope(context, ifClauseNode->scope());
						
						clauseList.push_back(ifClauseNode.get());
					}
					
					ConvertScope(context, statement->ifElseScope());
					
					return AST::Statement::If(std::move(clauseList), std::move(statement->ifElseScope()));
				}
				case AST::StatementDecl::SWITCH: {
					auto value = ConvertValue(context, statement->switchValue());
					
					const auto switchType = getDerefType(value.type())->resolveAliases()->stripConst();
					
					std::map<const AST::TypeInstance*, const AST::Type*> switchCaseTypes;
					
					std::vector<AST::SwitchCase*> caseList;
					for (const auto& caseNode: *(statement->switchCaseList())) {
						{
							PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::SwitchCase(*caseNode));
							
							(void) ConvertVar(context, Debug::VarInfo::VAR_LOCAL, caseNode->var());
							ConvertScope(context, caseNode->scope());
						}
						
						const auto caseType = caseNode->var()->type();
						
						// Check that all switch cases are based
						// on the same union datatype.
						if (switchType->isObject() &&
						    caseType->getObjectType()->parentTypeInstance() != switchType->getObjectType()) {
							context.issueDiag(SwitchCaseTypeNotMemberOfDatatype(*(caseType->getObjectType()),
							                                                    *(switchType->getObjectType())),
							                  caseNode->var().location());
						}
						
						const auto insertResult = switchCaseTypes.insert(std::make_pair(caseType->getObjectType(), caseType));
						
						// Check for duplicate cases.
						if (!insertResult.second) {
							context.issueDiag(DuplicateCaseDiag(*(caseType->getObjectType())),
							                  caseNode.location());
						}
						
						caseList.push_back(caseNode.get());
					}
					
					const auto& defaultCaseNode = statement->defaultCase();
					
					if (switchType->isObject()) {
						// Check whether all cases are handled.
						const auto switchTypeInstance = switchType->getObjectType();
						assert(switchTypeInstance != nullptr);
						
						Array<const AST::Type*, 8> unhandledCases;
						for (const auto variantType: switchTypeInstance->variantTypes()) {
							const auto variantTypeInstance = variantType->getObjectType();
							if (switchCaseTypes.find(variantTypeInstance) == switchCaseTypes.end()) {
								unhandledCases.push_back(variantType);
							}
						}
						
						if (defaultCaseNode->hasScope()) {
							if (unhandledCases.empty()) {
								context.issueDiag(UnnecessaryDefaultCaseDiag(),
								                  defaultCaseNode.location());
							}
						} else {
							if (!unhandledCases.empty()) {
								context.issueDiag(SwitchCasesNotHandledDiag(unhandledCases),
								                  location);
							}
						}
					} else {
						context.issueDiag(SwitchTypeNotObjectDiag(switchType),
						                  statement->switchValue().location());
					}
					
					// Cast value to switch type.
					auto castValue = ImplicitCast(context, std::move(value), switchType,
					                              statement->switchValue().location());
					
					if (defaultCaseNode->hasScope()) {
						ConvertScope(context, defaultCaseNode->scope());
					}
					
					return AST::Statement::Switch(std::move(castValue), caseList, *defaultCaseNode);
				}
				case AST::StatementDecl::WHILE: {
					auto condition = ConvertValue(context, statement->whileCondition());
					
					PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Loop());
					
					ConvertScope(context, statement->whileScope());
					auto advanceScope = AST::Scope::Create(statement.location());
					auto loopCondition = ImplicitCast(context, std::move(condition), context.typeBuilder().getBoolType(), location);
					return AST::Statement::Loop(std::move(loopCondition), std::move(statement->whileScope()), std::move(advanceScope));
				}
				case AST::StatementDecl::FOR: {
					auto loopScope = ConvertForLoop(context, statement->forVar(),
					                                statement->forInitValue(),
					                                statement->forInitScope());
					return AST::Statement::ScopeStmt(std::move(loopScope));
				}
				case AST::StatementDecl::TRY: {
					AST::Node<AST::Scope> tryScope;
					
					{
						PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::TryScope());
						ConvertScope(context, statement->tryScope());
						tryScope = std::move(statement->tryScope());
						
						const auto exitStates = tryScope->exitStates();
						if (!exitStates.hasAnyThrowingStates()) {
							context.issueDiag(TryWrapsScopeThatCannotThrowDiag(),
							                  location);
						}
					}
					
					std::vector<AST::CatchClause*> catchList;
					
					for (auto& catchNode: *(statement->tryCatchList())) {
						PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::CatchClause(*catchNode));
						
						if (!catchNode->var()->isNamed()) {
							context.issueDiag(CatchClauseCannotUsePatternMatchingDiag(),
							                  catchNode->var().location());
						}
						
						auto var = ConvertVar(context, Debug::VarInfo::VAR_EXCEPTION_CATCH, catchNode->var());
						assert(var == catchNode->var().get());
						(void) var;
						
						const auto varType = catchNode->var()->type();
						if (!varType->isException()) {
							context.issueDiag(CannotCatchNonExceptionTypeDiag(varType),
							                  catchNode->var().location());
						}
						
						ConvertScope(context, catchNode->scope());
						
						catchList.push_back(catchNode.get());
					}
					
					return AST::Statement::Try(std::move(tryScope), catchList);
				}
				case AST::StatementDecl::SCOPEEXIT: {
					auto scopeExitState = statement->scopeExitState();
					if (scopeExitState != "exit" && scopeExitState != "success" && scopeExitState != "failure") {
						context.issueDiag(InvalidScopeExitStateDiag(scopeExitState),
						                  location);
						scopeExitState = context.getCString("exit");
					}
					
					PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::ScopeAction(scopeExitState));
					
					ConvertScope(context, statement->scopeExitScope());
					const auto exitStates = statement->scopeExitScope()->exitStates();
					
					// scope(success) is allowed to throw.
					if (scopeExitState != "success" && exitStates.hasThrowExit()) {
						// TODO: remove this; each potentially throwing site should check for this.
						context.issueDiag(ScopeActionCanThrowDiag(scopeExitState),
						                  location);
					}
					
					return AST::Statement::ScopeExit(scopeExitState,
					                                 std::move(statement->scopeExitScope()));
				}
				case AST::StatementDecl::VARDECL: {
					auto& astVarNode = statement->varDeclVar();
					const auto& astInitialValueNode = statement->varDeclValue();
					
					auto astValue = ConvertValue(context, astInitialValueNode);
					
					const auto varDeclType = getVarType(context, astVarNode, astValue.type());
					
					// Cast the initialise value to the variable's type.
					auto astInitialiseValue = ImplicitCast(context, std::move(astValue),
					                                       varDeclType->stripConst(), location);
					
					// Convert the AST type var.
					const auto varType = astInitialiseValue.type();
					auto var = ConvertInitialisedVar(context, astVarNode, varType);
					assert(!var->isAny());
					
					// Add the variable to the scope.
					auto& astScope = context.scopeStack().back().scope();
					
					astScope.variables().push_back(var);
					
					// Generate the initialise statement.
					return AST::Statement::InitialiseStmt(*var, std::move(astInitialiseValue));
				}
				case AST::StatementDecl::ASSIGN: {
					const auto assignKind = statement->assignKind();
					auto astLvalue = derefValue(ConvertValue(context, statement->assignLvalue()));
					auto astRvalue = ConvertValue(context, statement->assignRvalue());
					
					const AST::Type* castType;
					
					if (astLvalue.type()->isRef()) {
						castType = astLvalue.type()->refTarget();
					} else {
						castType = astLvalue.type();
						context.issueDiag(CannotAssignToNonLvalueDiag(astLvalue.type()),
						                  statement->assignLvalue().location());
					}
					
					if (!TypeCapabilities(context).supportsMove(castType)) {
						context.issueDiag(CannotAssignNonMovableDiag(castType),
						                  location);
					}
					
					assert(!castType->isRef());
					
					// TODO: fix this to not copy the value!
					auto astAssignValue = GetAssignValue(context, assignKind, astLvalue.copy(), std::move(astRvalue), location);
					
					auto castAssignvalue = ImplicitCast(context, std::move(astAssignValue), castType->stripConst(),
					                                    statement->assignRvalue().location());
					
					return AST::Statement::AssignStmt(std::move(astLvalue), std::move(castAssignvalue));
				}
				case AST::StatementDecl::INCREMENT: {
					auto astOperandValue = ConvertValue(context, statement->incrementValue());
					auto opMethod = GetMethod(context, std::move(astOperandValue), context.getCString("increment"), location);
					auto opResult = CallValue(context, std::move(opMethod), { }, location);
					
					if (opResult.type()->isBuiltInVoid()) {
						return AST::Statement::ValueStmt(std::move(opResult));
					} else {
						// Automatically cast to void if necessary.
						const auto voidType = context.typeBuilder().getVoidType();
						auto voidCastedValue = AST::Value::Cast(voidType, std::move(opResult));
						return AST::Statement::ValueStmt(std::move(voidCastedValue));
					}
				}
				case AST::StatementDecl::DECREMENT: {
					auto astOperandValue = ConvertValue(context, statement->decrementValue());
					auto opMethod = GetMethod(context, std::move(astOperandValue), context.getCString("decrement"), location);
					auto opResult = CallValue(context, std::move(opMethod), { }, location);
					
					if (opResult.type()->isBuiltInVoid()) {
						return AST::Statement::ValueStmt(std::move(opResult));
					} else {
						// Automatically cast to void if necessary.
						const auto voidType = context.typeBuilder().getVoidType();
						auto voidCastedValue = AST::Value::Cast(voidType, std::move(opResult));
						return AST::Statement::ValueStmt(std::move(voidCastedValue));
					}
				}
				case AST::StatementDecl::RETURNVOID: {
					// Void return statement (i.e. return;)
					if (!getParentFunctionReturnType(context.scopeStack())->isBuiltInVoid()) {
						const auto& functionName = lookupParentFunction(context.scopeStack())->fullName();
						context.issueDiag(CannotReturnVoidInNonVoidFunctionDiag(functionName),
						                  location);
					}
					
					return AST::Statement::ReturnVoid();
				}
				case AST::StatementDecl::RETURN: {
					assert(statement->returnValue().get() != nullptr);
					
					// Check this is not being used inside a scope action.
					for (size_t i = 0; i < context.scopeStack().size(); i++) {
						const auto pos = context.scopeStack().size() - i - 1;
						const auto& element = context.scopeStack()[pos];
						if (element.isScopeAction()) {
							context.issueDiag(CannotReturnInScopeActionDiag(),
							                  location);
							break;
						}
					}
					
					auto astValue = ConvertValue(context, statement->returnValue());
					
					const bool functionIsVoid = getParentFunctionReturnType(context.scopeStack())->isBuiltInVoid();
					const bool valueIsVoid = astValue.type()->isBuiltInVoid();
					
					if (functionIsVoid && !valueIsVoid) {
						// Can't return in a function that returns void.
						const auto& name = lookupParentFunction(context.scopeStack())->fullName();
						context.issueDiag(CannotReturnNonVoidInVoidFunctionDiag(name),
						                  location);
						return AST::Statement::Return(std::move(astValue));
					}
					
					if (!functionIsVoid && valueIsVoid) {
						// Can't return void.
						const auto& name = lookupParentFunction(context.scopeStack())->fullName();
						context.issueDiag(CannotReturnVoidInNonVoidFunctionDiag(name),
						                  location);
						return AST::Statement::Return(std::move(astValue));
					}
					
					// Cast the return value to the function's
					// specified return type.
					auto castValue = ImplicitCast(context, std::move(astValue), getParentFunctionReturnType(context.scopeStack()), location);
					return AST::Statement::Return(std::move(castValue));
				}
				case AST::StatementDecl::THROW: {
					// Check this is not being used inside a scope action
					// (apart from inside scope(success), which is allowed).
					for (size_t i = 0; i < context.scopeStack().size(); i++) {
						const auto pos = context.scopeStack().size() - i - 1;
						const auto& element = context.scopeStack()[pos];
						if (element.isScopeAction() && element.scopeActionState() != "success") {
							context.issueDiag(ThrowInScopeActionDiag(element.scopeActionState()),
							                  location);
							break;
						} else if (element.isAssertNoExcept()) {
							context.issueDiag(ThrowInAssertNoExceptDiag(),
							                  location);
							break;
						}
					}
					
					auto astValue = ConvertValue(context, statement->throwValue());
					if (!astValue.type()->isObject() || !astValue.type()->getObjectType()->isException()) {
						context.issueDiag(ThrowNonExceptionValueDiag(astValue.type()->toString()),
						                  location);
					}
					return AST::Statement::Throw(std::move(astValue));
				}
				case AST::StatementDecl::RETHROW: {
					// Check this is being used inside a catch clause, and
					// is not inside a try clause or a scope action.
					bool foundCatchClause = false;
					bool foundAssertNoExcept = false;
					
					for (size_t i = 0; i < context.scopeStack().size(); i++) {
						const auto pos = context.scopeStack().size() - i - 1;
						const auto& element = context.scopeStack()[pos];
						
						if (element.isCatchClause()) {
							foundCatchClause = true;
						}
						
						if (foundAssertNoExcept) {
							continue;
						}
						
						if (element.isTryScope() && !foundCatchClause) {
							context.issueDiag(RethrowInTryScopeDiag(),
							                  location);
						} else if (element.isScopeAction() && element.scopeActionState() != "success") {
							context.issueDiag(RethrowInScopeActionDiag(element.scopeActionState()),
							                  location);
						} else if (element.isAssertNoExcept()) {
							foundAssertNoExcept = true;
							context.issueDiag(RethrowInAssertNoExceptDiag(),
							                  location);
						}
					}
					
					if (!foundCatchClause) {
						context.issueDiag(RethrowOutsideCatchDiag(),
						                  location);
					}
					
					return AST::Statement::Rethrow();
				}
				case AST::StatementDecl::BREAK: {
					// Check this is being used inside a loop, and
					// would not leave a scope-exit action.
					bool foundLoop = false;
					for (size_t i = 0; i < context.scopeStack().size(); i++) {
						const auto pos = context.scopeStack().size() - i - 1;
						const auto& element = context.scopeStack()[pos];
						if (element.isLoop()) {
							foundLoop = true;
							break;
						} else if (element.isScopeAction()) {
							context.issueDiag(BreakInScopeExitActionDiag(element.scopeActionState()),
							                  location);
						}
					}
					
					if (!foundLoop) {
						context.issueDiag(BreakNotInCorrectScopeDiag(),
						                  location);
					}
					
					return AST::Statement::Break();
				}
				case AST::StatementDecl::CONTINUE: {
					// Check this is being used inside a loop, and
					// would not leave a scope-exit action.
					bool foundLoop = false;
					for (size_t i = 0; i < context.scopeStack().size(); i++) {
						const auto pos = context.scopeStack().size() - i - 1;
						const auto& element = context.scopeStack()[pos];
						if (element.isLoop()) {
							foundLoop = true;
							break;
						} else if (element.isScopeAction()) {
							context.issueDiag(ContinueInScopeExitActionDiag(element.scopeActionState()),
							                  location);
						}
					}
					
					if (!foundLoop) {
						context.issueDiag(ContinueNotInCorrectScopeDiag(),
						                  location);
					}
					
					return AST::Statement::Continue();
				}
				case AST::StatementDecl::ASSERT: {
					assert(statement->assertValue().get() != nullptr);
					
					const auto boolType = context.typeBuilder().getBoolType();
					auto condition = ConvertValue(context, statement->assertValue());
					auto boolValue = ImplicitCast(context, std::move(condition), boolType, location);
					return AST::Statement::Assert(std::move(boolValue), statement->assertName());
				}
				case AST::StatementDecl::ASSERTNOEXCEPT: {
					PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::AssertNoExcept());
					
					ConvertScope(context, statement->assertNoExceptScope());
					const auto scopeExitStates = statement->assertNoExceptScope()->exitStates();
					if (!scopeExitStates.hasThrowExit() && !scopeExitStates.hasRethrowExit()) {
						context.issueDiag(AssertNoExceptAroundNoexceptScopeDiag(),
						                  location);
					}
					
					return AST::Statement::AssertNoExcept(std::move(statement->assertNoExceptScope()));
				}
				case AST::StatementDecl::UNREACHABLE: {
					return AST::Statement::Unreachable();
				}
			}
			
			locic_unreachable("Unknown AST::StatementDecl.");
		}
		
		static Debug::StatementInfo makeStatementInfo(const AST::Node<AST::StatementDecl>& astStatementNode) {
			Debug::StatementInfo statementInfo;
			statementInfo.location = astStatementNode.location();
			return statementInfo;
		}
		
		AST::Statement ConvertStatement(Context& context, const AST::Node<AST::StatementDecl>& astStatementNode) {
			auto astStatement = ConvertStatementData(context, astStatementNode);
			astStatement.setDebugInfo(makeStatementInfo(astStatementNode));
			return astStatement;
		}
		
	}
	
}


