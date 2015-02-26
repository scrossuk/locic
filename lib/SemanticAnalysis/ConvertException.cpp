#include <assert.h>
#include <locic/AST.hpp>
#include <locic/Name.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertException.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		namespace {
			
			SEM::TypeArray getFilteredConstructTypes(const std::vector<SEM::Var*>& variables) {
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
			
			std::vector<SEM::Var*> getParameters(Context& context, const SEM::TypeArray& constructTypes) {
				std::vector<SEM::Var*> parameters;
				for (const auto varType: constructTypes) {
					const bool isLvalConst = false;
					const auto lvalType = makeValueLvalType(context, isLvalConst, varType);
					parameters.push_back(SEM::Var::Basic(varType, lvalType));
				}
				return parameters;
			}
			
			void attachParameters(SEM::Function* function, const AST::Node<AST::TypeVarList>& astParametersNode, const std::vector<SEM::Var*>& semParameters) {
				assert(astParametersNode->size() == semParameters.size());
				
				for (size_t i = 0; i < astParametersNode->size(); i++) {
					const auto& astTypeVarNode = astParametersNode->at(i);
					const auto& semVar = semParameters.at(i);
					assert(astTypeVarNode->kind == AST::TypeVar::NAMEDVAR);
					
					const auto& varName = astTypeVarNode->namedVar.name;
					
					const auto insertResult = function->namedVariables().insert(std::make_pair(varName, semVar));
					if (!insertResult.second) {
						throw ParamVariableClashException(function->name().copy(), varName);
					}
				}
			}
			
		}
		
		SEM::Function* CreateExceptionConstructorDecl(Context& context, SEM::TypeInstance* const semTypeInstance) {
			if (semTypeInstance->parent() == nullptr) {
				// No parent, so just create a normal default constructor.
				return CreateDefaultConstructorDecl(context, semTypeInstance, semTypeInstance->name() + context.getCString("create"));
			}
			
			const auto semFunction = new SEM::Function(semTypeInstance->name() + context.getCString("create"), semTypeInstance->moduleScope().copy());
			semFunction->setRequiresPredicate(semTypeInstance->requiresPredicate().copy());
			
			semFunction->setMethod(true);
			semFunction->setStaticMethod(true);
			
			const bool isVarArg = false;
			const bool isDynamicMethod = false;
			const bool isTemplatedMethod = !semTypeInstance->templateVariables().empty();
			const bool isNoExcept = false;
			
			// Filter out first variable from construct types
			// since the first variable will store the parent.
			auto constructTypes = getFilteredConstructTypes(semTypeInstance->variables());
			semFunction->setParameters(getParameters(context, constructTypes));
			semFunction->setType(SEM::Type::Function(isVarArg, isDynamicMethod, isTemplatedMethod, isNoExcept, semTypeInstance->selfType(), std::move(constructTypes)));
			return semFunction;
		}
		
		void CreateExceptionConstructor(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode, SEM::TypeInstance* semTypeInstance, SEM::Function* function) {
			assert(semTypeInstance->isException());
			
			const auto& initializerNode = astTypeInstanceNode->initializer;
			const auto& location = astTypeInstanceNode.location();
			
			const bool hasParent = (initializerNode->kind == AST::ExceptionInitializer::INITIALIZE);
			
			if (!hasParent) {
				assert(semTypeInstance->parent() == nullptr);
				
				// No parent, so just create a normal default constructor.
				CreateDefaultConstructor(context, semTypeInstance, function, location);
				return;
			}
			
			assert(semTypeInstance->parent() != nullptr);
			
			// Attach parameters to the function.
			attachParameters(function, astTypeInstanceNode->variables, function->parameters());
			
			// Push function on to scope stack (to resolve references to parameters).
			PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(function));
			
			std::vector<SEM::Value> parentArguments;
			for (const auto& astValueNode: *(initializerNode->valueList)) {
				parentArguments.push_back(ConvertValue(context, astValueNode));
			}
			
			std::vector<SEM::Value> constructValues;
			
			// Call parent constructor.
			auto typeRefValue = createTypeRef(context, semTypeInstance->parent());
			constructValues.push_back(CallValue(context, GetStaticMethod(context, std::move(typeRefValue), context.getCString("create"), location), std::move(parentArguments), location));
			
			for (const auto semVar: function->parameters()) {
				const auto varType = getBuiltInType(context.scopeStack(), context.getCString("__ref"), { semVar->type() })->createRefType(semVar->type());
				auto varValue = SEM::Value::LocalVar(semVar, varType);
				
				// Move from each value_lval into the internal constructor.
				constructValues.push_back(CallValue(context, GetSpecialMethod(context, derefValue(std::move(varValue)), context.getCString("move"), location), {}, location));
			}
			
			auto returnValue = SEM::Value::InternalConstruct(semTypeInstance, std::move(constructValues));
			
			std::unique_ptr<SEM::Scope> scope(new SEM::Scope());
			scope->statements().push_back(SEM::Statement::Return(std::move(returnValue)));
			function->setScope(std::move(scope));
		}
		
	}
	
}

