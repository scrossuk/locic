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
	
		namespace {
			
			// Attach the variable to the SemanticAnalysis node tree.
			void attachVar(Context& context, const std::string& name, const AST::Node<AST::TypeVar>& astTypeVarNode, SEM::Var* var) {
				assert(var->isBasic());
				const Node localVarNode = Node::Variable(astTypeVarNode, var);
				const bool attachResult = context.node().tryAttach(name, localVarNode);
				
				if (!attachResult) {
					throw TodoException(makeString("Variable name '%s' already exists.", name.c_str()));
				}
			}
			
		}
		
		SEM::Var* ConvertVar(Context& context, bool isMember, const AST::Node<AST::TypeVar>& astTypeVarNode) {
			switch (astTypeVarNode->kind) {
				case AST::TypeVar::ANYVAR:
				{
					throw TodoException("Pattern vars not yet implemented for uninitialised variables.");
				}
				case AST::TypeVar::NAMEDVAR:
				{
					const auto& varName = astTypeVarNode->namedVar.name;
					
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
				case AST::TypeVar::PATTERNVAR:
				{
					throw TodoException("Pattern vars not yet implemented for uninitialised variables.");
				}
				default:
				{
					throw std::runtime_error("Unknown typevar kind.");
				}
			}
		}
		
		SEM::Var* ConvertInitialisedVar(Context& context, bool isMember, const AST::Node<AST::TypeVar>& astTypeVarNode, SEM::Type* initialiseType) {
			switch (astTypeVarNode->kind) {
				case AST::TypeVar::ANYVAR:
				{
					return SEM::Var::Any(initialiseType);
				}
				case AST::TypeVar::NAMEDVAR:
				{
					const auto& varName = astTypeVarNode->namedVar.name;
					
					const auto varDeclType = ConvertType(context, astTypeVarNode->namedVar.type);
					
					// Use ImplicitCast() to resolve any instances of
					// 'auto' in the variable's type.
					const auto castInitialiseValue = ImplicitCast(SEM::Value::CastDummy(initialiseType), varDeclType);
					
					const auto varType = castInitialiseValue->type();
					
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
				case AST::TypeVar::PATTERNVAR:
				{
					const auto varDeclType = ConvertType(context, astTypeVarNode->patternVar.type);
					
					if (!varDeclType->isDatatype()) {
						throw TodoException(makeString("Can't pattern match for non-datatype '%s'.",
							varDeclType->toString().c_str()));
					}
					
					// Use ImplicitCast() to resolve any instances of
					// 'auto' in the variable's type.
					const auto castInitialiseValue = ImplicitCast(SEM::Value::CastDummy(initialiseType), varDeclType);
					
					const auto varType = castInitialiseValue->type();
					
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
						
						const auto childInitialiseType = semVar->type()->substitute(templateVarMap);
						children.push_back(ConvertInitialisedVar(context, isMember, astVar, childInitialiseType));
					}
					
					return SEM::Var::Composite(varType, children);
				}
				default:
				{
					throw std::runtime_error("Unknown typevar kind.");
				}
			}
		}
		
	}
	
}

