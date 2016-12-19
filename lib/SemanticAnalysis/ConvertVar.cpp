#include <stdexcept>

#include <locic/AST.hpp>
#include <locic/AST/Type.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertVar.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>
#include <locic/SemanticAnalysis/TypeResolver.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		Debug::VarInfo makeVarInfo(const Debug::VarInfo::Kind kind, const AST::Node<AST::Var>& astVarNode) {
			assert(astVarNode->isNamed());
			
			Debug::VarInfo varInfo;
			varInfo.kind = kind;
			varInfo.name = astVarNode->name();
			varInfo.declLocation = astVarNode.location();
			
			// TODO
			varInfo.scopeLocation = Debug::SourceLocation::Null();
			return varInfo;
		}
		
		namespace {
			
			std::pair<FastMap<String, AST::Var*>::iterator, bool> insertVar(const ScopeElement& element, const String& name, AST::Var* var) {
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
					locic_unreachable("Invalid element kind for inserting var.");
				}
			}
			
		}
		
		class VariableDuplicatesExistingVariableDiag: public Error {
		public:
			VariableDuplicatesExistingVariableDiag(const String& name)
			: name_(name) { }
			
			std::string toString() const {
				return makeString("variable '%s' duplicates existing variable",
				                  name_.c_str());
			}
			
		private:
			String name_;
			
		};
		
		class PreviousVariableDiag: public Error {
		public:
			PreviousVariableDiag() { }
			
			std::string toString() const {
				return "variable previously defined here";
			}
			
		};
		
		// Attach the variable to the SemanticAnalysis node tree.
		void attachVar(Context& context, AST::Node<AST::Var>& varNode, const Debug::VarInfo::Kind varKind) {
			assert(varNode->isNamed());
			
			const auto insertResult = insertVar(context.scopeStack().back(),
			                                    varNode->name(), varNode.get());
			if (!insertResult.second) {
				const auto existingVar = insertResult.first->second;
				OptionalDiag previousVarDiag(PreviousVariableDiag(),
				                             existingVar->debugInfo()->declLocation);
				context.issueDiag(VariableDuplicatesExistingVariableDiag(varNode->name()),
				                  varNode.location(), std::move(previousVarDiag));
			}
			
			varNode->setDebugInfo(makeVarInfo(varKind, varNode));
		}
		
		const AST::Type* getVarType(Context& context, const AST::Node<AST::Var>& astVarNode, const AST::Type* /*initialiseType*/) {
			return TypeResolver(context).resolveType(astVarNode->declType())->resolveAliases();
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
			CannotPatternMatchNonDatatypeDiag(const AST::Type* const type)
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
			                                  const AST::Type* const type,
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
			
			const AST::Type* CastType(Context& context, const AST::Type* sourceType, const AST::Type* destType, const Debug::SourceLocation& location, bool isTopLevel) {
				// Pattern matched members are restricted
				// to format only casts.
				const bool formatOnly = !isTopLevel;
				
				const auto value = ImplicitCast(context, AST::Value::CastDummy(sourceType), destType, location, formatOnly);
				return value.type();
			}
			
			AST::Var*
			ConvertInitialisedVarRecurse(Context& context,
			                             AST::Node<AST::Var>& astVarNode,
			                             const AST::Type* initialiseType,
			                             bool isTopLevel) {
				const auto& location = astVarNode.location();
				
				switch (astVarNode->kind()) {
					case AST::Var::ANYVAR: {
						astVarNode->setConstructType(initialiseType);
						return astVarNode.get();
					}
					
					case AST::Var::NAMEDVAR: {
						const auto& varName = astVarNode->name();
						
						// Search all scopes outside of the current scope.
						const auto searchStartPosition = 1;
						const auto searchResult = performSearch(context, Name::Relative() + varName,
						                                        searchStartPosition);
						if (searchResult.isVar()) {
							OptionalDiag previousVarDiag(PreviousVariableDiag(),
							                             searchResult.var().debugInfo()->declLocation);
							context.issueDiag(VariableShadowsExistingVariableDiag(varName),
							                  location, std::move(previousVarDiag));
						}
						
						const auto varDeclType = TypeResolver(context).resolveType(astVarNode->declType())->resolveAliases();
						
						// Use cast to resolve any instances of
						// 'auto' in the variable's type.
						const auto varType = CastType(context, initialiseType, varDeclType, location, isTopLevel);
						
						if (varType->isBuiltInVoid()) {
							context.issueDiag(VariableCannotHaveVoidTypeDiag(varName),
							                  location);
						}
						
						astVarNode->setConstructType(varType);
						
						// 'final' keyword uses a different lval type (which doesn't support
						// moving or re-assignment).
						const bool isFinalLval = astVarNode->isFinal();
						
						const auto lvalType = makeLvalType(context, isFinalLval, varType);
						
						astVarNode->setLvalType(lvalType);
						
						attachVar(context, astVarNode, Debug::VarInfo::VAR_LOCAL);
						
						return astVarNode.get();
					}
					
					case AST::Var::PATTERNVAR: {
						const auto varDeclType = TypeResolver(context).resolveType(astVarNode->declType())->resolveAliases();
						
						if (!varDeclType->isDatatype()) {
							context.issueDiag(CannotPatternMatchNonDatatypeDiag(varDeclType),
							                  astVarNode->declType().location());
						}
						
						// Use cast to resolve any instances of
						// 'auto' in the variable's type.
						const auto varType = CastType(context, initialiseType, varDeclType, location, isTopLevel);
						
						astVarNode->setConstructType(varType);
						
						const auto& astChildVars = astVarNode->varList();
						const auto& typeChildVars = varType->getObjectType()->variables();
						
						if (astChildVars->size() != typeChildVars.size()) {
							context.issueDiag(PatternMatchIncorrectVarCountDiag(astChildVars->size(),
							                                                    varType, typeChildVars.size()),
							                  location);
						}
						
						const auto templateVarMap = varType->generateTemplateVarMap();
						
						const size_t numUsableVars = std::min(astChildVars->size(), typeChildVars.size());
						for (size_t i = 0; i < numUsableVars; i++) {
							auto& astVar = astChildVars->at(i);
							const auto& semVar = typeChildVars.at(i);
							
							const auto childInitialiseType = semVar->constructType()->substitute(templateVarMap);
							const bool childIsTopLevel = false;
							(void) ConvertInitialisedVarRecurse(context, astVar, childInitialiseType, childIsTopLevel);
						}
						
						return astVarNode.get();
					}
				}
				
				locic_unreachable("Unknown var kind.");
			}
			
		}
		
		class AnyVarsNotImplementedForUninitialisedVariablesDiag: public Error {
		public:
			AnyVarsNotImplementedForUninitialisedVariablesDiag() { }
			
			std::string toString() const {
				return "'any' vars not implemented for uninitialised variables";
			}
			
		};
		
		AST::Var*
		ConvertVar(Context& context, const Debug::VarInfo::Kind varKind,
		           AST::Node<AST::Var>& astVarNode) {
			const auto& location = astVarNode.location();
			
			switch (astVarNode->kind()) {
				case AST::Var::ANYVAR: {
					context.issueDiag(AnyVarsNotImplementedForUninitialisedVariablesDiag(),
					                  location);
					return astVarNode.get();
				}
				
				case AST::Var::NAMEDVAR: {
					const auto& varName = astVarNode->name();
					
					// Search all scopes outside of the current scope.
					const auto searchStartPosition = 1;
					if (varKind != Debug::VarInfo::VAR_MEMBER) {
						const auto searchResult = performSearch(context, Name::Relative() + varName,
						                                        searchStartPosition);
						if (searchResult.isVar()) {
							OptionalDiag previousVarDiag(PreviousVariableDiag(),
							                             searchResult.var().debugInfo()->declLocation);
							context.issueDiag(VariableShadowsExistingVariableDiag(varName),
							                  location, std::move(previousVarDiag));
						}
					}
					
					const auto varType = TypeResolver(context).resolveType(astVarNode->declType());
					astVarNode->setConstructType(varType);
					
					// 'final' keyword uses a different lval type (which doesn't support
					// moving or re-assignment).
					const bool isFinalLval = astVarNode->isFinal();
					
					// Variables in catch clauses don't use lvalues.
					const auto lvalType = (varKind != Debug::VarInfo::VAR_EXCEPTION_CATCH) ?
						makeLvalType(context, isFinalLval, varType) : varType;
					
					astVarNode->setLvalType(lvalType);
					
					attachVar(context, astVarNode, varKind);
					
					return astVarNode.get();
				}
				
				case AST::Var::PATTERNVAR: {
					const auto varType = TypeResolver(context).resolveType(astVarNode->declType())->resolveAliases();
					
					if (!varType->isDatatype()) {
						context.issueDiag(CannotPatternMatchNonDatatypeDiag(varType),
						                  astVarNode->declType().location());
					}
					
					astVarNode->setConstructType(varType);
					
					const auto& astChildVars = astVarNode->varList();
					const auto& typeChildVars = varType->getObjectType()->variables();
					
					if (astChildVars->size() != typeChildVars.size()) {
						context.issueDiag(PatternMatchIncorrectVarCountDiag(astChildVars->size(),
						                                                    varType, typeChildVars.size()),
						                  location);
					}
					
					const auto templateVarMap = varType->generateTemplateVarMap();
					
					const size_t numUsableVars = std::min(astChildVars->size(), typeChildVars.size());
					for (size_t i = 0; i < numUsableVars; i++) {
						auto& astVar = astChildVars->at(i);
						(void) ConvertVar(context, varKind, astVar);
					}
					
					return astVarNode.get();
				}
			}
			
			locic_unreachable("Unknown var kind.");
		}
		
		AST::Var* ConvertInitialisedVar(Context& context, AST::Node<AST::Var>& astVarNode,
		                                const AST::Type* const initialiseType) {
			const bool isTopLevel = true;
			return ConvertInitialisedVarRecurse(context, astVarNode, initialiseType, isTopLevel);
		}
		
	}
	
}

