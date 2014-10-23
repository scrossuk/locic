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
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		namespace {
			
			std::vector<const SEM::Type*> getFilteredConstructTypes(const std::vector<SEM::Var*>& variables) {
				std::vector<const SEM::Type*> types;
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
			
			std::vector<SEM::Var*> getParameters(Context& context, const std::vector<const SEM::Type*>& constructTypes) {
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
						throw ParamVariableClashException(function->name(), varName);
					}
				}
			}
			
		}
		
		SEM::Function* CreateExceptionConstructorDecl(Context& context, SEM::TypeInstance* semTypeInstance) {
			if (semTypeInstance->parent() == nullptr) {
				// No parent, so just create a normal default constructor.
				return CreateDefaultConstructorDecl(context, semTypeInstance, semTypeInstance->name() + "create");
			}
			
			const auto semFunction = new SEM::Function(semTypeInstance->name() + "create", semTypeInstance->moduleScope());
			
			semFunction->setMethod(true);
			semFunction->setStaticMethod(true);
			
			const bool isVarArg = false;
			const bool isDynamicMethod = false;
			const bool isTemplatedMethod = !semTypeInstance->templateVariables().empty();
			const bool isNoExcept = false;
			
			// Filter out first variable from construct types
			// since the first variable will store the parent.
			const auto constructTypes = getFilteredConstructTypes(semTypeInstance->variables());
			
			semFunction->setType(SEM::Type::Function(isVarArg, isDynamicMethod, isTemplatedMethod, isNoExcept, semTypeInstance->selfType(), constructTypes));
			semFunction->setParameters(getParameters(context, constructTypes));
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
			
			std::vector<SEM::Value*> parentArguments;
			for (const auto& astValueNode: *(initializerNode->valueList)) {
				parentArguments.push_back(ConvertValue(context, astValueNode));
			}
			
			std::vector<SEM::Value*> constructValues;
			
			// Call parent constructor.
			const auto typeRefValue = createTypeRef(context, semTypeInstance->parent());
			constructValues.push_back(CallValue(context, GetStaticMethod(context, typeRefValue, "create", location), parentArguments, location));
			
			for (const auto semVar: function->parameters()) {
				const auto referenceTypeInst = getBuiltInType(context.scopeStack(), "__ref")->getObjectType();
				const auto varType = SEM::Type::Object(referenceTypeInst, { semVar->type() })->createRefType(semVar->type());
				const auto varValue = SEM::Value::LocalVar(semVar, varType);
				
				// Move from each value_lval into the internal constructor.
				constructValues.push_back(CallValue(context, GetSpecialMethod(context, derefValue(varValue), "move", location), {}, location));
			}
			
			const auto returnValue = SEM::Value::InternalConstruct(semTypeInstance, constructValues);
			
			std::unique_ptr<SEM::Scope> scope(new SEM::Scope());
			scope->statements().push_back(SEM::Statement::Return(returnValue));
			function->setScope(std::move(scope));
		}
		
	}
	
}

