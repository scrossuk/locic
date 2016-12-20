#include <assert.h>
#include <locic/AST.hpp>
#include <locic/AST/Type.hpp>
#include <locic/Support/Name.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/CallValue.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertException.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/GetMethod.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		namespace {
			
			AST::TypeArray getFilteredConstructTypes(const std::vector<AST::Var*>& variables) {
				assert(!variables.empty());
				AST::TypeArray types;
				types.reserve(variables.size() - 1);
				bool isFirst = true;
				for (const auto var: variables) {
					if (isFirst) {
						isFirst = false;
						continue;
					}
					types.push_back(var->constructType());
				}
				return types;
			}
			
			std::vector<AST::Var*> getParameters(const std::vector<AST::Var*>& variables) {
				assert(!variables.empty());
				std::vector<AST::Var*> parameters;
				parameters.reserve(variables.size() - 1);
				bool isFirst = true;
				for (const auto var: variables) {
					if (isFirst) {
						isFirst = false;
						continue;
					}
					parameters.push_back(var);
				}
				return parameters;
			}
			
			void attachParameters(AST::Function& function) {
				for (size_t i = 0; i < function.parameters().size(); i++) {
					const auto& var = function.parameters().at(i);
					assert(var->isNamed());
					
					const auto& varName = var->name();
					
					// It doesn't matter if this fails to insert the variable as we
					// will have already issued an error to the user.
					(void) function.namedVariables().insert(std::make_pair(varName, var));
				}
			}
			
		}
		
		std::unique_ptr<AST::Function>
		CreateExceptionConstructorDecl(Context& context, AST::TypeInstance& typeInstance) {
			if (typeInstance.parentType() == nullptr) {
				// No parent, so just create a normal default constructor.
				return DefaultMethods(context).createDefaultConstructorDecl(&typeInstance,
				                                                            typeInstance.fullName() + context.getCString("create"));
			}
			
			std::unique_ptr<AST::Function> function(new AST::Function());
			function->setParent(AST::GlobalStructure::TypeInstance(typeInstance));
			function->setFullName(typeInstance.fullName() + context.getCString("create"));
			function->setModuleScope(typeInstance.moduleScope().copy());
			
			function->setDebugInfo(makeDefaultFunctionInfo(typeInstance, *function));
			
			function->setRequiresPredicate(typeInstance.requiresPredicate().copy());
			
			function->setMethod(true);
			function->setIsStatic(true);
			
			const bool isVarArg = false;
			const bool isDynamicMethod = false;
			const bool isTemplatedMethod = !typeInstance.templateVariables().empty();
			auto noExceptPredicate = SEM::Predicate::False();
			
			// Filter out first variable from construct types
			// since the first variable will store the parent.
			auto constructTypes = getFilteredConstructTypes(typeInstance.variables());
			function->setParameters(getParameters(typeInstance.variables()));
			
			AST::FunctionAttributes attributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate));
			function->setType(AST::FunctionType(std::move(attributes), typeInstance.selfType(), std::move(constructTypes)));
			return function;
		}
		
		void CreateExceptionConstructor(Context& context, AST::Node<AST::TypeInstance>& typeInstanceNode,
		                                AST::Function& function) {
			assert(typeInstanceNode->isException());
			
			const auto& initializerNode = typeInstanceNode->initializer;
			const auto& location = typeInstanceNode.location();
			
			const bool hasParent = (initializerNode->kind == AST::ExceptionInitializer::INITIALIZE);
			
			if (!hasParent) {
				assert(typeInstanceNode->parentType() == nullptr);
				
				// No parent, so just create a normal default constructor.
				DefaultMethods(context).createDefaultConstructor(typeInstanceNode.get(),
				                                                 function,
				                                                 location);
				return;
			}
			
			assert(typeInstanceNode->parentType() != nullptr);
			
			// Attach parameters to the function.
			attachParameters(function);
			
			// Push function on to scope stack (to resolve references to parameters).
			PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(function));
			
			HeapArray<AST::Value> parentArguments;
			for (const auto& astValueNode: *(initializerNode->valueList)) {
				parentArguments.push_back(ConvertValue(context, astValueNode));
			}
			
			HeapArray<AST::Value> constructValues;
			constructValues.reserve(1 + function.parameters().size());
			
			// Call parent constructor.
			auto typeRefValue = createTypeRef(context, typeInstanceNode->parentType());
			constructValues.push_back(CallValue(context, GetStaticMethod(context, std::move(typeRefValue), context.getCString("create"), location), std::move(parentArguments), location));
			
			for (const auto var: function.parameters()) {
				const auto varType = getBuiltInType(context, context.getCString("ref_t"), { var->lvalType() })->createRefType(var->lvalType());
				auto varValue = AST::Value::LocalVar(*var, varType);
				
				// Move from each value_lval into the internal constructor.
				constructValues.push_back(CallValue(context, GetSpecialMethod(context, derefValue(std::move(varValue)), context.getCString("move"), location), {}, location));
			}
			
			auto returnValue = AST::Value::InternalConstruct(typeInstanceNode->selfType(), std::move(constructValues));
			
			auto scope = AST::Scope::Create(location);
			scope->statements().push_back(SEM::Statement::Return(std::move(returnValue)));
			function.setScope(std::move(scope));
		}
		
	}
	
}

