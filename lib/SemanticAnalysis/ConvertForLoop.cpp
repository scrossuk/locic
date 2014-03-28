#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertForLoop.hpp>
#include <locic/SemanticAnalysis/ConvertScope.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/ConvertVar.hpp>
#include <locic/SemanticAnalysis/Node.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		/**
		 * This code converts:
		 * for (type value_var: range_value) {
		 *     //...
		 * }
		 * 
		 * ...to (roughly):
		 * 
		 * {
		 *     auto anon_var = range_value;
		 *     loop {
		 *         condition: !anon_var.empty(),
		 *         iterationScope:
		 *         {
		 *             type value_var = anon_var.front();
		 *             {
		 *                 //...
		 *             }
		 *         },
		 *         advanceScope:
		 *         {
		 *             anon_var.popFront();
		 *         }
		 *     }
		 * }
		 */
		
		SEM::Scope* ConvertForLoop(Context& context, const AST::Node<AST::TypeVar>& astTypeVarNode, const AST::Node<AST::Value>& astInitValueNode, const AST::Node<AST::Scope>& astScopeNode) {
			// TODO: fix this to be the correct location.
			const auto& location = astScopeNode.location();
			
			const auto outerScope = new SEM::Scope();
			
			const auto initValue = ConvertValue(context, astInitValueNode);
			
			const auto initVarType = (initValue->type()->isLvalOrRef()) ?
					SEM::Type::Reference(initValue->type()->lvalOrRefTarget())->createRefType(initValue->type()->lvalOrRefTarget()) :
					initValue->type();
			
			const auto initVar = SEM::Var::Basic(initVarType, initVarType);
			outerScope->localVariables().push_back(initVar);
			
			outerScope->statements().push_back(SEM::Statement::InitialiseStmt(initVar, ImplicitCast(initValue, initVarType)));
			
			const auto isEmpty = CallValue(GetMethod(SEM::Value::LocalVar(initVar), "empty"), {}, location);
			const auto isNotEmpty = CallValue(GetMethod(isEmpty, "not"), {}, location);
			const auto loopCondition = ImplicitCast(isNotEmpty, getBuiltInType(context, "bool")->selfType());
			
			const auto iterationScope = new SEM::Scope();
			auto scopeNode = Node::Scope(astScopeNode, iterationScope);
			NodeContext scopeContext(context, "##scope", scopeNode);
			
			const bool isMember = false;
			
			const auto currentValue = CallValue(GetMethod(SEM::Value::LocalVar(initVar), "front"), {}, location);
			const auto loopVar = ConvertInitialisedVar(scopeContext, isMember, astTypeVarNode, currentValue->type());
			iterationScope->localVariables().push_back(loopVar);
			
			iterationScope->statements().push_back(SEM::Statement::InitialiseStmt(loopVar, ImplicitCast(currentValue, loopVar->constructType())));
			
			const auto innerScope = ConvertScope(scopeContext, astScopeNode);
			
			iterationScope->statements().push_back(SEM::Statement::ScopeStmt(innerScope));
			
			const auto advanceScope = new SEM::Scope();
			const auto advanceCurrentValue = CallValue(GetMethod(SEM::Value::LocalVar(initVar), "popFront"), {}, location);
			advanceScope->statements().push_back(SEM::Statement::ValueStmt(advanceCurrentValue));
			
			outerScope->statements().push_back(SEM::Statement::Loop(loopCondition, iterationScope, advanceScope));
			
			return outerScope;
		}
		
	}
	
}


