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
	
		void CreateEnumConstructorMethod(Context& context,
		                                 const AST::Node<AST::TypeInstance>& typeInstanceNode,
		                                 AST::Function& function, const size_t value) {
			assert(typeInstanceNode->isEnum());
			
			if (function.hasScope()) {
				// Function already has a scope; this can happen when
				// the user has given duplicate enum constructors, in
				// which case we will have issued an error but continued
				// onwards to find more errors.
				return;
			}
			
			auto functionScope = AST::Scope::Create(typeInstanceNode.location());
			
			HeapArray<AST::Value> constructValues;
			
			const auto intConstant = Constant::Integer(APInt(value));
			const auto intType = getBuiltInType(context, context.getCString("int_t"), {});
			constructValues.push_back(AST::Value::Constant(intConstant, intType));
			
			auto internalConstructedValue = AST::Value::InternalConstruct(typeInstanceNode->selfType(), std::move(constructValues));
			functionScope->statements().push_back(SEM::Statement::Return(std::move(internalConstructedValue)));
			
			function.setScope(std::move(functionScope));
		}
		
		void CreateDefaultMethodOrRemove(Context& context, AST::TypeInstance& typeInstance,
		                                 AST::Function& function, const Debug::SourceLocation& location) {
			const bool created = DefaultMethods(context).createDefaultMethod(&typeInstance, function,
			                                                                 location);
			if (!created) {
				// Make sure that the require predicate is false so
				// CodeGen understands not to generate this function.
				function.setRequiresPredicate(AST::Predicate::False());
			}
		}
		
		void ConvertTypeInstance(Context& context, AST::Node<AST::TypeInstance>& typeInstanceNode) {
			for (const auto& function: *(typeInstanceNode->functionDecls)) {
				PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(*function));
				ConvertFunctionDef(context, function);
			}
			
			if (typeInstanceNode->isEnum()) {
				size_t enumValue = 0;
				// Generate enum constructors.
				for (const auto& constructorName: *(typeInstanceNode->constructors)) {
					const auto canonicalMethodName = CanonicalizeMethodName(constructorName);
					CreateEnumConstructorMethod(context, typeInstanceNode,
						typeInstanceNode->getFunction(canonicalMethodName), enumValue++);
				}
			}
			
			// Generate default constructor for applicable types.
			if (typeInstanceNode->isException()) {
				CreateExceptionConstructor(context, typeInstanceNode,
				                           typeInstanceNode->getFunction(context.getCString("create")));
			} else if (typeInstanceNode->isDatatype() || typeInstanceNode->isStruct() || typeInstanceNode->isUnion()) {
				(void) DefaultMethods(context).createDefaultMethod(typeInstanceNode.get(),
				                                                   typeInstanceNode->getFunction(context.getCString("create")),
				                                                   typeInstanceNode.location());
			}
			
			// Generate default implicitCopy if relevant.
			if (typeInstanceNode->isEnum() || typeInstanceNode->isStruct() || typeInstanceNode->isDatatype() ||
					typeInstanceNode->isUnionDatatype() || typeInstanceNode->isUnion()) {
				const auto existingFunction = typeInstanceNode->findFunction(context.getCString("implicitcopy"));
				if (existingFunction != nullptr) {
					CreateDefaultMethodOrRemove(context, *typeInstanceNode, *existingFunction,
					                            typeInstanceNode.location());
				}
			}
			
			// Generate default compare if relevant.
			if (typeInstanceNode->isEnum() || typeInstanceNode->isStruct() || typeInstanceNode->isDatatype() || typeInstanceNode->isUnionDatatype()) {
				const auto existingFunction = typeInstanceNode->findFunction(context.getCString("compare"));
				if (existingFunction != nullptr) {
					CreateDefaultMethodOrRemove(context, *typeInstanceNode, *existingFunction,
					                            typeInstanceNode.location());
				}
			}
			
			// Simplify all predicates to avoid confusing CodeGen.
			for (auto& function: typeInstanceNode->functions()) {
				PushScopeElement pushFunction(context.scopeStack(), ScopeElement::Function(*function));
				function->setConstPredicate(reducePredicate(context, function->constPredicate().copy()));
				function->setRequiresPredicate(reducePredicate(context, function->requiresPredicate().copy()));
				
				// Simplify function type noexcept predicate.
				const auto oldFunctionType = function->type();
				
				const bool isVarArg = oldFunctionType.attributes().isVarArg();
				const bool isMethod = oldFunctionType.attributes().isMethod();
				const bool isTemplated = oldFunctionType.attributes().isTemplated();
				auto noExceptPredicate = reducePredicate(context, oldFunctionType.attributes().noExceptPredicate().copy());
				const auto returnType = oldFunctionType.returnType();
				const auto& argTypes = oldFunctionType.parameterTypes();
				
				AST::FunctionAttributes attributes(isVarArg, isMethod, isTemplated, std::move(noExceptPredicate));
				function->setType(AST::FunctionType(std::move(attributes), returnType, argTypes.copy()));
			}
		}
		
	}
	
}

