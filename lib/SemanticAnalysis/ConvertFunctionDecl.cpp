#include <cassert>
#include <cstddef>
#include <cstdio>
#include <vector>
#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertPredicate.hpp>
#include <locic/SemanticAnalysis/ConvertVar.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/Template.hpp>
#include <locic/SemanticAnalysis/TypeResolver.hpp>
#include <locic/Support/MethodID.hpp>
#include <locic/Support/MethodIDMap.hpp>
#include <locic/Support/SharedMaps.hpp>

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
		
		namespace {
			
			class ShadowsTemplateParameterDiag: public Error {
			public:
				ShadowsTemplateParameterDiag(String name)
				: name_(std::move(name)) { }
				
				std::string toString() const {
					return makeString("declaration of '%s' shadows template parameter",
					                  name_.c_str());
				}
				
			private:
				String name_;
				
			};
			
		}
		
		class InterfaceMethodCannotBeTemplatedDiag: public Error {
		public:
			InterfaceMethodCannotBeTemplatedDiag(String name)
			: name_(std::move(name)) { }
			
			std::string toString() const {
				return makeString("interface method '%s' cannot be templated",
				                  name_.c_str());
			}
			
		private:
			String name_;
			
		};
		
		class FunctionCannotHaveConstSpecifierDiag: public Warning {
		public:
			FunctionCannotHaveConstSpecifierDiag(const Name& name)
			: name_(name.toString(/*addPrefix=*/false)) { }
			
			std::string toString() const {
				return makeString("non-method function '%s' cannot have const specifier",
				                  name_.c_str());
			}
			
		private:
			std::string name_;
			
		};
		
		class FunctionCannotBeStaticDiag: public Warning {
		public:
			FunctionCannotBeStaticDiag(const Name& name)
			: name_(name.toString(/*addPrefix=*/false)) { }
			
			std::string toString() const {
				return makeString("non-method function '%s' cannot be static",
				                  name_.c_str());
			}
			
		private:
			std::string name_;
			
		};
		
		class FunctionTemplateHasNonPrimitiveTypeDiag: public Error {
		public:
			FunctionTemplateHasNonPrimitiveTypeDiag(const String& varName,
			                                        const SEM::Type* type,
			                                        const Name& functionName)
			: varName_(varName), type_(type),
			functionName_(functionName.toString(/*addPrefix=*/false)) { }
			
			std::string toString() const {
				return makeString("template variable '%s' has non-primitive type '%s' in function '%s'",
				                  varName_.c_str(), type_->toDiagString().c_str(),
				                  functionName_.c_str());
			}
			
		private:
			String varName_;
			const SEM::Type* type_;
			std::string functionName_;
			
		};
		
		std::unique_ptr<SEM::Function>
		ConvertFunctionDecl(Context& context, AST::Node<AST::Function>& astFunctionNode,
		                    AST::ModuleScope moduleScope) {
			const auto thisTypeInstance = lookupParentType(context.scopeStack());
			
			const auto& name = astFunctionNode->name()->last();
			
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(getParent(context.scopeStack().back()),
			                                                             astFunctionNode,
			                                                             std::move(moduleScope)));
			
			const bool isMethod = thisTypeInstance != nullptr;
			
			if (!isMethod && !astFunctionNode->constSpecifier()->isNone()) {
				context.issueDiag(FunctionCannotHaveConstSpecifierDiag(semFunction->name()),
				                  astFunctionNode->constSpecifier().location());
			}
			
			if (!isMethod && astFunctionNode->isStatic()) {
				context.issueDiag(FunctionCannotBeStaticDiag(semFunction->name()),
				                  astFunctionNode.location());
			}
			
			// Don't treat extension methods as primitive methods.
			const bool isPrimitiveMethod = isMethod && thisTypeInstance->isPrimitive() && astFunctionNode->name()->size() == 1;
			const bool isPrimitiveFunction = astFunctionNode->isPrimitive();
			
			semFunction->setPrimitive(isPrimitiveMethod || isPrimitiveFunction);
			
			semFunction->setMethod(isMethod);
			semFunction->setStaticMethod(isMethod && astFunctionNode->isStatic());
			
			if (!astFunctionNode->templateVariables()->empty() && (thisTypeInstance != nullptr && thisTypeInstance->isInterface())) {
				context.issueDiag(InterfaceMethodCannotBeTemplatedDiag(name),
				                  astFunctionNode.location());
			}
			
			// Add template variables.
			size_t templateVarIndex = (thisTypeInstance != nullptr) ? thisTypeInstance->templateVariables().size() : 0;
			for (const auto& astTemplateVarNode: *(astFunctionNode->templateVariables())) {
				const auto& templateVarName = astTemplateVarNode->name();
				
				// TODO!
				const bool isVirtual = false;
				const auto semTemplateVar =
					new SEM::TemplateVar(context.semContext(),
						semFunction->name() + templateVarName,
						templateVarIndex++, isVirtual);
				
				const auto templateVarIterator = semFunction->namedTemplateVariables().find(templateVarName);
				if (templateVarIterator == semFunction->namedTemplateVariables().end()) {
					semFunction->namedTemplateVariables().insert(std::make_pair(templateVarName, semTemplateVar));
				} else {
					context.issueDiag(ShadowsTemplateParameterDiag(templateVarName),
					                  astTemplateVarNode.location());
				}
				
				// Also adding the function template variable type here.
				auto& astVarType = astTemplateVarNode->type();
				const auto semVarType = TypeResolver(context).resolveTemplateVarType(astVarType);
				
				if (!semVarType->isPrimitive()) {
					context.issueDiag(FunctionTemplateHasNonPrimitiveTypeDiag(templateVarName,
					                                                          semVarType,
					                                                          semFunction->name()),
					                  astTemplateVarNode->type().location());
				}
				
				semTemplateVar->setType(semVarType);
				semFunction->templateVariables().push_back(semTemplateVar);
			}
			
			assert(semFunction->isDeclaration());
			
			return semFunction;
		}
		
		bool isValidLifetimeMethod(const MethodID methodID) {
			switch (methodID) {
				case METHOD_MOVETO:
				case METHOD_DESTROY:
				case METHOD_ALIGNMASK:
				case METHOD_SIZEOF:
				case METHOD_ISLIVE:
				case METHOD_SETDEAD:
				case METHOD_DEAD:
				case METHOD_ISVALID:
				case METHOD_SETINVALID:
					return true;
				default:
					return false;
			}
		}
		
		bool isValidReturnType(const MethodID methodID, const SEM::Type* const type) {
			switch (methodID) {
				case METHOD_MOVETO:
					return type->isBuiltInVoid();
				case METHOD_DESTROY:
					return type->isBuiltInVoid();
				case METHOD_ALIGNMASK:
				case METHOD_SIZEOF:
					return type->isPrimitive() &&
					       type->primitiveID() == PrimitiveSize;
				case METHOD_ISLIVE:
					return type->isBuiltInBool();
				case METHOD_SETDEAD:
					return type->isBuiltInVoid();
				case METHOD_DEAD:
					// TODO
					return true;
				case METHOD_ISVALID:
					return type->isBuiltInBool();
				case METHOD_SETINVALID:
					return type->isBuiltInVoid();
				default:
					locic_unreachable("Unknown lifetime method.");
			}
		}
		
		bool isValidArgumentCount(const MethodID methodID, const size_t argCount) {
			switch (methodID) {
				case METHOD_MOVETO:
					return argCount == 2;
				case METHOD_DESTROY:
					return argCount == 0;
				case METHOD_ALIGNMASK:
					return argCount == 0;
				case METHOD_SIZEOF:
					return argCount == 0;
				case METHOD_ISLIVE:
					return argCount == 0;
				case METHOD_SETDEAD:
					return argCount == 0;
				case METHOD_DEAD:
					return argCount == 0;
				case METHOD_ISVALID:
					return argCount == 0;
				case METHOD_SETINVALID:
					return argCount == 0;
				default:
					locic_unreachable("Unknown lifetime method.");
			}
		}
		
		bool isValidArgumentTypes(const MethodID methodID, const SEM::TypeArray& types) {
			switch (methodID) {
				case METHOD_MOVETO:
					return types[0]->isPrimitive() &&
					       types[0]->primitiveID() == PrimitivePtr &&
					       types[0]->templateArguments()[0].typeRefType()->isBuiltInVoid() &&
					       types[1]->isPrimitive() &&
					       types[1]->primitiveID() == PrimitiveSize;
				case METHOD_DESTROY:
				case METHOD_ALIGNMASK:
				case METHOD_SIZEOF:
				case METHOD_ISLIVE:
				case METHOD_SETDEAD:
				case METHOD_DEAD:
				case METHOD_ISVALID:
				case METHOD_SETINVALID:
					return true;
				default:
					locic_unreachable("Unknown lifetime method.");
			}
		}
		
		bool isValidConstness(const MethodID methodID, const SEM::Predicate& constPredicate) {
			switch (methodID) {
				case METHOD_MOVETO:
					return constPredicate.isFalse();
				case METHOD_DESTROY:
					return constPredicate.isFalse();
				case METHOD_ALIGNMASK:
					return true;
				case METHOD_SIZEOF:
					return true;
				case METHOD_ISLIVE:
					return constPredicate.isTrue();
				case METHOD_SETDEAD:
					return constPredicate.isFalse();
				case METHOD_DEAD:
					return true;
				case METHOD_ISVALID:
					return constPredicate.isTrue();
				case METHOD_SETINVALID:
					return constPredicate.isFalse();
				default:
					locic_unreachable("Unknown lifetime method.");
			}
		}
		
		bool isValidStaticness(const MethodID methodID, const bool isStatic) {
			switch (methodID) {
				case METHOD_MOVETO:
					return !isStatic;
				case METHOD_DESTROY:
					return !isStatic;
				case METHOD_ALIGNMASK:
					return isStatic;
				case METHOD_SIZEOF:
					return isStatic;
				case METHOD_ISLIVE:
					return !isStatic;
				case METHOD_SETDEAD:
					return !isStatic;
				case METHOD_DEAD:
					return isStatic;
				case METHOD_ISVALID:
					return !isStatic;
				case METHOD_SETINVALID:
					return !isStatic;
				default:
					locic_unreachable("Unknown lifetime method.");
			}
		}
		
		class UnknownLifetimeMethodDiag: public Error {
		public:
			UnknownLifetimeMethodDiag(String functionName)
			: functionName_(std::move(functionName)) { }
			
			std::string toString() const {
				return makeString("unknown lifetime method '%s'",
				                  functionName_.c_str());
			}
			
		private:
			String functionName_;
			
		};
		
		class LifetimeMethodInvalidReturnTypeDiag: public Error {
		public:
			LifetimeMethodInvalidReturnTypeDiag(std::string functionName)
			: functionName_(std::move(functionName)) { }
			
			std::string toString() const {
				return makeString("lifetime method '%s' has incorrect return type",
				                  functionName_.c_str());
			}
			
		private:
			std::string functionName_;
			
		};
		
		class LifetimeMethodInvalidArgumentCountDiag: public Error {
		public:
			LifetimeMethodInvalidArgumentCountDiag(std::string functionName)
			: functionName_(std::move(functionName)) { }
			
			std::string toString() const {
				return makeString("lifetime method '%s' has incorrect argument count",
				                  functionName_.c_str());
			}
			
		private:
			std::string functionName_;
			
		};
		
		class LifetimeMethodInvalidArgumentTypesDiag: public Error {
		public:
			LifetimeMethodInvalidArgumentTypesDiag(std::string functionName)
			: functionName_(std::move(functionName)) { }
			
			std::string toString() const {
				return makeString("lifetime method '%s' has incorrect argument types",
				                  functionName_.c_str());
			}
			
		private:
			std::string functionName_;
			
		};
		
		class LifetimeMethodShouldBeStaticDiag: public Error {
		public:
			LifetimeMethodShouldBeStaticDiag(std::string functionName)
			: functionName_(std::move(functionName)) { }
			
			std::string toString() const {
				return makeString("lifetime method '%s' should be static",
				                  functionName_.c_str());
			}
			
		private:
			std::string functionName_;
			
		};
		
		class LifetimeMethodShouldNotBeStaticDiag: public Error {
		public:
			LifetimeMethodShouldNotBeStaticDiag(std::string functionName)
			: functionName_(std::move(functionName)) { }
			
			std::string toString() const {
				return makeString("lifetime method '%s' should not be static",
				                  functionName_.c_str());
			}
			
		private:
			std::string functionName_;
			
		};
		
		class LifetimeMethodInvalidConstPredicateDiag: public Error {
		public:
			LifetimeMethodInvalidConstPredicateDiag(std::string functionName)
			: functionName_(std::move(functionName)) { }
			
			std::string toString() const {
				return makeString("lifetime method '%s' has incorrect const predicate",
				                  functionName_.c_str());
			}
			
		private:
			std::string functionName_;
			
		};
		
		void validateFunctionType(Context& context, const Name& functionFullName,
		                          const SEM::FunctionType& functionType,
		                          const SEM::Predicate& constPredicate,
		                          const Debug::SourceLocation& location) {
			const auto& name = functionFullName.last();
			if (!name.starts_with("__")) {
				// Not a lifetime method; any type can be valid.
				return;
			}
			
			const auto methodID = context.sharedMaps().methodIDMap().tryGetMethodID(name);
			if (!methodID || !isValidLifetimeMethod(*methodID)) {
				context.issueDiag(UnknownLifetimeMethodDiag(name), location);
				return;
			}
			
			if (!isValidReturnType(*methodID, functionType.returnType())) {
				context.issueDiag(LifetimeMethodInvalidReturnTypeDiag(functionFullName.toString()),
				                  location);
			}
			
			if (!isValidArgumentCount(*methodID, functionType.parameterTypes().size())) {
				context.issueDiag(LifetimeMethodInvalidArgumentCountDiag(functionFullName.toString()),
				                  location);
			} else if (!isValidArgumentTypes(*methodID, functionType.parameterTypes())) {
				context.issueDiag(LifetimeMethodInvalidArgumentTypesDiag(functionFullName.toString()),
				                  location);
			}
			
			const bool isStatic = !functionType.attributes().isMethod();
			if (!isValidStaticness(*methodID, isStatic)) {
				if (isStatic) {
					context.issueDiag(LifetimeMethodShouldNotBeStaticDiag(functionFullName.toString()),
					                  location);
				} else {
					context.issueDiag(LifetimeMethodShouldBeStaticDiag(functionFullName.toString()),
					                  location);
				}
			} else if (!isValidConstness(*methodID, constPredicate)) {
				context.issueDiag(LifetimeMethodInvalidConstPredicateDiag(functionFullName.toString()),
				                  location);
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
		
		class PatternMatchingNotSupportedForParameterVariablesDiag: public Error {
		public:
			PatternMatchingNotSupportedForParameterVariablesDiag() { }
			
			std::string toString() const {
				return "pattern matching not supported for parameter variables";
			}
			
		};
		
		void ConvertFunctionDeclType(Context& context, SEM::Function& function) {
			if (function.isDefault()) {
				// Type is already converted.
				return;
			}
			
			auto& astFunctionNode = function.astFunction();
			const auto thisTypeInstance = lookupParentType(context.scopeStack());
			
			// Enable lookups for function template variables.
			PushScopeElement pushScopeElement(context.scopeStack(), ScopeElement::Function(function));
			
			// Convert const specifier.
			if (!astFunctionNode->constSpecifier().isNull()) {
				function.setConstPredicate(ConvertConstSpecifier(context, astFunctionNode->constSpecifier()));
			}
			
			auto& astReturnTypeNode = astFunctionNode->returnType();
			const SEM::Type* semReturnType = NULL;
			
			if (astReturnTypeNode->typeEnum == AST::TypeDecl::AUTO) {
				// Undefined return type means this must be a class
				// constructor, with no return type specified (i.e.
				// the return type will be the parent class type).
				assert(thisTypeInstance != nullptr);
				assert(astFunctionNode->isDefinition());
				assert(astFunctionNode->isStatic());
				
				semReturnType = thisTypeInstance->selfType();
			} else {
				semReturnType = TypeResolver(context).resolveType(astReturnTypeNode);
			}
			
			std::vector<SEM::Var*> parameterVars;
			parameterVars.reserve(astFunctionNode->parameters()->size());
			
			SEM::TypeArray parameterTypes;
			parameterTypes.reserve(astFunctionNode->parameters()->size());
			
			size_t index = 0;
			
			for (auto& astVarNode: *(astFunctionNode->parameters())) {
				if (!astVarNode->isNamed()) {
					context.issueDiag(PatternMatchingNotSupportedForParameterVariablesDiag(),
					                  astVarNode.location());
					continue;
				}
				
				auto paramVar = ConvertVar(context, Debug::VarInfo::VAR_ARGUMENT, astVarNode);
				assert(paramVar->isBasic());
				
				paramVar->setIndex(index++);
				
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
			validateFunctionType(context, function.name(),
			                     functionType,
			                     function.constPredicate(),
			                     astFunctionNode.location());
			
			function.setType(functionType);
		}
		
	}
	
}


