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
#include <locic/SemanticAnalysis/Node.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		namespace {
			
			std::vector<SEM::Type*> getFilteredConstructTypes(const std::vector<SEM::Var*>& variables) {
				std::vector<SEM::Type*> types;
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
			
			std::vector<SEM::Var*> getParameters(Context& context, const std::vector<SEM::Type*>& constructTypes) {
				std::vector<SEM::Var*> parameters;
				for (const auto varType: constructTypes) {
					const bool isLvalConst = false;
					const auto lvalType = makeValueLvalType(context, isLvalConst, varType);
					parameters.push_back(SEM::Var::Basic(varType, lvalType));
				}
				return parameters;
			}
			
			void attachParameters(Node& functionNode, const AST::Node<AST::TypeVarList>& astParametersNode, const std::vector<SEM::Var*>& semParameters) {
				assert(functionNode.isFunction());
				assert(astParametersNode->size() == semParameters.size());
				
				for (size_t i = 0; i < astParametersNode->size(); i++) {
					const auto& astTypeVarNode = astParametersNode->at(i);
					const auto& semVar = semParameters.at(i);
					assert(astTypeVarNode->kind == AST::TypeVar::NAMEDVAR);
					
					const Node paramNode = Node::Variable(astTypeVarNode, semVar);
					if (!functionNode.tryAttach(astTypeVarNode->namedVar.name, paramNode)) {
						throw ParamVariableClashException(functionNode.getSEMFunction()->name(), astTypeVarNode->namedVar.name);
					}
				}
			}
			
		}
		
		SEM::Function* CreateExceptionConstructorDecl(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode, SEM::TypeInstance* semTypeInstance) {
			const auto& initializerNode = astTypeInstanceNode->initializer;
			
			const bool isVarArg = false;
			const bool isStatic = true;
			const bool isMethod = true;
			const bool isConst = false;
			
			const bool hasParent = (initializerNode->kind == AST::ExceptionInitializer::INITIALIZE);
			if (!hasParent) {
				// No parent, so just create a normal default constructor.
				return CreateDefaultConstructorDecl(context, semTypeInstance);
			}
			
			// Filter out first variable from construct types
			// since the first variable will store the parent.
			const auto constructTypes = getFilteredConstructTypes(semTypeInstance->variables());
			
			const auto functionType = SEM::Type::Function(isVarArg, semTypeInstance->selfType(), constructTypes);
			const auto parameters = getParameters(context, constructTypes);
			return SEM::Function::Decl(isMethod, isStatic, isConst, functionType, semTypeInstance->name() + "Create", parameters);
		}
		
		void CreateExceptionConstructor(Context& context, SEM::Function* function) {
			const auto& node = context.node();
			assert(node.isFunction());
			
			const auto parentNode = lookupParentType(context);
			assert(parentNode.isTypeInstance());
			
			const auto& astTypeInstanceNode = parentNode.getASTTypeInstance();
			const auto semTypeInstance = parentNode.getSEMTypeInstance();
			
			assert(semTypeInstance->isException());
			
			const auto& initializerNode = astTypeInstanceNode->initializer;
			const auto& location = astTypeInstanceNode.location();
			
			const bool hasParent = (initializerNode->kind == AST::ExceptionInitializer::INITIALIZE);
			
			if (!hasParent) {
				assert(semTypeInstance->parent() == nullptr);
				
				// No parent, so just create a normal default constructor.
				CreateDefaultConstructor(semTypeInstance, function);
				return;
			}
			
			assert(semTypeInstance->parent() != nullptr);
			
			// Create node for function.
			auto functionNode = Node::Function(AST::Node<AST::Function>(), function);
			
			// Attach parameters to the function node.
			attachParameters(functionNode, astTypeInstanceNode->variables, function->parameters());
			
			// Create context for function (to resolve references to parameters).
			NodeContext functionContext(context, "Create", functionNode);
			
			std::vector<SEM::Value*> parentArguments;
			for (const auto& astValueNode: *(initializerNode->valueList)) {
				parentArguments.push_back(ConvertValue(functionContext, astValueNode));
			}
			
			std::vector<SEM::Value*> constructValues;
			
			// Call parent constructor.
			// TODO: should provide template arguments.
			const auto parentType = SEM::Type::Object(semTypeInstance->parent(), SEM::Type::NO_TEMPLATE_ARGS);
			constructValues.push_back(CallValue(GetStaticMethod(parentType, "Create", location), parentArguments, location));
			
			for (const auto semVar: function->parameters()) {
				const auto varValue = SEM::Value::LocalVar(semVar);
				
				// Move from each value_lval into the internal constructor.
				constructValues.push_back(CallValue(GetMethod(varValue, "move", location), {}, location));
			}
			
			const auto returnValue = SEM::Value::InternalConstruct(semTypeInstance, constructValues);
			
			const auto scope = new SEM::Scope();
			scope->statements().push_back(SEM::Statement::Return(returnValue));
			function->setScope(scope);
		}
		
	}
	
}

