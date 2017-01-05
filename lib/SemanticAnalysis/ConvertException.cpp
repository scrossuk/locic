#include <assert.h>
#include <locic/AST.hpp>
#include <locic/AST/Type.hpp>
#include <locic/Support/Name.hpp>

#include <locic/SemanticAnalysis/CallValue.hpp>
#include <locic/SemanticAnalysis/Cast.hpp>
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
					types.push_back(var->type());
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
			function->setAutoGenerated(true);
			
			const bool isVarArg = false;
			const bool isDynamicMethod = false;
			const bool isTemplatedMethod = !typeInstance.templateVariables().empty();
			auto noExceptPredicate = AST::Predicate::False();
			
			// Filter out first variable from construct types
			// since the first variable will store the parent.
			auto constructTypes = getFilteredConstructTypes(typeInstance.variables());
			function->setParameters(getParameters(typeInstance.variables()));
			
			AST::FunctionAttributes attributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate));
			function->setType(AST::FunctionType(std::move(attributes), typeInstance.selfType(), std::move(constructTypes)));
			return function;
		}
		
		class ParentExceptionIncorrectArgCountDiag: public Error {
		public:
			ParentExceptionIncorrectArgCountDiag(const AST::Type* const parentType,
			                                     size_t argsGiven, size_t argsRequired)
			: parentType_(parentType), argsGiven_(argsGiven),
			argsRequired_(argsRequired) { }
			
			std::string toString() const {
				return makeString("exception parent '%s' called with %llu "
				                  "parameter(s); expected %llu",
				                  parentType_->toDiagString().c_str(),
				                  static_cast<unsigned long long>(argsGiven_),
				                  static_cast<unsigned long long>(argsRequired_));
			}
			
		private:
			const AST::Type* parentType_;
			size_t argsGiven_;
			size_t argsRequired_;
			
		};
		
		void CreateExceptionConstructor(Context& context, AST::Node<AST::TypeInstance>& typeInstanceNode,
		                                AST::Function& function) {
			assert(typeInstanceNode->isException());
			
			const auto& initializerNode = typeInstanceNode->initializer;
			const auto& location = typeInstanceNode.location();
			
			const bool hasParent = (initializerNode->kind == AST::ExceptionInitializer::INITIALIZE);
			const auto parentType = typeInstanceNode->parentType();
			
			if (!hasParent) {
				assert(parentType == nullptr);
				
				// No parent, so just create a normal default constructor.
				DefaultMethods(context).createDefaultConstructor(typeInstanceNode.get(),
				                                                 function,
				                                                 location);
				return;
			}
			
			assert(parentType != nullptr);
			
			const auto& parentTypeInstance = *(parentType->getObjectType());
			
			// Attach parameters to the function.
			attachParameters(function);
			
			// Push function on to scope stack (to resolve references to parameters).
			PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(function));
			
			AST::ValueArray initializerValues;
			initializerValues.reserve(initializerNode->valueList->size());
			
			// If the parent type has its own parent, we need to skip its first
			// variable (which will have been created for its parent type).
			size_t i = parentTypeInstance.parentType() != nullptr ? 1 : 0;
			
			const auto givenCount = initializerNode->valueList->size();
			const auto expectedCount = parentTypeInstance.variables().size() - i;
			if (givenCount != expectedCount) {
				context.issueDiag(ParentExceptionIncorrectArgCountDiag(parentType,
				                                                       givenCount,
				                                                       expectedCount),
				                  initializerNode->valueList.location());
			}
			
			const auto parentTemplateMap = parentType->generateTemplateVarMap();
			
			for (const auto& astValueNode: *(initializerNode->valueList)) {
				auto value = ConvertValue(context, astValueNode);
				if (i < parentTypeInstance.variables().size()) {
					// If there is a corresponding variable, then try to
					// implicitly cast for it; if there isn't a variable this is
					// an error that we reported above, so we fail silently here.
					const auto var = parentTypeInstance.variables()[i];
					const auto varType = var->type()->substitute(parentTemplateMap);
					value = ImplicitCast(context, std::move(value), varType,
					                     astValueNode.location());
				}
				initializerValues.push_back(std::move(value));
				i++;
			}
			
			typeInstanceNode->setInitializerValues(std::move(initializerValues));
		}
		
	}
	
}

