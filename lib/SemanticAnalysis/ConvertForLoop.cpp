#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertForLoop.hpp>
#include <locic/SemanticAnalysis/ConvertScope.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/ConvertVar.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
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
			
			{
				PushScopeElement pushOuterScope(context.scopeStack(), ScopeElement::Scope(outerScope));
				
				const auto initValue = ConvertValue(context, astInitValueNode);
				
				const auto initVarType = (initValue->type()->isLvalOrRef()) ?
						createReferenceType(context, initValue->type()->lvalOrRefTarget()) :
						initValue->type();
				
				const auto initVar = SEM::Var::Basic(initVarType, initVarType);
				outerScope->variables().push_back(initVar);
				
				outerScope->statements().push_back(SEM::Statement::InitialiseStmt(initVar, ImplicitCast(context, initValue, initVarType, location)));
				
				{
					PushScopeElement pushLoop(context.scopeStack(), ScopeElement::Loop());
					
					const auto isEmpty = CallValue(context, GetMethod(context, createLocalVarRef(context, initVar), "empty", location), {}, location);
					const auto isNotEmpty = CallValue(context, GetMethod(context, isEmpty, "not", location), {}, location);
					const auto loopCondition = ImplicitCast(context, isNotEmpty, getBuiltInType(context.scopeStack(), "bool")->selfType(), location);
					
					const auto iterationScope = new SEM::Scope();
					
					{
						PushScopeElement pushIterationScope(context.scopeStack(), ScopeElement::Scope(iterationScope));
						
						const auto currentValue = CallValue(context, GetMethod(context, createLocalVarRef(context, initVar), "front", location), {}, location);
						
						const bool isMember = false;
						const auto loopVar = ConvertInitialisedVar(context, isMember, astTypeVarNode, currentValue->type());
						iterationScope->variables().push_back(loopVar);
						
						iterationScope->statements().push_back(SEM::Statement::InitialiseStmt(loopVar,
							ImplicitCast(context, currentValue, loopVar->constructType(), location)));
						
						const auto innerScope = ConvertScope(context, astScopeNode);
						
						iterationScope->statements().push_back(SEM::Statement::ScopeStmt(innerScope));
					}
					
					const auto advanceScope = new SEM::Scope();
					const auto advanceCurrentValue = CallValue(context, GetMethod(context, createLocalVarRef(context, initVar), "popFront", location), {}, location);
					advanceScope->statements().push_back(SEM::Statement::ValueStmt(advanceCurrentValue));
					
					outerScope->statements().push_back(SEM::Statement::Loop(loopCondition, iterationScope, advanceScope));
				}
			}
			
			return outerScope;
		}
		
	}
	
}


