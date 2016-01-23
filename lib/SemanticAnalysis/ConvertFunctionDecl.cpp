#include <cassert>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ConvertVar.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/Template.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		SEM::GlobalStructure getParent(const ScopeElement& topElement) {
			assert(topElement.isNamespace() || topElement.isTypeInstance());
			if (topElement.isNamespace()) {
				return SEM::GlobalStructure::Namespace(topElement.nameSpace());
			} else {
				return SEM::GlobalStructure::TypeInstance(topElement.typeInstance());
			}
		}
		
		std::unique_ptr<SEM::Function> ConvertFunctionDecl(Context& context, const AST::Node<AST::Function>& astFunctionNode, SEM::ModuleScope moduleScope) {
			const auto thisTypeInstance = lookupParentType(context.scopeStack());
			
			const auto& name = astFunctionNode->name()->last();
			
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(getParent(context.scopeStack().back()),
			                                                             astFunctionNode,
			                                                             std::move(moduleScope)));
			
			const bool isMethod = thisTypeInstance != nullptr;
			
			if (!isMethod && !astFunctionNode->constSpecifier()->isNone()) {
				throw ErrorException(makeString("Non-method function '%s' cannot have const specifier, at location %s.",
						name.c_str(), astFunctionNode.location().toString().c_str()));
			}
			
			if (!isMethod && astFunctionNode->isStatic()) {
				throw ErrorException(makeString("Non-method function '%s' cannot be static, at location %s.",
						name.c_str(), astFunctionNode.location().toString().c_str()));
			}
			
			// Don't treat extension methods as primitive methods.
			const bool isPrimitiveMethod = isMethod && thisTypeInstance->isPrimitive() && astFunctionNode->name()->size() == 1;
			const bool isPrimitiveFunction = astFunctionNode->isPrimitive();
			
			semFunction->setPrimitive(isPrimitiveMethod || isPrimitiveFunction);
			
			semFunction->setMethod(isMethod);
			semFunction->setStaticMethod(astFunctionNode->isStatic());
			
			if (!astFunctionNode->templateVariables()->empty() && (thisTypeInstance != nullptr && thisTypeInstance->isInterface())) {
				throw ErrorException(makeString("Interface '%s' has templated method '%s' (interfaces may only contain non-templated methods), at location %s.",
						thisTypeInstance->name().toString().c_str(), name.c_str(),
						astFunctionNode.location().toString().c_str()));
			}
			
			// Add template variables.
			size_t templateVarIndex = (thisTypeInstance != nullptr) ? thisTypeInstance->templateVariables().size() : 0;
			for (const auto& astTemplateVarNode: *(astFunctionNode->templateVariables())) {
				const auto& templateVarName = astTemplateVarNode->name;
				
				// TODO!
				const bool isVirtual = false;
				const auto semTemplateVar =
					new SEM::TemplateVar(context.semContext(),
						semFunction->name() + templateVarName,
						templateVarIndex++, isVirtual);
				
				const auto templateVarIterator = semFunction->namedTemplateVariables().find(templateVarName);
				if (templateVarIterator != semFunction->namedTemplateVariables().end()) {
					throw ErrorException(makeString("More than one template variable shares name '%s' in function '%s', at location %s.",
						templateVarName.c_str(), semFunction->name().toString().c_str(),
						astTemplateVarNode.location().toString().c_str()));
				}
				
				// Also adding the function template variable type here.
				const auto& astVarType = astTemplateVarNode->varType;
				const auto semVarType = ConvertType(context, astVarType);
				
				if (!semVarType->isPrimitive()) {
					throw ErrorException(makeString("Template variable '%s' in function '%s' has non-primitive type '%s', at position %s.",
						templateVarName.c_str(), semFunction->name().toString().c_str(),
						semVarType->toString().c_str(),
						astTemplateVarNode.location().toString().c_str()));
				}
				
				semTemplateVar->setType(semVarType);
				
				semFunction->templateVariables().push_back(semTemplateVar);
				semFunction->namedTemplateVariables().insert(std::make_pair(templateVarName, semTemplateVar));
			}
			
			assert(semFunction->isDeclaration());
			
			return semFunction;
		}
		
		bool isValidReturnType(const String& name,
		                       const SEM::Type* const type) {
			if (name == "__moveto") {
				return type->isBuiltInVoid();
			} else if (name == "__destroy") {
				return type->isBuiltInVoid();
			} else if (name == "__alignmask") {
				return type->isPrimitive() &&
				       type->primitiveID() == PrimitiveSize;
			} else if (name == "__sizeof") {
				return type->isPrimitive() &&
				       type->primitiveID() == PrimitiveSize;
			} else if (name == "__islive") {
				return type->isBuiltInBool();
			} else if (name == "__setdead") {
				return type->isBuiltInVoid();
			} else if (name == "__dead") {
				// TODO
				return true;
			} else if (name == "__isvalid") {
				return type->isBuiltInBool();
			} else if (name == "__setinvalid") {
				return type->isBuiltInVoid();
			} else {
				throw std::logic_error("Unknown lifetime method.");
			}
		}
		
		bool isValidArgumentCount(const String& name,
		                          const size_t argCount) {
			if (name == "__moveto") {
				return argCount == 2;
			} else if (name == "__destroy") {
				return argCount == 0;
			} else if (name == "__alignmask") {
				return argCount == 0;
			} else if (name == "__sizeof") {
				return argCount == 0;
			} else if (name == "__islive") {
				return argCount == 0;
			} else if (name == "__setdead") {
				return argCount == 0;
			} else if (name == "__dead") {
				return argCount == 0;
			} else if (name == "__isvalid") {
				return argCount == 0;
			} else if (name == "__setinvalid") {
				return argCount == 0;
			} else {
				throw std::logic_error("Unknown lifetime method.");
			}
		}
		
		bool isValidArgumentTypes(const String& name,
		                          const SEM::TypeArray& types) {
			if (name == "__moveto") {
				return types[0]->isPrimitive() &&
				       types[0]->primitiveID() == PrimitivePtr &&
				       types[0]->templateArguments()[0].typeRefType()->isBuiltInVoid() &&
				       types[1]->isPrimitive() &&
				       types[1]->primitiveID() == PrimitiveSize;
			} else if (name == "__destroy") {
				return true;
			} else if (name == "__alignmask") {
				return true;
			} else if (name == "__sizeof") {
				return true;
			} else if (name == "__islive") {
				return true;
			} else if (name == "__setdead") {
				return true;
			} else if (name == "__dead") {
				return true;
			} else if (name == "__isvalid") {
				return true;
			} else if (name == "__setinvalid") {
				return true;
			} else {
				throw std::logic_error("Unknown lifetime method.");
			}
		}
		
		bool isValidConstness(const String& name,
		                      const SEM::Predicate& constPredicate) {
			if (name == "__moveto") {
				return constPredicate.isFalse();
			} else if (name == "__destroy") {
				return constPredicate.isFalse();
			} else if (name == "__alignmask") {
				return true;
			} else if (name == "__sizeof") {
				return true;
			} else if (name == "__islive") {
				return constPredicate.isTrue();
			} else if (name == "__setdead") {
				return constPredicate.isFalse();
			} else if (name == "__dead") {
				return true;
			} else if (name == "__isvalid") {
				return constPredicate.isTrue();
			} else if (name == "__setinvalid") {
				return constPredicate.isFalse();
			} else {
				throw std::logic_error("Unknown lifetime method.");
			}
		}
		
		bool isValidStaticness(const String& name,
		                       const bool isStatic) {
			if (name == "__moveto") {
				return !isStatic;
			} else if (name == "__destroy") {
				return !isStatic;
			} else if (name == "__alignmask") {
				return isStatic;
			} else if (name == "__sizeof") {
				return isStatic;
			} else if (name == "__islive") {
				return !isStatic;
			} else if (name == "__setdead") {
				return !isStatic;
			} else if (name == "__dead") {
				return isStatic;
			} else if (name == "__isvalid") {
				return !isStatic;
			} else if (name == "__setinvalid") {
				return !isStatic;
			} else {
				throw std::logic_error("Unknown lifetime method.");
			}
		}
		
		void validateFunctionType(const Name& functionFullName,
		                          const SEM::FunctionType& functionType,
		                          const SEM::Predicate& constPredicate,
		                          const Debug::SourceLocation& location) {
			const auto& name = functionFullName.last();
			if (!name.starts_with("__")) {
				// Not a lifetime method; any type can be valid.
				return;
			}
			
			if (!isValidReturnType(name, functionType.returnType())) {
				throw ErrorException(makeString("Lifetime method '%s' has incorrect return type, at location %s.",
					functionFullName.toString().c_str(),
					location.toString().c_str()));
			}
			
			if (!isValidArgumentCount(name, functionType.parameterTypes().size())) {
				throw ErrorException(makeString("Lifetime method '%s' has incorrect argument count, at location %s.",
					functionFullName.toString().c_str(),
					location.toString().c_str()));
			}
			
			if (!isValidArgumentTypes(name, functionType.parameterTypes())) {
				throw ErrorException(makeString("Lifetime method '%s' has incorrect argument types, at location %s.",
					functionFullName.toString().c_str(),
					location.toString().c_str()));
			}
			
			if (!isValidStaticness(name, !functionType.attributes().isMethod())) {
				throw ErrorException(makeString("Lifetime method '%s' has incorrect static-ness, at location %s.",
					functionFullName.toString().c_str(),
					location.toString().c_str()));
			}
			
			if (!isValidConstness(name, constPredicate)) {
				throw ErrorException(makeString("Lifetime method '%s' has incorrect const predicate, at location %s.",
					functionFullName.toString().c_str(),
					location.toString().c_str()));
			}
		}
		
		class LifetimeMethodNotNoExceptDiag: public Error {
		public:
			LifetimeMethodNotNoExceptDiag(std::string functionName)
			: functionName_(std::move(functionName)) { }
			
			std::string toString() const {
				return makeString("lifetime method '%s' isn't marked 'noexcept'",
				                  functionName_.c_str());
			}
			
		private:
			std::string functionName_;
			
		};
		
		void ConvertFunctionDeclType(Context& context, SEM::Function& function) {
			if (function.isDefault()) {
				// Type is already converted.
				return;
			}
			
			const auto& astFunctionNode = function.astFunction();
			const auto thisTypeInstance = lookupParentType(context.scopeStack());
			
			// Enable lookups for function template variables.
			PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(function));
			
			// Convert const specifier.
			if (!astFunctionNode->constSpecifier().isNull()) {
				function.setConstPredicate(ConvertConstSpecifier(context, astFunctionNode->constSpecifier()));
			}
			
			const auto& astReturnTypeNode = astFunctionNode->returnType();
			const SEM::Type* semReturnType = NULL;
			
			if (astReturnTypeNode->typeEnum == AST::Type::AUTO) {
				// Undefined return type means this must be a class
				// constructor, with no return type specified (i.e.
				// the return type will be the parent class type).
				assert(thisTypeInstance != nullptr);
				assert(astFunctionNode->isDefinition());
				assert(astFunctionNode->isStatic());
				
				semReturnType = thisTypeInstance->selfType();
			} else {
				semReturnType = ConvertType(context, astReturnTypeNode);
			}
			
			std::vector<SEM::Var*> parameterVars;
			parameterVars.reserve(astFunctionNode->parameters()->size());
			
			SEM::TypeArray parameterTypes;
			parameterTypes.reserve(astFunctionNode->parameters()->size());
			
			for (const auto& astTypeVarNode: *(astFunctionNode->parameters())) {
				if (!astTypeVarNode->isNamed()) {
					throw ErrorException(makeString("Pattern variables not supported (yet!) for parameter variables, at location %s.",
						astTypeVarNode.location().toString().c_str()));
				}
				
				auto paramVar = ConvertVar(context, Debug::VarInfo::VAR_ARGUMENT, astTypeVarNode);
				assert(paramVar->isBasic());
				
				parameterTypes.push_back(paramVar->constructType());
				parameterVars.push_back(paramVar.release());
			}
			
			function.setParameters(std::move(parameterVars));
			
			auto noExceptPredicate = ConvertNoExceptSpecifier(context, astFunctionNode->noexceptSpecifier());
			if (function.name().last() == "__destroy") {
				// Destructors are always noexcept.
				noExceptPredicate = SEM::Predicate::True();
			}
			
			if (!noExceptPredicate.isTrue() && function.name().last().starts_with("__")) {
				context.issueDiag(LifetimeMethodNotNoExceptDiag(function.name().toString()),
				                  astFunctionNode.location());
				noExceptPredicate = SEM::Predicate::True();
			}
			
			const bool isDynamicMethod = function.isMethod() && !astFunctionNode->isStatic();
			const bool isTemplatedMethod = !function.templateVariables().empty() ||
				(thisTypeInstance != nullptr && !thisTypeInstance->templateVariables().empty());
			
			SEM::FunctionAttributes attributes(astFunctionNode->isVarArg(), isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate));
			SEM::FunctionType functionType(std::move(attributes), semReturnType, std::move(parameterTypes));
			validateFunctionType(function.name(),
			                     functionType,
			                     function.constPredicate(),
			                     astFunctionNode.location());
			
			function.setType(functionType);
		}
		
	}
	
}


