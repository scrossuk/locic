#include <stdexcept>

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ConvertVar.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		Debug::VarInfo makeVarInfo(const Debug::VarInfo::Kind kind, const AST::Node<AST::TypeVar>& astTypeVarNode) {
			assert(astTypeVarNode->isNamed());
			
			Debug::VarInfo varInfo;
			varInfo.kind = kind;
			varInfo.name = astTypeVarNode->name();
			varInfo.declLocation = astTypeVarNode.location();
			
			// TODO
			varInfo.scopeLocation = Debug::SourceLocation::Null();
			return varInfo;
		}
		
		namespace {
			
			std::pair<FastMap<String, SEM::Var*>::iterator, bool> insertVar(const ScopeElement& element, const String& name, SEM::Var* var) {
				if (element.isScope()) {
					return element.scope().namedVariables().insert(std::make_pair(name, var));
				} else if (element.isSwitchCase()) {
					return element.switchCase().namedVariables().insert(std::make_pair(name, var));
				} else if (element.isCatchClause()) {
					return element.catchClause().namedVariables().insert(std::make_pair(name, var));
				} else if (element.isFunction()) {
					return element.function().namedVariables().insert(std::make_pair(name, var));
				} else if (element.isTypeInstance()) {
					return element.typeInstance().namedVariables().insert(std::make_pair(name, var));
				} else {
					assert(false && "Invalid element kind for inserting var.");
					throw std::logic_error("Invalid element kind for inserting var.");
				}
			}
			
		}
		
		// Attach the variable to the SemanticAnalysis node tree.
		void attachVar(Context& context, const String& name, const AST::Node<AST::TypeVar>& astTypeVarNode, SEM::Var& var, const Debug::VarInfo::Kind varKind) {
			assert(var.isBasic());
			
			const auto insertResult = insertVar(context.scopeStack().back(), name, &var);
			if (!insertResult.second) {
				const auto existingVar = insertResult.first->second;
				throw ErrorException(makeString("Variable name '%s' at position %s duplicates existing variable of the same name at position %s.",
					name.c_str(), astTypeVarNode.location().toString().c_str(),
					existingVar->debugInfo()->declLocation.toString().c_str()));
			}
			
			var.setDebugInfo(makeVarInfo(varKind, astTypeVarNode));
		}
		
		const SEM::Type* getVarType(Context& context, const AST::Node<AST::TypeVar>& astTypeVarNode, const SEM::Type* initialiseType) {
			switch (astTypeVarNode->kind()) {
				case AST::TypeVar::ANYVAR: {
					return initialiseType;
				}
				
				case AST::TypeVar::NAMEDVAR: {
					return ConvertType(context, astTypeVarNode->namedType())->resolveAliases();
				}
				
				case AST::TypeVar::PATTERNVAR: {
					return ConvertType(context, astTypeVarNode->patternType())->resolveAliases();
				}
			}
			
			std::terminate();
		}
		
		class VariableShadowsExistingVariableDiag: public Error {
		public:
			VariableShadowsExistingVariableDiag(const String& name)
			: name_(name) { }
			
			std::string toString() const {
				return makeString("variable '%s' shadows existing variable",
				                  name_.c_str());
			}
			
		private:
			String name_;
			
		};
		
		class VariableCannotHaveVoidTypeDiag: public Error {
		public:
			VariableCannotHaveVoidTypeDiag(const String& name)
			: name_(name) { }
			
			std::string toString() const {
				return makeString("variable '%s' cannot have void type",
				                  name_.c_str());
			}
			
		private:
			String name_;
			
		};
		
		class CannotPatternMatchNonDatatypeDiag: public Error {
		public:
			CannotPatternMatchNonDatatypeDiag(const SEM::Type* const type)
			: name_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("cannot pattern match for non-datatype '%s'",
				                  name_.c_str());
			}
			
		private:
			std::string name_;
			
		};
		
		class PatternMatchIncorrectVarCountDiag: public Error {
		public:
			PatternMatchIncorrectVarCountDiag(const size_t varCount,
			                                  const SEM::Type* const type,
			                                  const size_t expectedVarCount)
			: varCount_(varCount), name_(type->toDiagString()),
			expectedVarCount_(expectedVarCount) { }
			
			std::string toString() const {
				return makeString("%llu variable(s) in pattern match for type '%s'; expected %llu",
				                  (unsigned long long) varCount_, name_.c_str(),
				                  (unsigned long long) expectedVarCount_);
			}
			
		private:
			size_t varCount_;
			std::string name_;
			size_t expectedVarCount_;
			
		};
		
		namespace {
			
			const SEM::Type* CastType(Context& context, const SEM::Type* sourceType, const SEM::Type* destType, const Debug::SourceLocation& location, bool isTopLevel) {
				// Pattern matched members are restricted
				// to format only casts.
				const bool formatOnly = !isTopLevel;
				
				const auto value = ImplicitCast(context, SEM::Value::CastDummy(sourceType), destType, location, formatOnly);
				return value.type();
			}
			
			std::unique_ptr<SEM::Var> ConvertInitialisedVarRecurse(Context& context,
			                                                       const AST::Node<AST::TypeVar>& astTypeVarNode,
			                                                       const SEM::Type* initialiseType, bool isTopLevel) {
				const auto& location = astTypeVarNode.location();
				
				switch (astTypeVarNode->kind()) {
					case AST::TypeVar::ANYVAR: {
						return SEM::Var::Any(initialiseType);
					}
					
					case AST::TypeVar::NAMEDVAR: {
						const auto& varName = astTypeVarNode->name();
						
						// Search all scopes outside of the current scope.
						const auto searchStartPosition = 1;
						if (performSearch(context, Name::Relative() + varName, searchStartPosition).isVar()) {
							context.issueDiag(VariableShadowsExistingVariableDiag(varName),
							                  location);
						}
						
						const auto varDeclType = ConvertType(context, astTypeVarNode->namedType())->resolveAliases();
						
						// Use cast to resolve any instances of
						// 'auto' in the variable's type.
						const auto varType = CastType(context, initialiseType, varDeclType, location, isTopLevel);
						
						if (varType->isBuiltInVoid()) {
							context.issueDiag(VariableCannotHaveVoidTypeDiag(varName),
							                  location);
						}
						
						// 'final' keyword uses a different lval type (which doesn't support
						// moving or re-assignment).
						const bool isFinalLval = astTypeVarNode->isFinal();
						
						const auto lvalType = makeLvalType(context, isFinalLval, varType);
						
						auto var = SEM::Var::Basic(varType, lvalType);
						var->setMarkedUnused(astTypeVarNode->isUnused());
						var->setOverrideConst(astTypeVarNode->isOverrideConst());
						attachVar(context, varName, astTypeVarNode, *var, Debug::VarInfo::VAR_LOCAL);
						return var;
					}
					
					case AST::TypeVar::PATTERNVAR: {
						const auto varDeclType = ConvertType(context, astTypeVarNode->patternType())->resolveAliases();
						
						if (!varDeclType->isDatatype()) {
							context.issueDiag(CannotPatternMatchNonDatatypeDiag(varDeclType),
							                  astTypeVarNode->patternType().location());
						}
						
						// Use cast to resolve any instances of
						// 'auto' in the variable's type.
						const auto varType = CastType(context, initialiseType, varDeclType, location, isTopLevel);
						
						const auto& astChildTypeVars = astTypeVarNode->typeVarList();
						const auto& typeChildVars = varType->getObjectType()->variables();
						
						if (astChildTypeVars->size() != typeChildVars.size()) {
							context.issueDiag(PatternMatchIncorrectVarCountDiag(astChildTypeVars->size(),
							                                                    varType, typeChildVars.size()),
							                  location);
						}
						
						const auto templateVarMap = varType->generateTemplateVarMap();
						
						std::vector<std::unique_ptr<SEM::Var>> children;
						
						const size_t numUsableVars = std::min(astChildTypeVars->size(), typeChildVars.size());
						for (size_t i = 0; i < numUsableVars; i++) {
							const auto& astVar = astChildTypeVars->at(i);
							const auto& semVar = typeChildVars.at(i);
							
							const auto childInitialiseType = semVar->constructType()->substitute(templateVarMap);
							const bool childIsTopLevel = false;
							children.push_back(ConvertInitialisedVarRecurse(context, astVar, childInitialiseType, childIsTopLevel));
						}
						
						return SEM::Var::Composite(varType, std::move(children));
					}
				}
				
				std::terminate();
			}
			
		}
		
		class AnyVarsNotImplementedForUninitialisedVariablesDiag: public Error {
		public:
			AnyVarsNotImplementedForUninitialisedVariablesDiag() { }
			
			std::string toString() const {
				return "'any' vars not implemented for uninitialised variables";
			}
			
		};
		
		std::unique_ptr<SEM::Var> ConvertVar(Context& context, const Debug::VarInfo::Kind varKind, const AST::Node<AST::TypeVar>& astTypeVarNode) {
			const auto& location = astTypeVarNode.location();
			
			switch (astTypeVarNode->kind()) {
				case AST::TypeVar::ANYVAR: {
					context.issueDiag(AnyVarsNotImplementedForUninitialisedVariablesDiag(),
					                  location);
					return nullptr;
				}
				
				case AST::TypeVar::NAMEDVAR: {
					const auto& varName = astTypeVarNode->name();
					
					// Search all scopes outside of the current scope.
					const auto searchStartPosition = 1;
					if (varKind != Debug::VarInfo::VAR_MEMBER && performSearch(context, Name::Relative() + varName, searchStartPosition).isVar()) {
						context.issueDiag(VariableShadowsExistingVariableDiag(varName),
						                  location);
					}
					
					const auto varType = ConvertType(context, astTypeVarNode->namedType());
					
					// 'final' keyword uses a different lval type (which doesn't support
					// moving or re-assignment).
					const bool isFinalLval = astTypeVarNode->isFinal();
					
					// Variables in catch clauses don't use lvalues.
					const auto lvalType = (varKind != Debug::VarInfo::VAR_EXCEPTION_CATCH) ?
						makeLvalType(context, isFinalLval, varType) : varType;
					
					auto var = SEM::Var::Basic(varType, lvalType);
					var->setMarkedUnused(astTypeVarNode->isUnused());
					var->setOverrideConst(astTypeVarNode->isOverrideConst());
					attachVar(context, varName, astTypeVarNode, *var, varKind);
					return var;
				}
				
				case AST::TypeVar::PATTERNVAR: {
					const auto varType = ConvertType(context, astTypeVarNode->patternType())->resolveAliases();
					
					if (!varType->isDatatype()) {
						context.issueDiag(CannotPatternMatchNonDatatypeDiag(varType),
						                  astTypeVarNode->patternType().location());
					}
					
					const auto& astChildTypeVars = astTypeVarNode->typeVarList();
					const auto& typeChildVars = varType->getObjectType()->variables();
					
					if (astChildTypeVars->size() != typeChildVars.size()) {
						context.issueDiag(PatternMatchIncorrectVarCountDiag(astChildTypeVars->size(),
						                                                    varType, typeChildVars.size()),
						                  location);
					}
					
					const auto templateVarMap = varType->generateTemplateVarMap();
					
					std::vector<std::unique_ptr<SEM::Var>> children;
					
					const size_t numUsableVars = std::min(astChildTypeVars->size(), typeChildVars.size());
					for (size_t i = 0; i < numUsableVars; i++) {
						const auto& astVar = astChildTypeVars->at(i);
						children.push_back(ConvertVar(context, varKind, astVar));
					}
					
					return SEM::Var::Composite(varType, std::move(children));
				}
			}
			
			std::terminate();
		}
		
		std::unique_ptr<SEM::Var> ConvertInitialisedVar(Context& context, const AST::Node<AST::TypeVar>& astTypeVarNode,
		                                                const SEM::Type* const initialiseType) {
			const bool isTopLevel = true;
			return ConvertInitialisedVarRecurse(context, astTypeVarNode, initialiseType, isTopLevel);
		}
		
	}
	
}

