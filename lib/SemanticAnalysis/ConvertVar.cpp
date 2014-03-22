#include <stdexcept>

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ConvertVar.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>

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
		
		// Attach the variable to the SemanticAnalysis node tree.
		void attachVar(Context& context, const std::string& name, const AST::Node<AST::TypeVar>& astTypeVarNode, SEM::Var* var) {
			assert(var->isBasic());
			
			const Node localVarNode = Node::Variable(astTypeVarNode, var);
			const bool attachResult = context.node().tryAttach(name, localVarNode);
			
			if (!attachResult) {
				throw TodoException(makeString("Variable name '%s' already exists.", name.c_str()));
			}
			
			// TODO: add support for member and parameter variables.
			const auto varInfo = makeVarInfo(Debug::VarInfo::VAR_AUTO, astTypeVarNode);
			context.debugModule().varMap.insert(std::make_pair(var, makeVarInfo(Debug::VarInfo::VAR_AUTO, astTypeVarNode)));
		}
		
		namespace {
			
			SEM::Type* CastType(SEM::Type* sourceType, SEM::Type* destType, bool isTopLevel) {
				// Pattern matched members are restricted
				// to format only casts.
				const bool formatOnly = !isTopLevel;
				
				const auto value = ImplicitCast(SEM::Value::CastDummy(sourceType), destType, formatOnly);
				return value->type();
			}
			
			SEM::Var* ConvertInitialisedVarRecurse(Context& context, bool isMember, const AST::Node<AST::TypeVar>& astTypeVarNode, SEM::Type* initialiseType, bool isTopLevel) {
				switch (astTypeVarNode->kind) {
					case AST::TypeVar::ANYVAR: {
						return SEM::Var::Any(initialiseType);
					}
					
					case AST::TypeVar::NAMEDVAR: {
						const auto& varName = astTypeVarNode->namedVar.name;
						
						if (!context.lookupName(Name::Relative() + varName).isNone()) {
							throw TodoException(makeString("Variable '%s' shadows existing object.", varName.c_str()));
						}
						
						const auto varDeclType = ConvertType(context, astTypeVarNode->namedVar.type);
						
						// Use cast to resolve any instances of
						// 'auto' in the variable's type.
						const auto varType = CastType(initialiseType, varDeclType, isTopLevel);
						
						if (varType->isVoid()) {
							throw TodoException(makeString("Variable '%s' cannot have void type.",
									astTypeVarNode->namedVar.name.c_str()));
						}
						
						// 'final' keyword makes the default lval const.
						const bool isLvalConst = astTypeVarNode->namedVar.isFinal;
						
						const auto lvalType = makeLvalType(context, isMember, isLvalConst, varType);
						
						const auto var = SEM::Var::Basic(varType, lvalType);
						attachVar(context, varName, astTypeVarNode, var);
						return var;
					}
					
					case AST::TypeVar::PATTERNVAR: {
						const auto varDeclType = ConvertType(context, astTypeVarNode->patternVar.type);
						
						if (!varDeclType->isDatatype()) {
							throw TodoException(makeString("Can't pattern match for non-datatype '%s'.",
									varDeclType->toString().c_str()));
						}
						
						// Use cast to resolve any instances of
						// 'auto' in the variable's type.
						const auto varType = CastType(initialiseType, varDeclType, isTopLevel);
						
						const auto& astChildTypeVars = astTypeVarNode->patternVar.typeVarList;
						const auto& typeChildVars = varType->getObjectType()->variables();
						
						if (astChildTypeVars->size() != typeChildVars.size()) {
							throw TodoException(makeString("%llu pattern match children specified; %llu expected (for type '%s').",
									(unsigned long long) astChildTypeVars->size(),
									(unsigned long long) typeChildVars.size(),
									varType->toString().c_str()));
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
					
					default: {
						throw std::runtime_error("Unknown typevar kind.");
					}
				}
			}
			
		}
		
		SEM::Var* ConvertVar(Context& context, bool isMember, const AST::Node<AST::TypeVar>& astTypeVarNode) {
			switch (astTypeVarNode->kind) {
				case AST::TypeVar::ANYVAR: {
					throw TodoException("'Any' vars not yet implemented for uninitialised variables.");
				}
				
				case AST::TypeVar::NAMEDVAR: {
					const auto& varName = astTypeVarNode->namedVar.name;
					
					if (!context.lookupName(Name::Relative() + varName).isNone()) {
						throw TodoException(makeString("Variable '%s' shadows existing object.", varName.c_str()));
					}
					
					const auto varType = ConvertType(context, astTypeVarNode->namedVar.type);
					
					if (varType->isVoid()) {
						throw TodoException(makeString("Variable '%s' cannot have void type.", varName.c_str()));
					}
					
					// 'final' keyword makes the default lval const.
					const bool isLvalConst = astTypeVarNode->namedVar.isFinal;
					
					const auto lvalType = makeLvalType(context, isMember, isLvalConst, varType);
					
					const auto var = SEM::Var::Basic(varType, lvalType);
					attachVar(context, varName, astTypeVarNode, var);
					return var;
				}
				
				case AST::TypeVar::PATTERNVAR: {
					const auto varType = ConvertType(context, astTypeVarNode->patternVar.type);
					
					if (!varType->isDatatype()) {
						throw TodoException(makeString("Can't pattern match for non-datatype '%s'.",
								varType->toString().c_str()));
					}
					
					const auto& astChildTypeVars = astTypeVarNode->patternVar.typeVarList;
					const auto& typeChildVars = varType->getObjectType()->variables();
					
					if (astChildTypeVars->size() != typeChildVars.size()) {
						throw TodoException(makeString("%llu pattern match children specified; %llu expected (for type '%s').",
								(unsigned long long) astChildTypeVars->size(),
								(unsigned long long) typeChildVars.size(),
								varType->toString().c_str()));
					}
					
					const auto templateVarMap = varType->generateTemplateVarMap();
					
					std::vector<SEM::Var*> children;
					
					for (size_t i = 0; i < typeChildVars.size(); i++) {
						const auto& astVar = astChildTypeVars->at(i);
						children.push_back(ConvertVar(context, isMember, astVar));
					}
					
					return SEM::Var::Composite(varType, children);
				}
				
				default: {
					throw std::runtime_error("Unknown typevar kind.");
				}
			}
		}
		
		SEM::Var* ConvertInitialisedVar(Context& context, bool isMember, const AST::Node<AST::TypeVar>& astTypeVarNode, SEM::Type* initialiseType) {
			const bool isTopLevel = true;
			return ConvertInitialisedVarRecurse(context, isMember, astTypeVarNode, initialiseType, isTopLevel);
		}
		
	}
	
}

