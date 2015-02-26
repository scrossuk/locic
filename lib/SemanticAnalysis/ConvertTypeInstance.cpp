#include <cassert>
#include <locic/AST.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertException.hpp>
#include <locic/SemanticAnalysis/ConvertFunctionDef.hpp>
#include <locic/SemanticAnalysis/ConvertTypeInstance.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		void ConvertTypeInstance(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			const auto semTypeInstance = context.scopeStack().back().typeInstance();
			
			bool createdMove = false;
			
			for (const auto& astFunctionNode: *(astTypeInstanceNode->functions)) {
				const auto methodName = CanonicalizeMethodName(astFunctionNode->name()->last());
				
				if (methodName == context.getCString("__moveto")) {
					createdMove = true;
				}
				
				const auto semChildFunction = semTypeInstance->functions().at(methodName);
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(semChildFunction));
				ConvertFunctionDef(context, astFunctionNode);
			}
			
			// Generate default move for applicable types.
			if ((semTypeInstance->isClassDef() || semTypeInstance->isException() || semTypeInstance->isStruct() ||
					semTypeInstance->isDatatype() || semTypeInstance->isUnionDatatype()) && !createdMove) {
				CreateDefaultMethod(context, semTypeInstance, semTypeInstance->functions().at(context.getCString("__moveto")), astTypeInstanceNode.location());
			}
			
			// Generate default constructor for applicable types.
			if (semTypeInstance->isException()) {
				CreateExceptionConstructor(context, astTypeInstanceNode, semTypeInstance, semTypeInstance->functions().at(context.getCString("create")));
			} else if (semTypeInstance->isDatatype() ||semTypeInstance->isStruct() || semTypeInstance->isException()) {
				CreateDefaultMethod(context, semTypeInstance, semTypeInstance->functions().at(context.getCString("create")), astTypeInstanceNode.location());
			}
			
			// Generate default implicitCopy if relevant.
			if (semTypeInstance->isStruct() || semTypeInstance->isDatatype() || semTypeInstance->isUnionDatatype()) {
				const auto iterator = semTypeInstance->functions().find(context.getCString("implicitcopy"));
				if (iterator != semTypeInstance->functions().end()) {
					CreateDefaultMethod(context, semTypeInstance, iterator->second, astTypeInstanceNode.location());
				}
			}
			
			// Generate default compare if relevant.
			if (semTypeInstance->isStruct() || semTypeInstance->isDatatype() || semTypeInstance->isUnionDatatype()) {
				const auto iterator = semTypeInstance->functions().find(context.getCString("compare"));
				if (iterator != semTypeInstance->functions().end()) {
					CreateDefaultMethod(context, semTypeInstance, iterator->second, astTypeInstanceNode.location());
				}
			}
		}
		
	}
	
}

