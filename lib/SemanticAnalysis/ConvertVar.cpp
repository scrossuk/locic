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

namespace locic {

	namespace SemanticAnalysis {
	
		Debug::VarInfo makeVarInfo(Debug::VarInfo::Kind kind, const AST::Node<AST::TypeVar>& astTypeVarNode) {
			assert(astTypeVarNode->kind == AST::TypeVar::NAMEDVAR);
			
			Debug::VarInfo varInfo;
			varInfo.kind = kind;
			varInfo.name = astTypeVarNode->namedVar.name;
			varInfo.declLocation = astTypeVarNode.location();
			
			// TODO
			varInfo.scopeLocation = Debug::SourceLocation::Null();
			return varInfo;
		}
		
		namespace {
			
			bool insertVar(const ScopeElement& element, const std::string& name, SEM::Var* var) {
				if (element.isScope()) {
					return element.scope()->namedVariables().insert(std::make_pair(name, var)).second;
				} else if (element.isSwitchCase()) {
					return element.switchCase()->namedVariables().insert(std::make_pair(name, var)).second;
				} else if (element.isCatchClause()) {
					return element.catchClause()->namedVariables().insert(std::make_pair(name, var)).second;
				} else {
					assert(false && "Invalid element kind for inserting var.");
					return false;
				}
			}
			
		}
		
		// Attach the variable to the SemanticAnalysis node tree.
		void attachVar(Context& context, const std::string& name, const AST::Node<AST::TypeVar>& astTypeVarNode, SEM::Var* var) {
			assert(var->isBasic());
			
			const auto insertResult = insertVar(context.scopeStack().back(), name, var);
			if (!insertResult) {
				throw ErrorException(makeString("Variable name '%s' already exists at position %s.",
					name.c_str(), astTypeVarNode.location().toString().c_str()));
			}
			
			// TODO: add support for member and parameter variables.
			const auto varInfo = makeVarInfo(Debug::VarInfo::VAR_AUTO, astTypeVarNode);
			context.debugModule().varMap.insert(std::make_pair(var, makeVarInfo(Debug::VarInfo::VAR_AUTO, astTypeVarNode)));
		}
		
		namespace {
			
			SEM::Type* CastType(Context& context, SEM::Type* sourceType, SEM::Type* destType, const Debug::SourceLocation& location, bool isTopLevel) {
				// Pattern matched members are restricted
				// to format only casts.
				const bool formatOnly = !isTopLevel;
				
				const auto value = ImplicitCast(context, SEM::Value::CastDummy(sourceType), destType, location, formatOnly);
				return value->type();
			}
			
			SEM::Var* ConvertInitialisedVarRecurse(Context& context, bool isMember, const AST::Node<AST::TypeVar>& astTypeVarNode, SEM::Type* initialiseType, bool isTopLevel) {
				const auto& location = astTypeVarNode.location();
				
				switch (astTypeVarNode->kind) {
					case AST::TypeVar::ANYVAR: {
						return SEM::Var::Any(initialiseType);
					}
					
					case AST::TypeVar::NAMEDVAR: {
						const auto& varName = astTypeVarNode->namedVar.name;
						
						if (!performSearch(context, Name::Relative() + varName).isNone()) {
							throw ErrorException(makeString("Variable '%s' shadows existing object at position %s.",
								varName.c_str(), location.toString().c_str()));
						}
						
						const auto varDeclType = ConvertType(context, astTypeVarNode->namedVar.type)->resolveAliases();
						
						// Use cast to resolve any instances of
						// 'auto' in the variable's type.
						const auto varType = CastType(context, initialiseType, varDeclType, location, isTopLevel);
						
						if (varType->isBuiltInVoid()) {
							throw ErrorException(makeString("Variable '%s' cannot have void type at position %s.",
								astTypeVarNode->namedVar.name.c_str(), location.toString().c_str()));
						}
						
						// 'final' keyword makes the default lval const.
						const bool isLvalConst = astTypeVarNode->namedVar.isFinal;
						
						const auto lvalType = makeLvalType(context, isMember, isLvalConst, varType);
						
						const auto var = SEM::Var::Basic(varType, lvalType);
						attachVar(context, varName, astTypeVarNode, var);
						return var;
					}
					
					case AST::TypeVar::PATTERNVAR: {
						const auto varDeclType = ConvertType(context, astTypeVarNode->patternVar.type)->resolveAliases();
						
						if (!varDeclType->isDatatype()) {
							throw ErrorException(makeString("Can't pattern match for non-datatype '%s' at position %s.",
								varDeclType->toString().c_str(), location.toString().c_str()));
						}
						
						// Use cast to resolve any instances of
						// 'auto' in the variable's type.
						const auto varType = CastType(context, initialiseType, varDeclType, location, isTopLevel);
						
						const auto& astChildTypeVars = astTypeVarNode->patternVar.typeVarList;
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
							children.push_back(ConvertInitialisedVarRecurse(context, isMember, astVar, childInitialiseType, childIsTopLevel));
						}
						
						return SEM::Var::Composite(varType, children);
					}
				}
				
				std::terminate();
			}
			
		}
		
		SEM::Var* ConvertVar(Context& context, bool isMember, const AST::Node<AST::TypeVar>& astTypeVarNode) {
			const auto& location = astTypeVarNode.location();
			
			switch (astTypeVarNode->kind) {
				case AST::TypeVar::ANYVAR: {
					throw ErrorException("'Any' vars not yet implemented for uninitialised variables.");
				}
				
				case AST::TypeVar::NAMEDVAR: {
					const auto& varName = astTypeVarNode->namedVar.name;
					
					if (!performSearch(context, Name::Relative() + varName).isNone()) {
						throw ErrorException(makeString("Variable '%s' shadows existing object at position %s.",
							varName.c_str(), location.toString().c_str()));
					}
					
					const auto varType = ConvertType(context, astTypeVarNode->namedVar.type)->resolveAliases();
					
					if (varType->isBuiltInVoid()) {
						throw ErrorException(makeString("Variable '%s' cannot have void type at position %s.",
							varName.c_str(), location.toString().c_str()));
					}
					
					// 'final' keyword makes the default lval const.
					const bool isLvalConst = astTypeVarNode->namedVar.isFinal;
					
					const auto lvalType = makeLvalType(context, isMember, isLvalConst, varType);
					
					const auto var = SEM::Var::Basic(varType, lvalType);
					attachVar(context, varName, astTypeVarNode, var);
					return var;
				}
				
				case AST::TypeVar::PATTERNVAR: {
					const auto varType = ConvertType(context, astTypeVarNode->patternVar.type)->resolveAliases();
					
					if (!varType->isDatatype()) {
						throw ErrorException(makeString("Can't pattern match for non-datatype '%s' at position %s.",
							varType->toString().c_str(), location.toString().c_str()));
					}
					
					const auto& astChildTypeVars = astTypeVarNode->patternVar.typeVarList;
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
						children.push_back(ConvertVar(context, isMember, astVar));
					}
					
					return SEM::Var::Composite(varType, children);
				}
			}
			
			std::terminate();
		}
		
		SEM::Var* ConvertInitialisedVar(Context& context, bool isMember, const AST::Node<AST::TypeVar>& astTypeVarNode, SEM::Type* initialiseType) {
			const bool isTopLevel = true;
			return ConvertInitialisedVarRecurse(context, isMember, astTypeVarNode, initialiseType, isTopLevel);
		}
		
	}
	
}

