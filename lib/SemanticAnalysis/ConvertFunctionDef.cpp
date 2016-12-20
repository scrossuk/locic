#include <cassert>
#include <cstdio>
#include <list>
#include <locic/AST.hpp>
#include <locic/AST/Type.hpp>
#include <locic/Debug/Module.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertFunctionDef.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertScope.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeCapabilities.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		void DeadCodeSearchScope(Context& context, const AST::Scope& scope);
		
		static void DeadCodeSearchStatement(Context& context, const SEM::Statement& statement) {
			switch(statement.kind()) {
				case SEM::Statement::VALUE:
				case SEM::Statement::INITIALISE:
				case SEM::Statement::RETURN:
				case SEM::Statement::RETURNVOID:
				case SEM::Statement::THROW:
				case SEM::Statement::RETHROW:
				case SEM::Statement::BREAK:
				case SEM::Statement::CONTINUE:
				case SEM::Statement::ASSERT:
				case SEM::Statement::UNREACHABLE: {
					return;
				}
				case SEM::Statement::SCOPE: {
					DeadCodeSearchScope(context, statement.getScope());
					return;
				}
				case SEM::Statement::IF: {
					for (const auto ifClause: statement.getIfClauseList()) {
						DeadCodeSearchScope(context, *(ifClause->scope()));
					}
					
					DeadCodeSearchScope(context, statement.getIfElseScope());
					return;
				}
				case SEM::Statement::SWITCH: {
					for (auto switchCase: statement.getSwitchCaseList()) {
						DeadCodeSearchScope(context, switchCase->scope());
					}
					return;
				}
				case SEM::Statement::LOOP: {
					DeadCodeSearchScope(context, statement.getLoopIterationScope());
					DeadCodeSearchScope(context, statement.getLoopAdvanceScope());
					return;
				}
				case SEM::Statement::FOR: {
					DeadCodeSearchScope(context, statement.getForScope());
					return;
				}
				case SEM::Statement::TRY: {
					DeadCodeSearchScope(context, statement.getTryScope());
					for (auto catchClause: statement.getTryCatchList()) {
						DeadCodeSearchScope(context, *(catchClause->scope()));
					}
					return;
				}
				case SEM::Statement::SCOPEEXIT: {
					DeadCodeSearchScope(context, statement.getScopeExitScope());
					return;
				}
				case SEM::Statement::ASSERTNOEXCEPT: {
					DeadCodeSearchScope(context, statement.getAssertNoExceptScope());
					return;
				}
			}
			
			locic_unreachable("Unknown statement kind.");
		}
		
		class CodeNeverExecutedDiag: public Warning {
		public:
			CodeNeverExecutedDiag() { }
			
			std::string toString() const {
				return "code will never be executed";
			}
			
		};
		
		class ScopeExitNeverExecutedDiag: public Warning {
		public:
			ScopeExitNeverExecutedDiag() { }
			
			std::string toString() const {
				return "scope(exit) will never be executed";
			}
			
		};
		
		class ScopeFailureNeverExecutedDiag: public Warning {
		public:
			ScopeFailureNeverExecutedDiag() { }
			
			std::string toString() const {
				return "scope(failure) will never be executed";
			}
			
		};
		
		class ScopeSuccessNeverExecutedDiag: public Warning {
		public:
			ScopeSuccessNeverExecutedDiag() { }
			
			std::string toString() const {
				return "scope(success) will never be executed";
			}
			
		};
		
		void DeadCodeSearchScope(Context& context, const AST::Scope& scope) {
			const SEM::Statement* lastScopeExit = nullptr;
			const SEM::Statement* lastScopeFailure = nullptr;
			const SEM::Statement* lastScopeSuccess = nullptr;
			
			bool isNormalBlocked = false;
			for (const auto& statement: scope.statements()) {
				DeadCodeSearchStatement(context, statement);
				
				if (isNormalBlocked) {
					const auto debugInfo = statement.debugInfo();
					assert(debugInfo);
					context.issueDiag(CodeNeverExecutedDiag(),
					                  debugInfo->location);
					continue;
				}
				
				if (statement.isScopeExitStatement()) {
					if (statement.getScopeExitState() == "exit") {
						lastScopeExit = &statement;
					} else if (statement.getScopeExitState() == "failure") {
						lastScopeFailure = &statement;
						
						// Any previous scope(exit) will
						// be reached after this
						// scope(failure) has executed.
						lastScopeExit = nullptr;
					} else if (statement.getScopeExitState() == "success") {
						const auto scopeExitStates = statement.getScopeExitScope().exitStates();
						
						if (!scopeExitStates.hasNormalExit()) {
							// Any previous scope(failure) will
							// be reached after this
							// scope(success) has executed
							// since we're guaranteeed to
							// be throwing.
							lastScopeFailure = nullptr;
						}
						
						if (lastScopeSuccess != nullptr &&
						    !scopeExitStates.hasNormalExit()) {
							context.issueDiag(ScopeSuccessNeverExecutedDiag(),
							                  lastScopeSuccess->debugInfo()->location);
						}
						
						lastScopeSuccess = &statement;
						
						// Any previous scope(exit) will
						// be reached after this
						// scope(success) has executed.
						lastScopeExit = nullptr;
					} else {
						locic_unreachable("Unknown scope exit state.");
					}
				}
				
				const auto exitStates = statement.exitStates();
				if (!exitStates.hasNormalExit()) {
					isNormalBlocked = true;
				}
				
				if (exitStates.hasAnyStates(AST::ExitStates::AllExceptNormal())) {
					// TODO: only the most recent
					// scope(exit) will be reached!
					
					// Scope exits will be reached by
					// anything other than continuing to the
					// next statement, such as an exception
					// being thrown.
					lastScopeExit = nullptr;
					
					if (exitStates.hasAnyNonThrowingStates()) {
						lastScopeSuccess = nullptr;
					}
					
					if (exitStates.hasAnyThrowingStates()) {
						lastScopeFailure = nullptr;
					}
				}
			}
			
			if (isNormalBlocked && lastScopeExit != nullptr) {
				context.issueDiag(ScopeExitNeverExecutedDiag(),
				                  lastScopeExit->debugInfo()->location);
			}
			
			if (lastScopeFailure != nullptr) {
				context.issueDiag(ScopeFailureNeverExecutedDiag(),
				                  lastScopeFailure->debugInfo()->location);
			}
			
			if (isNormalBlocked && lastScopeSuccess != nullptr) {
				context.issueDiag(ScopeSuccessNeverExecutedDiag(),
				                  lastScopeSuccess->debugInfo()->location);
			}
		}
		
		class NoExceptFunctionThrowsDiag: public Error {
		public:
			NoExceptFunctionThrowsDiag(std::string functionName)
			: functionName_(std::move(functionName)) { }
			
			std::string toString() const {
				return makeString("function '%s' is declared as 'noexcept' but can throw",
				                  functionName_.c_str());
			}
			
		private:
			std::string functionName_;
			
		};
		
		class UnusedParameterVarDiag: public Warning {
		public:
			UnusedParameterVarDiag(String varName)
			: varName_(varName) { }
			
			std::string toString() const {
				return makeString("unused parameter '%s' (which is not marked as 'unused')",
				                  varName_.c_str());
			}
			
		private:
			String varName_;
			
		};
		
		class UsedParameterVarMarkedUnusedDiag: public Warning {
		public:
			UsedParameterVarMarkedUnusedDiag(String varName)
			: varName_(varName) { }
			
			std::string toString() const {
				return makeString("parameter '%s' is marked 'unused' but is used",
				                  varName_.c_str());
			}
			
		private:
			String varName_;
			
		};
		
		class ReturnTypeNotMovableDiag: public Error {
		public:
			ReturnTypeNotMovableDiag(const AST::Type* const type,
			                         const Name& functionName)
			: typeString_(type->toDiagString()),
			functionNameString_(functionName.toString(/*addPrefix=*/false)) { }
			
			std::string toString() const {
				return makeString("return type '%s' of function '%s' is not movable",
				                  typeString_.c_str(), functionNameString_.c_str());
			}
			
		private:
			std::string typeString_;
			std::string functionNameString_;
			
		};
		
		class MissingReturnStatementDiag: public Error {
		public:
			MissingReturnStatementDiag(const Name& functionName)
			: functionNameString_(functionName.toString(/*addPrefix=*/false)) { }
			
			std::string toString() const {
				return makeString("control reaches end of function '%s' with non-void return type; it needs a return statement",
				                  functionNameString_.c_str());
			}
			
		private:
			std::string functionNameString_;
			
		};
		
		class NoExceptPredicateIsntImpliedDiag: public Error {
		public:
			NoExceptPredicateIsntImpliedDiag(const Name& functionName,
			                                 const SEM::Predicate& noexceptPredicate,
			                                 const SEM::Predicate& declaredNoexceptPredicate)
			: functionNameString_(functionName.toString(/*addPrefix=*/false)),
			noexceptPredicateString_(noexceptPredicate.toString()),
			declaredNoexceptPredicateString_(declaredNoexceptPredicate.toString()) { }
			
			std::string toString() const {
				return makeString("function '%s' has noexcept predicate '%s' which "
				                  "isn't implied by explicitly declared noexcept "
				                  "predicate '%s'", functionNameString_.c_str(),
				                  noexceptPredicateString_.c_str(),
				                  declaredNoexceptPredicateString_.c_str());
			}
			
		private:
			std::string functionNameString_;
			std::string noexceptPredicateString_;
			std::string declaredNoexceptPredicateString_;
			
		};
		
		void ConvertFunctionDef(Context& context, const AST::Node<AST::Function>& function) {
			const auto functionType = function->type();
			
			if (!TypeCapabilities(context).supportsMove(functionType.returnType())) {
				context.issueDiag(ReturnTypeNotMovableDiag(functionType.returnType(),
				                                           function->fullName()),
				                  function->returnType().location());
			}
			
			if (function->isAutoGenerated()) {
				// Has a default definition.
				DefaultMethods(context).createDefaultMethod(lookupParentType(context.scopeStack()),
				                                            *function, function.location());
				return;
			}
			
			if (!function->hasScope()) {
				// Only a declaration.
				return;
			}
			
			assert(function->scope()->statements().empty());
			
			// Generate the outer function scope.
			// (which will then generate its contents etc.)
			ConvertScope(context, function->scope());
			
			const auto returnType = functionType.returnType();
			const auto exitStates = function->scope()->exitStates();
			
			if (exitStates.hasNormalExit()) {
				if (!returnType->isBuiltInVoid()) {
					// Functions with non-void return types must return a value.
					context.issueDiag(MissingReturnStatementDiag(function->fullName()),
					                  function->scope().location());
				} else {
					// Need to add a void return statement if the program didn't.
					function->scope()->statements().push_back(SEM::Statement::ReturnVoid());
				}
			}
			
			DeadCodeSearchScope(context, *(function->scope()));
			
			const auto actualNoexceptPredicate = reducePredicate(context, exitStates.noexceptPredicate().copy());
			const auto declaredNoexceptPredicate = reducePredicate(context, functionType.attributes().noExceptPredicate().copy());
			
			if (!declaredNoexceptPredicate.implies(actualNoexceptPredicate)) {
				// TODO: These diagnostics should appear where we actually throw.
				const auto location = function->noexceptSpecifier().location().range().isNull() ?
				    function.location() : function->noexceptSpecifier().location();
				if (declaredNoexceptPredicate.isTrue() && actualNoexceptPredicate.isFalse()) {
					context.issueDiag(NoExceptFunctionThrowsDiag(function->fullName().toString()),
					                  location);
				} else {
					context.issueDiag(NoExceptPredicateIsntImpliedDiag(function->fullName(),
					                                                   exitStates.noexceptPredicate(),
					                                                   functionType.attributes().noExceptPredicate()),
					                  location);
				}
			}
			
			// Check all variables are either used and not marked unused,
			// or are unused and marked as such.
			for (const auto& varPair: function->namedVariables()) {
				const auto& varName = varPair.first;
				const auto& var = varPair.second;
				if (var->isUsed() && var->isMarkedUnused()) {
					const auto& debugInfo = var->debugInfo();
					assert(debugInfo);
					const auto& location = debugInfo->declLocation;
					context.issueDiag(UsedParameterVarMarkedUnusedDiag(varName),
					                  location);
				} else if (!var->isUsed() && !var->isMarkedUnused()) {
					const auto& debugInfo = var->debugInfo();
					assert(debugInfo);
					const auto& location = debugInfo->declLocation;
					context.issueDiag(UnusedParameterVarDiag(varName),
					                  location);
				}
			}
			
		}
		
	}
	
}

