#include <assert.h>
#include <locic/AST.hpp>
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
			
			SEM::TypeArray getFilteredConstructTypes(const std::vector<AST::Var*>& variables) {
				assert(!variables.empty());
				SEM::TypeArray types;
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
			
			void attachParameters(SEM::Function& function, const std::vector<AST::Var*>& parameters) {
				for (size_t i = 0; i < parameters.size(); i++) {
					const auto& var = parameters.at(i);
					assert(var->isNamed());
					
					const auto& varName = var->name();
					
					// It doesn't matter if this fails to insert the variable as we
					// will have already issued an error to the user.
					(void) function.namedVariables().insert(std::make_pair(varName, var));
				}
			}
			
		}
		
		std::unique_ptr<SEM::Function> CreateExceptionConstructorDecl(Context& context, SEM::TypeInstance* const semTypeInstance) {
			if (semTypeInstance->parentType() == nullptr) {
				// No parent, so just create a normal default constructor.
				return DefaultMethods(context).createDefaultConstructorDecl(semTypeInstance,
				                                                            semTypeInstance->name() + context.getCString("create"));
			}
			
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(SEM::GlobalStructure::TypeInstance(*semTypeInstance),
			                                                             semTypeInstance->name() + context.getCString("create"),
			                                                             semTypeInstance->moduleScope().copy()));
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*semTypeInstance, *semFunction));
			
			semFunction->setRequiresPredicate(semTypeInstance->requiresPredicate().copy());
			
			semFunction->setMethod(true);
			semFunction->setStaticMethod(true);
			
			const bool isVarArg = false;
			const bool isDynamicMethod = false;
			const bool isTemplatedMethod = !semTypeInstance->templateVariables().empty();
			auto noExceptPredicate = SEM::Predicate::False();
			
			// Filter out first variable from construct types
			// since the first variable will store the parent.
			auto constructTypes = getFilteredConstructTypes(semTypeInstance->variables());
			semFunction->setParameters(getParameters(semTypeInstance->variables()));
			
			SEM::FunctionAttributes attributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate));
			semFunction->setType(SEM::FunctionType(std::move(attributes), semTypeInstance->selfType(), std::move(constructTypes)));
			return semFunction;
		}
		
		void CreateExceptionConstructor(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode, SEM::TypeInstance* semTypeInstance, SEM::Function* function) {
			assert(semTypeInstance->isException());
			
			const auto& initializerNode = astTypeInstanceNode->initializer;
			const auto& location = astTypeInstanceNode.location();
			
			const bool hasParent = (initializerNode->kind == AST::ExceptionInitializer::INITIALIZE);
			
			if (!hasParent) {
				assert(semTypeInstance->parentType() == nullptr);
				
				// No parent, so just create a normal default constructor.
				DefaultMethods(context).createDefaultConstructor(semTypeInstance,
				                                                 function,
				                                                 location);
				return;
			}
			
			assert(semTypeInstance->parentType() != nullptr);
			
			// Attach parameters to the function.
			attachParameters(*function, function->parameters());
			
			// Push function on to scope stack (to resolve references to parameters).
			PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(*function));
			
			HeapArray<SEM::Value> parentArguments;
			for (const auto& astValueNode: *(initializerNode->valueList)) {
				parentArguments.push_back(ConvertValue(context, astValueNode));
			}
			
			HeapArray<SEM::Value> constructValues;
			constructValues.reserve(1 + function->parameters().size());
			
			// Call parent constructor.
			auto typeRefValue = createTypeRef(context, semTypeInstance->parentType());
			constructValues.push_back(CallValue(context, GetStaticMethod(context, std::move(typeRefValue), context.getCString("create"), location), std::move(parentArguments), location));
			
			for (const auto var: function->parameters()) {
				const auto varType = getBuiltInType(context, context.getCString("ref_t"), { var->lvalType() })->createRefType(var->lvalType());
				auto varValue = SEM::Value::LocalVar(*var, varType);
				
				// Move from each value_lval into the internal constructor.
				constructValues.push_back(CallValue(context, GetSpecialMethod(context, derefValue(std::move(varValue)), context.getCString("move"), location), {}, location));
			}
			
			auto returnValue = SEM::Value::InternalConstruct(semTypeInstance->selfType(), std::move(constructValues));
			
			std::unique_ptr<SEM::Scope> scope(new SEM::Scope());
			scope->statements().push_back(SEM::Statement::Return(std::move(returnValue)));
			function->setScope(std::move(scope));
		}
		
	}
	
}

