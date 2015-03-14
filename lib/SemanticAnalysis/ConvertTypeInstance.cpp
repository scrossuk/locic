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
	
		void CreateEnumConstructorMethod(Context& context, SEM::TypeInstance* const typeInstance, SEM::Function* const function, const size_t value) {
			assert(typeInstance->isEnum());
			
			auto functionScope = SEM::Scope::Create();
			
			HeapArray<SEM::Value> constructValues;
			
			const auto intConstant = Constant::Integer(static_cast<Constant::IntegerVal>(value));
			const auto intType = getBuiltInType(context, context.getCString("int_t"), {});
			constructValues.push_back(SEM::Value::Constant(intConstant, intType));
			
			auto internalConstructedValue = SEM::Value::InternalConstruct(typeInstance, std::move(constructValues));
			functionScope->statements().push_back(SEM::Statement::Return(std::move(internalConstructedValue)));
			
			function->setScope(std::move(functionScope));
		}
		
		void ConvertTypeInstance(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			const auto semTypeInstance = context.scopeStack().back().typeInstance();
			
			bool createdMove = false;
			const auto moveString = context.getCString("__moveto");
			
			for (const auto& astFunctionNode: *(astTypeInstanceNode->functions)) {
				const auto methodName = CanonicalizeMethodName(astFunctionNode->name()->last());
				
				if (methodName == moveString) {
					createdMove = true;
				}
				
				const auto semChildFunction = semTypeInstance->functions().at(methodName);
				
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(semChildFunction));
				ConvertFunctionDef(context, astFunctionNode);
			}
			
			if (semTypeInstance->isEnum()) {
				size_t enumValue = 0;
				// Generate enum constructors.
				for (const auto& constructorName: *(astTypeInstanceNode->constructors)) {
					const auto canonicalMethodName = CanonicalizeMethodName(constructorName);
					CreateEnumConstructorMethod(context, semTypeInstance,
						semTypeInstance->functions().at(canonicalMethodName), enumValue++);
				}
			}
			
			// Generate default move for applicable types.
			if ((semTypeInstance->isClassDef() || semTypeInstance->isException() || semTypeInstance->isStruct() ||
					semTypeInstance->isDatatype() || semTypeInstance->isUnionDatatype() ||
					semTypeInstance->isEnum() || semTypeInstance->isUnion()) && !createdMove) {
				CreateDefaultMethod(context, semTypeInstance, semTypeInstance->functions().at(moveString), astTypeInstanceNode.location());
			}
			
			// Generate default constructor for applicable types.
			if (semTypeInstance->isException()) {
				CreateExceptionConstructor(context, astTypeInstanceNode, semTypeInstance, semTypeInstance->functions().at(context.getCString("create")));
			} else if (semTypeInstance->isDatatype() || semTypeInstance->isStruct() || semTypeInstance->isUnion()) {
				CreateDefaultMethod(context, semTypeInstance, semTypeInstance->functions().at(context.getCString("create")), astTypeInstanceNode.location());
			}
			
			// Generate default implicitCopy if relevant.
			if (semTypeInstance->isEnum() || semTypeInstance->isStruct() || semTypeInstance->isDatatype() ||
					semTypeInstance->isUnionDatatype() || semTypeInstance->isUnion()) {
				const auto iterator = semTypeInstance->functions().find(context.getCString("implicitcopy"));
				if (iterator != semTypeInstance->functions().end()) {
					CreateDefaultMethod(context, semTypeInstance, iterator->second, astTypeInstanceNode.location());
				}
			}
			
			// Generate default compare if relevant.
			if (semTypeInstance->isEnum() || semTypeInstance->isStruct() || semTypeInstance->isDatatype() || semTypeInstance->isUnionDatatype()) {
				const auto iterator = semTypeInstance->functions().find(context.getCString("compare"));
				if (iterator != semTypeInstance->functions().end()) {
					CreateDefaultMethod(context, semTypeInstance, iterator->second, astTypeInstanceNode.location());
				}
			}
		}
		
	}
	
}

