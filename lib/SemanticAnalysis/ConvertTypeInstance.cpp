#include <cassert>
#include <locic/AST.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertException.hpp>
#include <locic/SemanticAnalysis/ConvertFunctionDef.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertTypeInstance.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		void CreateEnumConstructorMethod(Context& context, SEM::TypeInstance* const typeInstance, SEM::Function& function, const size_t value) {
			assert(typeInstance->isEnum());
			
			auto functionScope = SEM::Scope::Create();
			
			HeapArray<SEM::Value> constructValues;
			
			const auto intConstant = Constant::Integer(static_cast<Constant::IntegerVal>(value));
			const auto intType = getBuiltInType(context, context.getCString("int_t"), {});
			constructValues.push_back(SEM::Value::Constant(intConstant, intType));
			
			auto internalConstructedValue = SEM::Value::InternalConstruct(typeInstance->selfType(), std::move(constructValues));
			functionScope->statements().push_back(SEM::Statement::Return(std::move(internalConstructedValue)));
			
			function.setScope(std::move(functionScope));
		}
		
		void CreateDefaultMethodOrRemove(Context& context, SEM::TypeInstance* const typeInstance, SEM::Function* const function, const Debug::SourceLocation& location) {
			const bool created = CreateDefaultMethod(context, typeInstance, function, location);
			if (!created) {
				// Make sure that the require predicate is false so
				// CodeGen understands not to generate this function.
				function->setRequiresPredicate(SEM::Predicate::False());
			}
		}
		
		void ConvertTypeInstance(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode) {
			auto& semTypeInstance = context.scopeStack().back().typeInstance();
			
			for (const auto& astFunctionNode: *(astTypeInstanceNode->functions)) {
				auto& semChildFunction = astFunctionNode->semFunction();
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(semChildFunction));
				ConvertFunctionDef(context, astFunctionNode);
			}
			
			if (semTypeInstance.isEnum()) {
				size_t enumValue = 0;
				// Generate enum constructors.
				for (const auto& constructorName: *(astTypeInstanceNode->constructors)) {
					const auto canonicalMethodName = CanonicalizeMethodName(constructorName);
					CreateEnumConstructorMethod(context, &semTypeInstance,
						*(semTypeInstance.functions().at(canonicalMethodName)), enumValue++);
				}
			}
			
			// Generate default constructor for applicable types.
			if (semTypeInstance.isException()) {
				CreateExceptionConstructor(context, astTypeInstanceNode, &semTypeInstance,
				                           semTypeInstance.functions().at(context.getCString("create")).get());
			} else if (semTypeInstance.isDatatype() || semTypeInstance.isStruct() || semTypeInstance.isUnion()) {
				(void) CreateDefaultMethod(context, &semTypeInstance,
				                           semTypeInstance.functions().at(context.getCString("create")).get(),
				                           astTypeInstanceNode.location());
			}
			
			// Generate default implicitCopy if relevant.
			if (semTypeInstance.isEnum() || semTypeInstance.isStruct() || semTypeInstance.isDatatype() ||
					semTypeInstance.isUnionDatatype() || semTypeInstance.isUnion()) {
				const auto iterator = semTypeInstance.functions().find(context.getCString("implicitcopy"));
				if (iterator != semTypeInstance.functions().end()) {
					CreateDefaultMethodOrRemove(context, &semTypeInstance, iterator->second.get(),
					                            astTypeInstanceNode.location());
				}
			}
			
			// Generate default compare if relevant.
			if (semTypeInstance.isEnum() || semTypeInstance.isStruct() || semTypeInstance.isDatatype() || semTypeInstance.isUnionDatatype()) {
				const auto iterator = semTypeInstance.functions().find(context.getCString("compare"));
				if (iterator != semTypeInstance.functions().end()) {
					CreateDefaultMethodOrRemove(context, &semTypeInstance,
					                            iterator->second.get(),
					                            astTypeInstanceNode.location());
				}
			}
			
			// Simplify all predicates to avoid confusing CodeGen.
			for (auto& functionPair: semTypeInstance.functions()) {
				auto& function = *(functionPair.second);
				PushScopeElement pushFunction(context.scopeStack(), ScopeElement::Function(function));
				function.setConstPredicate(reducePredicate(context, function.constPredicate().copy()));
				function.setRequiresPredicate(reducePredicate(context, function.requiresPredicate().copy()));
				
				// Simplify function type noexcept predicate.
				const auto oldFunctionType = function.type();
				
				const bool isVarArg = oldFunctionType.attributes().isVarArg();
				const bool isMethod = oldFunctionType.attributes().isMethod();
				const bool isTemplated = oldFunctionType.attributes().isTemplated();
				auto noExceptPredicate = reducePredicate(context, oldFunctionType.attributes().noExceptPredicate().copy());
				const auto returnType = oldFunctionType.returnType();
				const auto& argTypes = oldFunctionType.parameterTypes();
				
				SEM::FunctionAttributes attributes(isVarArg, isMethod, isTemplated, std::move(noExceptPredicate));
				function.setType(SEM::FunctionType(std::move(attributes), returnType, argTypes.copy()));
			}
		}
		
	}
	
}

