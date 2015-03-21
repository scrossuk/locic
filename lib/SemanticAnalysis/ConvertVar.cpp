#include <stdexcept>

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/CanCast.hpp>
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
					return element.scope()->namedVariables().insert(std::make_pair(name, var));
				} else if (element.isSwitchCase()) {
					return element.switchCase()->namedVariables().insert(std::make_pair(name, var));
				} else if (element.isCatchClause()) {
					return element.catchClause()->namedVariables().insert(std::make_pair(name, var));
				} else if (element.isFunction()) {
					return element.function()->namedVariables().insert(std::make_pair(name, var));
				} else if (element.isTypeInstance()) {
					return element.typeInstance()->namedVariables().insert(std::make_pair(name, var));
				} else {
					assert(false && "Invalid element kind for inserting var.");
					throw std::logic_error("Invalid element kind for inserting var.");
				}
			}
			
		}
		
		// Attach the variable to the SemanticAnalysis node tree.
		void attachVar(Context& context, const String& name, const AST::Node<AST::TypeVar>& astTypeVarNode, SEM::Var* const var, const Debug::VarInfo::Kind varKind) {
			assert(var->isBasic());
			
			const auto insertResult = insertVar(context.scopeStack().back(), name, var);
			if (!insertResult.second) {
				const auto existingVar = insertResult.first->second;
				throw ErrorException(makeString("Variable name '%s' at position %s duplicates existing variable of the same name at position %s.",
					name.c_str(), astTypeVarNode.location().toString().c_str(),
					existingVar->debugInfo()->declLocation.toString().c_str()));
			}
			
			var->setDebugInfo(makeVarInfo(varKind, astTypeVarNode));
		}
		
		namespace {
			
			const SEM::Type* CastType(Context& context, const SEM::Type* sourceType, const SEM::Type* destType, const Debug::SourceLocation& location, bool isTopLevel) {
				// Pattern matched members are restricted
				// to format only casts.
				const bool formatOnly = !isTopLevel;
				
				const auto value = ImplicitCast(context, SEM::Value::CastDummy(sourceType), destType, location, formatOnly);
				return value.type();
			}
			
			SEM::Var* ConvertInitialisedVarRecurse(Context& context, const AST::Node<AST::TypeVar>& astTypeVarNode, const SEM::Type* initialiseType, bool isTopLevel) {
				const auto& location = astTypeVarNode.location();
				
				switch (astTypeVarNode->kind()) {
					case AST::TypeVar::ANYVAR: {
						return SEM::Var::Any(initialiseType);
					}
					
					case AST::TypeVar::NAMEDVAR: {
						const auto& varName = astTypeVarNode->name();
						
						// Search all scopes outside of the current scope.
						const auto searchStartPosition = 1;
						if (!performSearch(context, Name::Relative() + varName, searchStartPosition).isNone()) {
							throw ErrorException(makeString("Variable '%s' shadows existing object at position %s.",
								varName.c_str(), location.toString().c_str()));
						}
						
						const auto varDeclType = ConvertType(context, astTypeVarNode->namedType())->resolveAliases();
						
						// Use cast to resolve any instances of
						// 'auto' in the variable's type.
						const auto varType = CastType(context, initialiseType, varDeclType, location, isTopLevel);
						
						if (varType->isBuiltInVoid()) {
							throw ErrorException(makeString("Variable '%s' cannot have void type at position %s.",
								astTypeVarNode->name().c_str(), location.toString().c_str()));
						}
						
						// 'final' keyword uses a different lval type (which doesn't support
						// moving or re-assignment).
						const bool isFinalLval = astTypeVarNode->isFinal();
						
						const bool isMember = false;
						const auto lvalType = makeLvalType(context, isMember, isFinalLval, varType);
						
						const auto var = SEM::Var::Basic(varType, lvalType);
						var->setMarkedUnused(astTypeVarNode->isUnused());
						var->setOverrideConst(astTypeVarNode->isOverrideConst());
						attachVar(context, varName, astTypeVarNode, var, Debug::VarInfo::VAR_LOCAL);
						return var;
					}
					
					case AST::TypeVar::PATTERNVAR: {
						const auto varDeclType = ConvertType(context, astTypeVarNode->patternType())->resolveAliases();
						
						if (!varDeclType->isDatatype()) {
							throw ErrorException(makeString("Can't pattern match for non-datatype '%s' at position %s.",
								varDeclType->toString().c_str(), location.toString().c_str()));
						}
						
						// Use cast to resolve any instances of
						// 'auto' in the variable's type.
						const auto varType = CastType(context, initialiseType, varDeclType, location, isTopLevel);
						
						const auto& astChildTypeVars = astTypeVarNode->typeVarList();
						const auto& typeChildVars = varType->getObjectType()->variables();
						
						if (astChildTypeVars->size() != typeChildVars.size()) {
							throw ErrorException(makeString("%llu pattern match children specified; %llu expected (for type '%s') at position %s.",
									(unsigned long long) astChildTypeVars->size(),
									(unsigned long long) typeChildVars.size(),
									varType->toString().c_str(),
									location.toString().c_str()));
						}
						
						const auto templateVarMap = varType->generateTemplateVarMap();
						
						std::vector<SEM::Var*> children;
						
						for (size_t i = 0; i < typeChildVars.size(); i++) {
							const auto& astVar = astChildTypeVars->at(i);
							const auto& semVar = typeChildVars.at(i);
							
							const auto childInitialiseType = semVar->constructType()->substitute(templateVarMap);
							const bool childIsTopLevel = false;
							children.push_back(ConvertInitialisedVarRecurse(context, astVar, childInitialiseType, childIsTopLevel));
						}
						
						return SEM::Var::Composite(varType, children);
					}
				}
				
				std::terminate();
			}
			
		}
		
		SEM::Var* ConvertVar(Context& context, const Debug::VarInfo::Kind varKind, const AST::Node<AST::TypeVar>& astTypeVarNode) {
			const auto& location = astTypeVarNode.location();
			
			switch (astTypeVarNode->kind()) {
				case AST::TypeVar::ANYVAR: {
					throw ErrorException("'Any' vars not yet implemented for uninitialised variables.");
				}
				
				case AST::TypeVar::NAMEDVAR: {
					const auto& varName = astTypeVarNode->name();
					
					// Search all scopes outside of the current scope.
					const auto searchStartPosition = 1;
					if (varKind != Debug::VarInfo::VAR_MEMBER && !performSearch(context, Name::Relative() + varName, searchStartPosition).isNone()) {
						throw ErrorException(makeString("Variable '%s' shadows existing object at position %s.",
							varName.c_str(), location.toString().c_str()));
					}
					
					const auto varType = ConvertType(context, astTypeVarNode->namedType())->resolveAliases();
					
					if (varType->isBuiltInVoid()) {
						throw ErrorException(makeString("Variable '%s' cannot have void type at position %s.",
							varName.c_str(), location.toString().c_str()));
					}
					
					// 'final' keyword uses a different lval type (which doesn't support
					// moving or re-assignment).
					const bool isFinalLval = astTypeVarNode->isFinal();
					
					const bool isMember = (varKind == Debug::VarInfo::VAR_MEMBER);
					const auto lvalType = makeLvalType(context, isMember, isFinalLval, varType);
					
					const auto var = SEM::Var::Basic(varType, lvalType);
					var->setMarkedUnused(astTypeVarNode->isUnused());
					var->setOverrideConst(astTypeVarNode->isOverrideConst());
					attachVar(context, varName, astTypeVarNode, var, varKind);
					return var;
				}
				
				case AST::TypeVar::PATTERNVAR: {
					const auto varType = ConvertType(context, astTypeVarNode->patternType())->resolveAliases();
					
					if (!varType->isDatatype()) {
						throw ErrorException(makeString("Can't pattern match for non-datatype '%s' at position %s.",
							varType->toString().c_str(), location.toString().c_str()));
					}
					
					const auto& astChildTypeVars = astTypeVarNode->typeVarList();
					const auto& typeChildVars = varType->getObjectType()->variables();
					
					if (astChildTypeVars->size() != typeChildVars.size()) {
						throw ErrorException(makeString("%llu pattern match children specified; %llu expected (for type '%s') at position %s.",
								(unsigned long long) astChildTypeVars->size(),
								(unsigned long long) typeChildVars.size(),
								varType->toString().c_str(),
								location.toString().c_str()));
					}
					
					const auto templateVarMap = varType->generateTemplateVarMap();
					
					std::vector<SEM::Var*> children;
					
					for (size_t i = 0; i < typeChildVars.size(); i++) {
						const auto& astVar = astChildTypeVars->at(i);
						children.push_back(ConvertVar(context, varKind, astVar));
					}
					
					return SEM::Var::Composite(varType, children);
				}
			}
			
			std::terminate();
		}
		
		SEM::Var* ConvertInitialisedVar(Context& context, const AST::Node<AST::TypeVar>& astTypeVarNode, const SEM::Type* const initialiseType) {
			const bool isTopLevel = true;
			return ConvertInitialisedVarRecurse(context, astTypeVarNode, initialiseType, isTopLevel);
		}
		
	}
	
}

