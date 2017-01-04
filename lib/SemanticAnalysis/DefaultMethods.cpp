#include <stdexcept>
#include <vector>

#include <locic/AST/Function.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/Constant.hpp>

#include <locic/Support/MakeArray.hpp>
#include <locic/Support/Name.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>
#include <locic/SemanticAnalysis/TypeCapabilities.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		Debug::FunctionInfo makeDefaultFunctionInfo(const AST::TypeInstance& parentType,
		                                            const AST::Function& function) {
			Debug::FunctionInfo functionInfo;
			functionInfo.name = function.fullName().copy();
			functionInfo.declLocation = parentType.debugInfo()->location;
			functionInfo.scopeLocation = parentType.debugInfo()->location;
			return functionInfo;
		}
		
		AST::Predicate getDefaultSizedTypePredicate(Context& context, const AST::TypeInstance* const typeInstance) {
			auto requirePredicate = AST::Predicate::True();
			
			const auto sizedType = getBuiltInType(context, context.getCString("sized_type_t"), {});
			
			// All member variables need to be sized.
			for (const auto& var: typeInstance->variables()) {
				const auto varType = var->constructType();
				requirePredicate = AST::Predicate::And(std::move(requirePredicate), AST::Predicate::Satisfies(varType, sizedType));
			}
			
			// All variants need to be sized.
			for (const auto& variantTypeInstance: typeInstance->variants()) {
				const auto varType = variantTypeInstance->selfType();
				requirePredicate = AST::Predicate::And(std::move(requirePredicate), AST::Predicate::Satisfies(varType, sizedType));
			}
			
			return requirePredicate;
		}
		
		AST::Predicate getAutoDefaultMovePredicate(Context& context, const AST::TypeInstance* const typeInstance) {
			auto requirePredicate = getDefaultSizedTypePredicate(context, typeInstance);
			
			// All member variables need to be movable.
			for (const auto& var: typeInstance->variables()) {
				const auto varType = var->constructType();
				const auto movableType = context.typeBuilder().getMovableInterfaceType(varType->createNoTagType());
				requirePredicate = AST::Predicate::And(std::move(requirePredicate), AST::Predicate::Satisfies(varType, movableType));
			}
			
			// All variants need to be movable.
			for (const auto& variantTypeInstance: typeInstance->variants()) {
				const auto varType = variantTypeInstance->selfType();
				const auto movableType = context.typeBuilder().getMovableInterfaceType(varType->createNoTagType());
				requirePredicate = AST::Predicate::And(std::move(requirePredicate), AST::Predicate::Satisfies(varType, movableType));
			}
			
			return requirePredicate;
		}
		
		AST::Predicate getDefaultMovePredicate(Context& context, const AST::TypeInstance* const typeInstance) {
			if (typeInstance->movePredicate()) {
				// Use a user-provided move predicate if available.
				return typeInstance->movePredicate()->copy();
			} else {
				// Otherwise automatically generate a move predicate.
				return getAutoDefaultMovePredicate(context, typeInstance);
			}
		}
		
		AST::Predicate getDefaultCopyPredicate(Context& context, const AST::TypeInstance* const typeInstance, const String& propertyName) {
			auto predicate = getDefaultSizedTypePredicate(context, typeInstance);
			
			// All member variables need to be copyable.
			for (const auto& var: typeInstance->variables()) {
				const auto varType = var->constructType()->withoutTags();
				const auto copyableType = getBuiltInType(context, propertyName, { varType });
				predicate = AST::Predicate::And(std::move(predicate), AST::Predicate::Satisfies(varType, copyableType));
			}
			
			// All variants need to be copyable.
			for (const auto& variantTypeInstance: typeInstance->variants()) {
				const auto varType = variantTypeInstance->selfType();
				const auto copyableType = getBuiltInType(context, propertyName, { varType });
				predicate = AST::Predicate::And(std::move(predicate), AST::Predicate::Satisfies(varType, copyableType));
			}
			
			return predicate;
		}
		
		AST::Predicate getDefaultImplicitCopyNoExceptPredicate(Context& context, const AST::TypeInstance* const typeInstance) {
			return getDefaultCopyPredicate(context, typeInstance, context.getCString("noexcept_implicit_copyable_t"));
		}
		
		AST::Predicate getDefaultImplicitCopyRequirePredicate(Context& context, const AST::TypeInstance* const typeInstance) {
			return getDefaultCopyPredicate(context, typeInstance, context.getCString("implicit_copyable_t"));
		}
		
		AST::Predicate getDefaultExplicitCopyNoExceptPredicate(Context& context, const AST::TypeInstance* const typeInstance) {
			return getDefaultCopyPredicate(context, typeInstance, context.getCString("noexcept_copyable_t"));
		}
		
		AST::Predicate getDefaultExplicitCopyRequirePredicate(Context& context, const AST::TypeInstance* const typeInstance) {
			return getDefaultCopyPredicate(context, typeInstance, context.getCString("copyable_t"));
		}
		
		AST::Predicate getDefaultComparePredicate(Context& context, const AST::TypeInstance* const typeInstance, const String& propertyName) {
			auto predicate = getDefaultSizedTypePredicate(context, typeInstance);
			
			// All member variables need to be copyable.
			for (const auto& var: typeInstance->variables()) {
				const auto varType = var->constructType()->withoutTags();
				const auto copyableType = getBuiltInType(context, propertyName, { varType });
				predicate = AST::Predicate::And(std::move(predicate), AST::Predicate::Satisfies(varType, copyableType));
			}
			
			// All variants need to be copyable.
			for (const auto& variantTypeInstance: typeInstance->variants()) {
				const auto varType = variantTypeInstance->selfType();
				const auto copyableType = getBuiltInType(context, propertyName, { varType });
				predicate = AST::Predicate::And(std::move(predicate), AST::Predicate::Satisfies(varType, copyableType));
			}
			
			return predicate;
		}
		
		AST::Predicate getDefaultCompareNoExceptPredicate(Context& context, const AST::TypeInstance* const typeInstance) {
			const auto propertyName = context.getCString("noexcept_comparable_t");
			return getDefaultComparePredicate(context, typeInstance, propertyName);
		}
		
		AST::Predicate getDefaultCompareRequirePredicate(Context& context, const AST::TypeInstance* const typeInstance) {
			const auto propertyName = context.getCString("comparable_t");
			return getDefaultComparePredicate(context, typeInstance, propertyName);
		}
		
		DefaultMethods::DefaultMethods(Context& context)
		: context_(context) { }
		
		std::unique_ptr<AST::Function>
		DefaultMethods::createAutoGeneratedDecl(AST::TypeInstance* typeInstance,
		                                        const Name& name, bool isStatic) {
			std::unique_ptr<AST::Function> function(new AST::Function());
			function->setParent(AST::GlobalStructure::TypeInstance(*typeInstance));
			function->setFullName(name.copy());
			function->setModuleScope(typeInstance->moduleScope().copy());
			
			function->setAutoGenerated(true);
			
			function->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *function));
			
			function->setMethod(true);
			function->setIsStatic(isStatic);
			
			return function;
		}
		
		std::unique_ptr<AST::Function>
		DefaultMethods::createDefaultConstructorDecl(AST::TypeInstance* typeInstance,
		                                             const Name& name) {
			auto function = createAutoGeneratedDecl(typeInstance, name,
			                                        /*isStatic=*/true);
			
			completeDefaultConstructorDecl(typeInstance, *function);
			
			return function;
		}
		
		void
		DefaultMethods::completeDefaultConstructorDecl(AST::TypeInstance* typeInstance,
		                                               AST::Function& function) {
			// This method requires move, so add the move predicate.
			function.setRequiresPredicate(AST::Predicate::And(typeInstance->requiresPredicate().copy(),
			                                                  getDefaultMovePredicate(context_, typeInstance)));
			
			const bool isVarArg = false;
			const bool isDynamicMethod = false;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Default constructor only moves, and since moves never
			// throw the constructor never throws.
			auto noExceptPredicate = AST::Predicate::True();
			
			auto constructTypes = typeInstance->constructTypes();
			
			AST::FunctionAttributes attributes(isVarArg, isDynamicMethod,
			                                   isTemplatedMethod,
			                                   std::move(noExceptPredicate));
			function.setType(AST::FunctionType(std::move(attributes),
			                                   typeInstance->selfType(),
			                                   std::move(constructTypes)));
		}
		
		std::unique_ptr<AST::Function>
		DefaultMethods::createDefaultAlignMaskDecl(AST::TypeInstance* typeInstance,
		                                           const Name& name) {
			auto function = createAutoGeneratedDecl(typeInstance, name,
			                                        /*isStatic=*/true);
			
			completeDefaultAlignMaskDecl(typeInstance, *function);
			
			return function;
		}
		
		void
		DefaultMethods::completeDefaultAlignMaskDecl(AST::TypeInstance* typeInstance,
		                                             AST::Function& function) {
			function.setRequiresPredicate(AST::Predicate::And(typeInstance->requiresPredicate().copy(),
			                                                  getDefaultSizedTypePredicate(context_, typeInstance)));
			
			const bool isVarArg = false;
			const bool isDynamicMethod = false;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// alignmask never throws.
			auto noExceptPredicate = AST::Predicate::True();
			
			const auto sizeType = context_.typeBuilder().getSizeType();
			function.setType(AST::FunctionType(AST::FunctionAttributes(isVarArg, isDynamicMethod,
			                                                           isTemplatedMethod,
			                                                           std::move(noExceptPredicate)),
			                                   sizeType, {}));
		}
		
		std::unique_ptr<AST::Function>
		DefaultMethods::createDefaultSizeOfDecl(AST::TypeInstance* typeInstance,
		                                        const Name& name) {
			auto function = createAutoGeneratedDecl(typeInstance, name,
			                                        /*isStatic=*/true);
			
			completeDefaultSizeOfDecl(typeInstance, *function);
			
			return function;
		}
		
		void
		DefaultMethods::completeDefaultSizeOfDecl(AST::TypeInstance* typeInstance,
		                                          AST::Function& function) {
			function.setRequiresPredicate(AST::Predicate::And(typeInstance->requiresPredicate().copy(),
			                                                  getDefaultSizedTypePredicate(context_, typeInstance)));
			
			const bool isVarArg = false;
			const bool isDynamicMethod = false;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// sizeof never throws.
			auto noExceptPredicate = AST::Predicate::True();
			
			const auto sizeType = context_.typeBuilder().getSizeType();
			function.setType(AST::FunctionType(AST::FunctionAttributes(isVarArg, isDynamicMethod,
			                                                           isTemplatedMethod,
			                                                           std::move(noExceptPredicate)),
			                                   sizeType, {}));
		}
		
		std::unique_ptr<AST::Function>
		DefaultMethods::createDefaultDestroyDecl(AST::TypeInstance* typeInstance,
		                                         const Name& name) {
			auto function = createAutoGeneratedDecl(typeInstance, name,
			                                        /*isStatic=*/false);
			
			completeDefaultDestroyDecl(typeInstance, *function);
			
			return function;
		}
		
		void
		DefaultMethods::completeDefaultDestroyDecl(AST::TypeInstance* typeInstance,
		                                           AST::Function& function) {
			function.setRequiresPredicate(typeInstance->requiresPredicate().copy());
			
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Destructor never throws.
			auto noExceptPredicate = AST::Predicate::True();
			
			const auto voidType = context_.typeBuilder().getVoidType();
			
			function.setType(AST::FunctionType(AST::FunctionAttributes(isVarArg, isDynamicMethod,
			                                                           isTemplatedMethod,
			                                                           std::move(noExceptPredicate)),
			                                   voidType, /*argTypes=*/{}));
		}
		
		std::unique_ptr<AST::Function>
		DefaultMethods::createDefaultMoveDecl(AST::TypeInstance* typeInstance,
		                                      const Name& name) {
			auto function = createAutoGeneratedDecl(typeInstance, name,
			                                        /*isStatic=*/false);
			
			completeDefaultMoveDecl(typeInstance, *function);
			
			return function;
		}
		
		void
		DefaultMethods::completeDefaultMoveDecl(AST::TypeInstance* typeInstance,
		                                        AST::Function& function) {
			function.setRequiresPredicate(AST::Predicate::And(typeInstance->requiresPredicate().copy(),
			                                                  getDefaultMovePredicate(context_, typeInstance)));
			
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Move never throws.
			auto noExceptPredicate = AST::Predicate::True();
			
			AST::FunctionAttributes attributes(isVarArg, isDynamicMethod,
			                                   isTemplatedMethod,
			                                   std::move(noExceptPredicate));
			
			const auto returnType = typeInstance->selfType();
			function.setType(AST::FunctionType(std::move(attributes),
			                                   returnType, {}));
		}
		
		std::unique_ptr<AST::Function>
		DefaultMethods::createDefaultImplicitCopyDecl(AST::TypeInstance* typeInstance,
		                                              const Name& name) {
			auto function = createAutoGeneratedDecl(typeInstance, name,
			                                        /*isStatic=*/false);
			
			completeDefaultImplicitCopyDecl(typeInstance, *function);
			
			return function;
		}
		
		void
		DefaultMethods::completeDefaultImplicitCopyDecl(AST::TypeInstance* typeInstance,
		                                                AST::Function& function) {
			function.setRequiresPredicate(AST::Predicate::And(typeInstance->requiresPredicate().copy(),
			                                                  getDefaultImplicitCopyRequirePredicate(context_, typeInstance)));
			
			function.setConstPredicate(AST::Predicate::True());
			
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Default copy method may throw since it
			// may call child copy methods that throw.
			auto noExceptPredicate = getDefaultImplicitCopyNoExceptPredicate(context_, typeInstance);
			
			// Implicit copy should return a notag() type.
			const auto returnType = typeInstance->selfType()->createNoTagType();
			
			function.setType(AST::FunctionType(AST::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), returnType, {}));
		}
		
		std::unique_ptr<AST::Function>
		DefaultMethods::createDefaultExplicitCopyDecl(AST::TypeInstance* typeInstance,
		                                              const Name& name) {
			auto function = createAutoGeneratedDecl(typeInstance, name,
			                                        /*isStatic=*/false);
			
			completeDefaultExplicitCopyDecl(typeInstance, *function);
			
			return function;
		}
		
		void
		DefaultMethods::completeDefaultExplicitCopyDecl(AST::TypeInstance* typeInstance,
		                                                AST::Function& function) {
			function.setRequiresPredicate(AST::Predicate::And(typeInstance->requiresPredicate().copy(),
			                                                  getDefaultExplicitCopyRequirePredicate(context_, typeInstance)));
			
			function.setConstPredicate(AST::Predicate::True());
			
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Default copy method may throw since it
			// may call child copy methods that throw.
			auto noExceptPredicate = getDefaultExplicitCopyNoExceptPredicate(context_, typeInstance);
			
			// Implicit copy should return a notag() type.
			const auto returnType = typeInstance->selfType()->createNoTagType();
			
			function.setType(AST::FunctionType(AST::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), returnType, {}));
		}
		
		std::unique_ptr<AST::Function>
		DefaultMethods::createDefaultCompareDecl(AST::TypeInstance* typeInstance,
		                                         const Name& name) {
			auto function = createAutoGeneratedDecl(typeInstance, name,
			                                        /*isStatic=*/false);
			
			completeDefaultCompareDecl(typeInstance, *function);
			
			return function;
		}
		
		void
		DefaultMethods::completeDefaultCompareDecl(AST::TypeInstance* typeInstance,
		                                           AST::Function& function) {
			function.setRequiresPredicate(AST::Predicate::And(typeInstance->requiresPredicate().copy(),
			                                                  getDefaultCompareRequirePredicate(context_, typeInstance)));
			
			function.setConstPredicate(AST::Predicate::True());
			
			const bool isVarArg = false;
			const bool isDynamicMethod =true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Default compare method may throw since it
			// may call child compare methods that throw.
			auto noExceptPredicate = getDefaultCompareNoExceptPredicate(context_, typeInstance);
			
			const auto selfType = typeInstance->selfType();
			const auto argType = createReferenceType(context_, selfType->createConstType(AST::Predicate::True()));
			const auto compareResultType = getBuiltInType(context_, context_.getCString("compare_result_t"), {});
			
			AST::TypeArray argTypes;
			argTypes.reserve(1);
			argTypes.push_back(argType);
			
			AST::FunctionAttributes attributes(isVarArg, isDynamicMethod,
			                                   isTemplatedMethod,
			                                   std::move(noExceptPredicate));
			function.setType(AST::FunctionType(std::move(attributes),
			                                   compareResultType,
			                                   std::move(argTypes)));
		}
		
		std::unique_ptr<AST::Function>
		DefaultMethods::createDefaultSetDeadDecl(AST::TypeInstance* typeInstance,
		                                         const Name& name) {
			auto function = createAutoGeneratedDecl(typeInstance, name,
			                                        /*isStatic=*/false);
			
			completeDefaultSetDeadDecl(typeInstance, *function);
			
			return function;
		}
		
		void
		DefaultMethods::completeDefaultSetDeadDecl(AST::TypeInstance* typeInstance,
		                                           AST::Function& function) {
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Never throws.
			auto noExceptPredicate = AST::Predicate::True();
			
			const auto voidType = context_.typeBuilder().getVoidType();
			
			function.setType(AST::FunctionType(AST::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), voidType, {}));
		}
		
		std::unique_ptr<AST::Function>
		DefaultMethods::createDefaultIsLiveDecl(AST::TypeInstance* typeInstance,
		                                        const Name& name) {
			auto function = createAutoGeneratedDecl(typeInstance, name,
			                                        /*isStatic=*/false);
			
			completeDefaultIsLiveDecl(typeInstance, *function);
			
			return function;
		}
		
		void
		DefaultMethods::completeDefaultIsLiveDecl(AST::TypeInstance* typeInstance,
		                                          AST::Function& function) {
			function.setConstPredicate(AST::Predicate::True());
			
			const bool isVarArg = false;
			const bool isDynamicMethod =true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Never throws.
			auto noExceptPredicate = AST::Predicate::True();
			
			const auto boolType = context_.typeBuilder().getBoolType();
			
			function.setType(AST::FunctionType(AST::FunctionAttributes(isVarArg, isDynamicMethod,
			                                                           isTemplatedMethod,
			                                                           std::move(noExceptPredicate)),
			                                   boolType, {}));
		}
		
		class DefaultMethodMustBeStaticDiag: public Error {
		public:
			DefaultMethodMustBeStaticDiag(const Name& name)
			: nameString_(name.toString(/*addPrefix=*/false)) { }
			
			std::string toString() const {
				return makeString("default method '%s' must be static",
				                  nameString_.c_str());
			}
			
		private:
			std::string nameString_;
			
		};
		
		class DefaultMethodMustBeNonStaticDiag: public Error {
		public:
			DefaultMethodMustBeNonStaticDiag(const Name& name)
			: nameString_(name.toString(/*addPrefix=*/false)) { }
			
			std::string toString() const {
				return makeString("default method '%s' must be non-static",
				                  nameString_.c_str());
			}
			
		private:
			std::string nameString_;
			
		};
		
		class UnknownDefaultMethodDiag: public Error {
		public:
			UnknownDefaultMethodDiag(String functionName)
			: functionName_(std::move(functionName)) { }
			
			std::string toString() const {
				return makeString("unknown default method '%s'",
				                  functionName_.c_str());
			}
			
		private:
			String functionName_;
			
		};
		
		void
		DefaultMethods::completeDefaultMethodDecl(AST::TypeInstance* typeInstance,
		                                          AST::Node<AST::Function>& function) {
			assert(!typeInstance->isClassDecl());
			assert(!typeInstance->isInterface());
			
			const auto& name = function->fullName();
			const bool isStatic = function->isStatic();
			const auto location = function.location();
			
			const auto canonicalName = CanonicalizeMethodName(name.last());
			if (canonicalName == "create") {
				if (!isStatic) {
					context_.issueDiag(DefaultMethodMustBeStaticDiag(name),
					                  location);
					function->setIsStatic(true);
				}
				
				completeDefaultConstructorDecl(typeInstance, *function);
			} else if (canonicalName == "__destroy") {
				if (isStatic) {
					context_.issueDiag(DefaultMethodMustBeNonStaticDiag(name),
					                  location);
					function->setIsStatic(false);
				}
				
				completeDefaultDestroyDecl(typeInstance, *function);
			} else if (canonicalName == "__move") {
				if (isStatic) {
					context_.issueDiag(DefaultMethodMustBeNonStaticDiag(name),
					                  location);
					function->setIsStatic(false);
				}
				
				completeDefaultMoveDecl(typeInstance, *function);
			} else if (canonicalName == "implicitcopy") {
				if (isStatic) {
					context_.issueDiag(DefaultMethodMustBeNonStaticDiag(name),
					                  location);
					function->setIsStatic(false);
				}
				
				completeDefaultImplicitCopyDecl(typeInstance, *function);
			} else if (canonicalName == "copy") {
				if (isStatic) {
					context_.issueDiag(DefaultMethodMustBeNonStaticDiag(name),
					                  location);
					function->setIsStatic(false);
				}
				
				completeDefaultExplicitCopyDecl(typeInstance, *function);
			} else if (canonicalName == "compare") {
				if (isStatic) {
					context_.issueDiag(DefaultMethodMustBeNonStaticDiag(name),
					                  location);
					function->setIsStatic(false);
				}
				
				completeDefaultCompareDecl(typeInstance, *function);
			} else if (canonicalName == "__setdead") {
				if (isStatic) {
					context_.issueDiag(DefaultMethodMustBeNonStaticDiag(name),
					                  location);
					function->setIsStatic(false);
				}
				
				completeDefaultSetDeadDecl(typeInstance, *function);
			} else if (canonicalName == "__islive") {
				if (isStatic) {
					context_.issueDiag(DefaultMethodMustBeNonStaticDiag(name),
					                  location);
					function->setIsStatic(false);
				}
				
				completeDefaultIsLiveDecl(typeInstance, *function);
			} else {
				context_.issueDiag(UnknownDefaultMethodDiag(name.last()), location);
				completeDefaultConstructorDecl(typeInstance, *function);
			}
		}
		
		bool
		DefaultMethods::hasDefaultConstructor(AST::TypeInstance* const typeInstance) {
			return typeInstance->isDatatype() ||
				typeInstance->isException() ||
				typeInstance->isStruct() ||
				typeInstance->isUnion();
		}
		
		bool
		DefaultMethods::hasDefaultAlignMask(AST::TypeInstance* const typeInstance) {
			if (typeInstance->isInterface() || typeInstance->isPrimitive()) {
				return false;
			}
			
			// There's only a default method if the user
			// hasn't specified a custom method.
			return !typeInstance->hasFunction(context_.getCString("__alignmask"));
		}
		
		bool DefaultMethods::hasDefaultSizeOf(AST::TypeInstance* const typeInstance) {
			if (typeInstance->isInterface() || typeInstance->isPrimitive()) {
				return false;
			}
			
			// There's only a default method if the user
			// hasn't specified a custom method.
			return !typeInstance->hasFunction(context_.getCString("__sizeof"));
		}
		
		bool DefaultMethods::hasDefaultDestroy(AST::TypeInstance* const typeInstance) {
			if (typeInstance->isInterface() || typeInstance->isPrimitive()) {
				return false;
			}
			
			// There's only a default destructor if the user
			// hasn't specified a custom destructor.
			return !typeInstance->hasFunction(context_.getCString("__destroy"));
		}
		
		bool
		DefaultMethods::hasDefaultMove(AST::TypeInstance* const typeInstance) {
			if (typeInstance->isInterface() || typeInstance->isPrimitive()) {
				return false;
			}
			
			// There's only a default move method if the user
			// hasn't specified a custom move method.
			return !typeInstance->hasFunction(context_.getCString("__move"));
		}
		
		bool
		DefaultMethods::hasDefaultImplicitCopy(AST::TypeInstance* const typeInstance) {
			return typeInstance->isClassDef() ||
				typeInstance->isDatatype() ||
				typeInstance->isEnum() ||
				typeInstance->isException() ||
				typeInstance->isStruct() ||
				typeInstance->isUnion() ||
				typeInstance->isUnionDatatype();
		}
		
		bool
		DefaultMethods::hasDefaultExplicitCopy(AST::TypeInstance* const typeInstance) {
			return typeInstance->isClassDef() ||
				typeInstance->isDatatype() ||
				typeInstance->isEnum() ||
				typeInstance->isException() ||
				typeInstance->isStruct() ||
				typeInstance->isUnion() ||
				typeInstance->isUnionDatatype();
		}
		
		bool
		DefaultMethods::hasDefaultCompare(AST::TypeInstance* const typeInstance) {
			return typeInstance->isClassDef() ||
				typeInstance->isDatatype() ||
				typeInstance->isEnum() ||
				typeInstance->isException() ||
				typeInstance->isStruct() ||
				typeInstance->isUnionDatatype();
		}
		
		bool
		DefaultMethods::hasDefaultSetDead(AST::TypeInstance* const typeInstance) {
			if (typeInstance->isInterface() || typeInstance->isPrimitive()) {
				return false;
			}
			
			// There's only a default __setdead method if the user
			// hasn't specified a custom __setdead method.
			return !typeInstance->hasFunction(context_.getCString("__setdead"));
		}
		
		bool
		DefaultMethods::hasDefaultIsLive(AST::TypeInstance* const typeInstance) {
			if (typeInstance->isInterface() || typeInstance->isPrimitive()) {
				return false;
			}
			
			// There's only a default islive method if the user
			// hasn't specified a custom islive method.
			return !typeInstance->hasFunction(context_.getCString("__islive"));
		}
		
		void
		DefaultMethods::createDefaultConstructor(AST::TypeInstance* const typeInstance,
		                                         AST::Function& /*function*/,
		                                         const Debug::SourceLocation& /*location*/) {
			(void) typeInstance;
			assert(!typeInstance->isUnionDatatype());
			
			// TODO: Need to check if default constructor can be created.
		}
		
		class CannotGenerateImplicitCopyDiag: public Error {
		public:
			CannotGenerateImplicitCopyDiag() { }
			
			std::string toString() const {
				return "cannot generate implicit copy since members don't support it";
			}
			
		};
		
		bool
		DefaultMethods::createDefaultImplicitCopy(AST::TypeInstance* typeInstance,
		                                          AST::Function& /*function*/,
		                                          const Debug::SourceLocation& location) {
			if (!TypeCapabilities(context_).supportsImplicitCopy(typeInstance->selfType())) {
				if (!typeInstance->isClassDef()) {
					// Auto-generated, so ignore the problem.
					return false;
				}
				context_.issueDiag(CannotGenerateImplicitCopyDiag(), location);
			}
			
			return true;
		}
		
		class CannotGenerateExplicitCopyDiag: public Error {
		public:
			CannotGenerateExplicitCopyDiag() { }
			
			std::string toString() const {
				return "cannot generate copy since members don't support it";
			}
			
		};
		
		bool
		DefaultMethods::createDefaultExplicitCopy(AST::TypeInstance* typeInstance,
		                                          AST::Function& /*function*/,
		                                          const Debug::SourceLocation& location) {
			if (!TypeCapabilities(context_).supportsExplicitCopy(typeInstance->selfType())) {
				if (!typeInstance->isClassDef()) {
					// Auto-generated, so ignore the problem.
					return false;
				}
				context_.issueDiag(CannotGenerateExplicitCopyDiag(), location);
			}
			
			return true;
		}
		
		class CannotGenerateCompareDiag: public Error {
		public:
			CannotGenerateCompareDiag() { }
			
			std::string toString() const {
				return "cannot generate compare since members don't support it";
			}
			
		};
		
		bool
		DefaultMethods::createDefaultCompare(AST::TypeInstance* typeInstance,
		                                     AST::Function& /*function*/,
		                                     const Debug::SourceLocation& location) {
			assert(!typeInstance->isUnion());
			if (!TypeCapabilities(context_).supportsCompare(typeInstance->selfType())) {
				if (!typeInstance->isClassDef()) {
					// Auto-generated, so ignore the problem.
					return false;
				}
				context_.issueDiag(CannotGenerateCompareDiag(), location);
			}
			
			return true;
		}
		
		bool
		DefaultMethods::createDefaultMethod(AST::TypeInstance* const typeInstance,
		                                    AST::Function& function,
		                                    const Debug::SourceLocation& location) {
			assert(!typeInstance->isClassDecl() && !typeInstance->isInterface());
			assert(function.isAutoGenerated() && !function.hasScope());
			
			PushScopeElement pushFunction(context_.scopeStack(), ScopeElement::Function(function));
			
			const auto& name = function.fullName();
			const auto canonicalName = CanonicalizeMethodName(name.last());
			if (canonicalName == "__move" ||
			    canonicalName == "__destroy" ||
			    canonicalName == "__dead" ||
			    canonicalName == "__islive") {
				// Generate by CodeGen.
				return true;
			} else if (canonicalName == "create") {
				assert(!typeInstance->isException());
				createDefaultConstructor(typeInstance, function, location);
				return true;
			} else if (canonicalName == "implicitcopy") {
				assert(hasDefaultImplicitCopy(typeInstance));
				return createDefaultImplicitCopy(typeInstance, function, location);
			} else if (canonicalName == "copy") {
				assert(hasDefaultExplicitCopy(typeInstance));
				return createDefaultExplicitCopy(typeInstance, function, location);
			} else if (canonicalName == "compare") {
				assert(hasDefaultCompare(typeInstance));
				return createDefaultCompare(typeInstance, function, location);
			} else {
				// Unknown default methods should be handled by now.
				return true;
			}
		}
		
	}
	
}

