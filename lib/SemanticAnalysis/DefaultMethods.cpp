#include <stdexcept>
#include <vector>

#include <locic/Constant.hpp>
#include <locic/Support/MakeArray.hpp>
#include <locic/Support/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/MethodSet.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		Debug::FunctionInfo makeDefaultFunctionInfo(const SEM::TypeInstance& parentType, const SEM::Function& function) {
			Debug::FunctionInfo functionInfo;
			functionInfo.isDefinition = !parentType.isClassDecl();
			functionInfo.name = function.name().copy();
			functionInfo.declLocation = parentType.debugInfo()->location;
			functionInfo.scopeLocation = parentType.debugInfo()->location;
			return functionInfo;
		}
		
		SEM::Predicate getDefaultSizedTypePredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			auto requirePredicate = SEM::Predicate::True();
			
			const auto sizedType = getBuiltInType(context, context.getCString("sized_type_t"), {});
			
			// All member variables need to be sized.
			for (const auto& var: typeInstance->variables()) {
				const auto varType = var->constructType();
				requirePredicate = SEM::Predicate::And(std::move(requirePredicate), SEM::Predicate::Satisfies(varType, sizedType));
			}
			
			// All variants need to be sized.
			for (const auto& variantTypeInstance: typeInstance->variants()) {
				const auto varType = variantTypeInstance->selfType();
				requirePredicate = SEM::Predicate::And(std::move(requirePredicate), SEM::Predicate::Satisfies(varType, sizedType));
			}
			
			return requirePredicate;
		}
		
		SEM::Predicate getAutoDefaultMovePredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			auto requirePredicate = SEM::Predicate::True();
			
			const auto movableType = context.typeBuilder().getMovableInterfaceType();
			
			// All member variables need to be movable.
			for (const auto& var: typeInstance->variables()) {
				const auto varType = var->constructType();
				requirePredicate = SEM::Predicate::And(std::move(requirePredicate), SEM::Predicate::Satisfies(varType, movableType));
			}
			
			// All variants need to be movable.
			for (const auto& variantTypeInstance: typeInstance->variants()) {
				const auto varType = variantTypeInstance->selfType();
				requirePredicate = SEM::Predicate::And(std::move(requirePredicate), SEM::Predicate::Satisfies(varType, movableType));
			}
			
			return requirePredicate;
		}
		
		SEM::Predicate getDefaultMovePredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			if (typeInstance->movePredicate()) {
				// Use a user-provided move predicate if available.
				return typeInstance->movePredicate()->copy();
			} else {
				// Otherwise automatically generate a move predicate.
				return getAutoDefaultMovePredicate(context, typeInstance);
			}
		}
		
		// A similar version of the move predicate but uses notag() to get non-const type.
		SEM::Predicate getDefaultCopyMovePredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			auto requirePredicate = SEM::Predicate::True();
			
			const auto movableType = context.typeBuilder().getMovableInterfaceType();
			
			// All member variables need to be movable.
			for (const auto& var: typeInstance->variables()) {
				const auto varType = var->constructType()->createNoTagType();
				requirePredicate = SEM::Predicate::And(std::move(requirePredicate), SEM::Predicate::Satisfies(varType, movableType));
			}
			
			// All variants need to be movable.
			for (const auto& variantTypeInstance: typeInstance->variants()) {
				const auto varType = variantTypeInstance->selfType()->createNoTagType();
				requirePredicate = SEM::Predicate::And(std::move(requirePredicate), SEM::Predicate::Satisfies(varType, movableType));
			}
			
			return requirePredicate;
		}
		
		SEM::Predicate getDefaultCopyPredicate(Context& context, const SEM::TypeInstance* const typeInstance, const String& propertyName) {
			// Types must be movable to be copyable.
			auto predicate = getDefaultCopyMovePredicate(context, typeInstance);
			
			// All member variables need to be copyable.
			for (const auto& var: typeInstance->variables()) {
				const auto varType = var->constructType()->withoutTags();
				const auto copyableType = getBuiltInType(context, propertyName, { varType });
				predicate = SEM::Predicate::And(std::move(predicate), SEM::Predicate::Satisfies(varType, copyableType));
			}
			
			// All variants need to be copyable.
			for (const auto& variantTypeInstance: typeInstance->variants()) {
				const auto varType = variantTypeInstance->selfType();
				const auto copyableType = getBuiltInType(context, propertyName, { varType });
				predicate = SEM::Predicate::And(std::move(predicate), SEM::Predicate::Satisfies(varType, copyableType));
			}
			
			return predicate;
		}
		
		SEM::Predicate getDefaultImplicitCopyNoExceptPredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			return getDefaultCopyPredicate(context, typeInstance, context.getCString("noexcept_implicit_copyable_t"));
		}
		
		SEM::Predicate getDefaultImplicitCopyRequirePredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			return getDefaultCopyPredicate(context, typeInstance, context.getCString("implicit_copyable_t"));
		}
		
		SEM::Predicate getDefaultExplicitCopyNoExceptPredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			return getDefaultCopyPredicate(context, typeInstance, context.getCString("noexcept_copyable_t"));
		}
		
		SEM::Predicate getDefaultExplicitCopyRequirePredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			return getDefaultCopyPredicate(context, typeInstance, context.getCString("copyable_t"));
		}
		
		SEM::Predicate getDefaultComparePredicate(Context& context, const SEM::TypeInstance* const typeInstance, const String& propertyName) {
			auto predicate = SEM::Predicate::True();
			
			// All member variables need to be copyable.
			for (const auto& var: typeInstance->variables()) {
				const auto varType = var->constructType()->withoutTags();
				const auto copyableType = getBuiltInType(context, propertyName, { varType });
				predicate = SEM::Predicate::And(std::move(predicate), SEM::Predicate::Satisfies(varType, copyableType));
			}
			
			// All variants need to be copyable.
			for (const auto& variantTypeInstance: typeInstance->variants()) {
				const auto varType = variantTypeInstance->selfType();
				const auto copyableType = getBuiltInType(context, propertyName, { varType });
				predicate = SEM::Predicate::And(std::move(predicate), SEM::Predicate::Satisfies(varType, copyableType));
			}
			
			return predicate;
		}
		
		SEM::Predicate getDefaultCompareNoExceptPredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			const auto propertyName = context.getCString("noexcept_comparable_t");
			return getDefaultComparePredicate(context, typeInstance, propertyName);
		}
		
		SEM::Predicate getDefaultCompareRequirePredicate(Context& context, const SEM::TypeInstance* const typeInstance) {
			const auto propertyName = context.getCString("comparable_t");
			return getDefaultComparePredicate(context, typeInstance, propertyName);
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultConstructorDecl(Context& context, SEM::TypeInstance* const typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(SEM::GlobalStructure::TypeInstance(*typeInstance),
			                                                             name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			// This method requires move, so add the move predicate.
			semFunction->setRequiresPredicate(SEM::Predicate::And(typeInstance->requiresPredicate().copy(), getDefaultMovePredicate(context, typeInstance)));
			
			semFunction->setMethod(true);
			semFunction->setStaticMethod(true);
			
			const bool isVarArg = false;
			const bool isDynamicMethod = false;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Default constructor only moves, and since moves never
			// throw the constructor never throws.
			auto noExceptPredicate = SEM::Predicate::True();
			
			auto constructTypes = typeInstance->constructTypes();
			
			std::vector<SEM::Var*> argVars;
			argVars.reserve(constructTypes.size());
			for (const auto constructType: constructTypes) {
				const auto lvalType = makeValueLvalType(context, constructType);
				argVars.push_back(SEM::Var::Basic(constructType, lvalType).release());
			}
			
			semFunction->setParameters(std::move(argVars));
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), typeInstance->selfType(), std::move(constructTypes)));
			return semFunction;
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultAlignMaskDecl(Context& context, SEM::TypeInstance* const typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(SEM::GlobalStructure::TypeInstance(*typeInstance),
			                                                             name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			semFunction->setRequiresPredicate(SEM::Predicate::And(typeInstance->requiresPredicate().copy(), getDefaultSizedTypePredicate(context, typeInstance)));
			
			semFunction->setMethod(true);
			semFunction->setStaticMethod(true);
			
			const bool isVarArg = false;
			const bool isDynamicMethod = false;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// alignmask never throws.
			auto noExceptPredicate = SEM::Predicate::True();
			
			const auto sizeType = context.typeBuilder().getSizeType();
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), sizeType, {}));
			return semFunction;
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultSizeOfDecl(Context& context, SEM::TypeInstance* const typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(SEM::GlobalStructure::TypeInstance(*typeInstance),
			                                                             name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			semFunction->setRequiresPredicate(SEM::Predicate::And(typeInstance->requiresPredicate().copy(), getDefaultSizedTypePredicate(context, typeInstance)));
			
			semFunction->setMethod(true);
			semFunction->setStaticMethod(true);
			
			const bool isVarArg = false;
			const bool isDynamicMethod = false;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// sizeof never throws.
			auto noExceptPredicate = SEM::Predicate::True();
			
			const auto sizeType = context.typeBuilder().getSizeType();
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), sizeType, {}));
			return semFunction;
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultDestroyDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(SEM::GlobalStructure::TypeInstance(*typeInstance),
			                                                             name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			semFunction->setRequiresPredicate(typeInstance->requiresPredicate().copy());
			
			semFunction->setMethod(true);
			
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Destructor never throws.
			auto noExceptPredicate = SEM::Predicate::True();
			
			const auto voidType = context.typeBuilder().getVoidType();
			
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), voidType, /*argTypes=*/{}));
			return semFunction;
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultMoveDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(SEM::GlobalStructure::TypeInstance(*typeInstance),
			                                                             name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			semFunction->setRequiresPredicate(SEM::Predicate::And(typeInstance->requiresPredicate().copy(), getDefaultMovePredicate(context, typeInstance)));
			
			semFunction->setMethod(true);
			
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Move never throws.
			auto noExceptPredicate = SEM::Predicate::True();
			
			const auto voidType = context.typeBuilder().getVoidType();
			const auto voidPtrType = getBuiltInType(context, context.getCString("ptr_t"), { voidType });
			
			const auto sizeType = context.typeBuilder().getSizeType();
			
			SEM::TypeArray argTypes;
			argTypes.reserve(2);
			argTypes.push_back(voidPtrType);
			argTypes.push_back(sizeType);
			
			std::vector<SEM::Var*> argVars;
			argVars.reserve(2);
			
			{
				const auto lvalType = makeValueLvalType(context, voidPtrType);
				argVars.push_back(SEM::Var::Basic(voidPtrType, lvalType).release());
			}
			
			{
				const auto lvalType = makeValueLvalType(context, sizeType);
				argVars.push_back(SEM::Var::Basic(sizeType, lvalType).release());
			}
			
			semFunction->setParameters(std::move(argVars));
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), voidType, std::move(argTypes)));
			return semFunction;
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultImplicitCopyDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(SEM::GlobalStructure::TypeInstance(*typeInstance),
			                                                             name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			semFunction->setRequiresPredicate(SEM::Predicate::And(typeInstance->requiresPredicate().copy(), getDefaultImplicitCopyRequirePredicate(context, typeInstance)));
			
			semFunction->setMethod(true);
			semFunction->setConstPredicate(SEM::Predicate::True());
			
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Default copy method may throw since it
			// may call child copy methods that throw.
			auto noExceptPredicate = getDefaultImplicitCopyNoExceptPredicate(context, typeInstance);
			
			// Implicit copy should return a notag() type.
			const auto returnType = typeInstance->selfType()->createNoTagType();
			
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), returnType, {}));
			return semFunction;
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultExplicitCopyDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(SEM::GlobalStructure::TypeInstance(*typeInstance),
			                                                             name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			semFunction->setRequiresPredicate(SEM::Predicate::And(typeInstance->requiresPredicate().copy(), getDefaultExplicitCopyRequirePredicate(context, typeInstance)));
			
			semFunction->setMethod(true);
			semFunction->setConstPredicate(SEM::Predicate::True());
			
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Default copy method may throw since it
			// may call child copy methods that throw.
			auto noExceptPredicate = getDefaultExplicitCopyNoExceptPredicate(context, typeInstance);
			
			// Implicit copy should return a notag() type.
			const auto returnType = typeInstance->selfType()->createNoTagType();
			
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), returnType, {}));
			return semFunction;
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultCompareDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(SEM::GlobalStructure::TypeInstance(*typeInstance),
			                                                             name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			semFunction->setRequiresPredicate(SEM::Predicate::And(typeInstance->requiresPredicate().copy(), getDefaultCompareRequirePredicate(context, typeInstance)));
			
			semFunction->setMethod(true);
			semFunction->setConstPredicate(SEM::Predicate::True());
			
			const bool isVarArg = false;
			const bool isDynamicMethod =true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Default compare method may throw since it
			// may call child compare methods that throw.
			auto noExceptPredicate = getDefaultCompareNoExceptPredicate(context, typeInstance);
			
			const auto selfType = typeInstance->selfType();
			const auto argType = createReferenceType(context, selfType->createTransitiveConstType(SEM::Predicate::True()));
			const auto compareResultType = getBuiltInType(context, context.getCString("compare_result_t"), {});
			
			SEM::TypeArray argTypes;
			argTypes.reserve(1);
			argTypes.push_back(argType);
			
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), compareResultType, std::move(argTypes)));
			semFunction->setParameters({ SEM::Var::Basic(argType, argType).release() });
			return semFunction;
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultSetDeadDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(SEM::GlobalStructure::TypeInstance(*typeInstance),
			                                                             name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			semFunction->setMethod(true);
			
			const bool isVarArg = false;
			const bool isDynamicMethod = true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Never throws.
			auto noExceptPredicate = SEM::Predicate::True();
			
			const auto voidType = context.typeBuilder().getVoidType();
			
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), voidType, {}));
			return semFunction;
		}
		
		std::unique_ptr<SEM::Function> CreateDefaultIsLiveDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name) {
			std::unique_ptr<SEM::Function> semFunction(new SEM::Function(SEM::GlobalStructure::TypeInstance(*typeInstance),
			                                                             name.copy(), typeInstance->moduleScope().copy()));
			semFunction->setDefault(true);
			
			semFunction->setDebugInfo(makeDefaultFunctionInfo(*typeInstance, *semFunction));
			
			semFunction->setMethod(true);
			semFunction->setConstPredicate(SEM::Predicate::True());
			
			const bool isVarArg = false;
			const bool isDynamicMethod =true;
			const bool isTemplatedMethod = !typeInstance->templateVariables().empty();
			
			// Never throws.
			auto noExceptPredicate = SEM::Predicate::True();
			
			const auto boolType = context.typeBuilder().getBoolType();
			
			semFunction->setType(SEM::FunctionType(SEM::FunctionAttributes(isVarArg, isDynamicMethod, isTemplatedMethod, std::move(noExceptPredicate)), boolType, {}));
			return semFunction;
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
		
		std::unique_ptr<SEM::Function>
		CreateDefaultMethodDecl(Context& context, SEM::TypeInstance* typeInstance,
		                        bool isStatic, const Name& name, const Debug::SourceLocation& location) {
			assert(!typeInstance->isClassDecl());
			assert(!typeInstance->isInterface());
			
			const auto canonicalName = CanonicalizeMethodName(name.last());
			if (canonicalName == "create") {
				if (!isStatic) {
					context.issueDiag(DefaultMethodMustBeStaticDiag(name),
					                  location);
				}
				
				return CreateDefaultConstructorDecl(context, typeInstance, name);
			} else if (canonicalName == "__destroy") {
				if (isStatic) {
					context.issueDiag(DefaultMethodMustBeNonStaticDiag(name),
					                  location);
				}
				
				return CreateDefaultDestroyDecl(context, typeInstance, name);
			} else if (canonicalName == "__moveto") {
				if (isStatic) {
					context.issueDiag(DefaultMethodMustBeNonStaticDiag(name),
					                  location);
				}
				
				return CreateDefaultMoveDecl(context, typeInstance, name);
			} else if (canonicalName == "implicitcopy") {
				if (isStatic) {
					context.issueDiag(DefaultMethodMustBeNonStaticDiag(name),
					                  location);
				}
				
				return CreateDefaultImplicitCopyDecl(context, typeInstance, name);
			} else if (canonicalName == "copy") {
				if (isStatic) {
					context.issueDiag(DefaultMethodMustBeNonStaticDiag(name),
					                  location);
				}
				
				return CreateDefaultExplicitCopyDecl(context, typeInstance, name);
			} else if (canonicalName == "compare") {
				if (isStatic) {
					context.issueDiag(DefaultMethodMustBeNonStaticDiag(name),
					                  location);
				}
				
				return CreateDefaultCompareDecl(context, typeInstance, name);
			} else if (canonicalName == "__setdead") {
				if (isStatic) {
					context.issueDiag(DefaultMethodMustBeNonStaticDiag(name),
					                  location);
				}
				
				return CreateDefaultSetDeadDecl(context, typeInstance, name);
			} else if (canonicalName == "__islive") {
				if (isStatic) {
					context.issueDiag(DefaultMethodMustBeNonStaticDiag(name),
					                  location);
				}
				
				return CreateDefaultIsLiveDecl(context, typeInstance, name);
			}
			
			context.issueDiag(UnknownDefaultMethodDiag(name.last()), location);
			return CreateDefaultConstructorDecl(context, typeInstance, name);
		}
		
		bool HasDefaultConstructor(Context& /*context*/, SEM::TypeInstance* const typeInstance) {
			return typeInstance->isDatatype() ||
				typeInstance->isException() ||
				typeInstance->isStruct() ||
				typeInstance->isUnion();
		}
		
		bool HasDefaultAlignMask(Context& context, SEM::TypeInstance* const typeInstance) {
			if (typeInstance->isInterface() || typeInstance->isPrimitive()) {
				return false;
			}
			
			// There's only a default method if the user
			// hasn't specified a custom method.
			return !typeInstance->hasFunction(context.getCString("__alignmask"));
		}
		
		bool HasDefaultSizeOf(Context& context, SEM::TypeInstance* const typeInstance) {
			if (typeInstance->isInterface() || typeInstance->isPrimitive()) {
				return false;
			}
			
			// There's only a default method if the user
			// hasn't specified a custom method.
			return !typeInstance->hasFunction(context.getCString("__sizeof"));
		}
		
		bool HasDefaultDestroy(Context& context, SEM::TypeInstance* const typeInstance) {
			if (typeInstance->isInterface() || typeInstance->isPrimitive()) {
				return false;
			}
			
			// There's only a default destructor if the user
			// hasn't specified a custom destructor.
			return !typeInstance->hasFunction(context.getCString("__destroy"));
		}
		
		bool HasDefaultMove(Context& context, SEM::TypeInstance* const typeInstance) {
			if (typeInstance->isInterface() || typeInstance->isPrimitive()) {
				return false;
			}
			
			// There's only a default move method if the user
			// hasn't specified a custom move method.
			return !typeInstance->hasFunction(context.getCString("__moveto"));
		}
		
		bool HasDefaultImplicitCopy(Context& /*context*/, SEM::TypeInstance* const typeInstance) {
			return typeInstance->isClassDef() ||
				typeInstance->isDatatype() ||
				typeInstance->isEnum() ||
				typeInstance->isException() ||
				typeInstance->isStruct() ||
				typeInstance->isUnion() ||
				typeInstance->isUnionDatatype();
		}
		
		bool HasDefaultExplicitCopy(Context& /*context*/, SEM::TypeInstance* const typeInstance) {
			return typeInstance->isClassDef() ||
				typeInstance->isDatatype() ||
				typeInstance->isEnum() ||
				typeInstance->isException() ||
				typeInstance->isStruct() ||
				typeInstance->isUnion() ||
				typeInstance->isUnionDatatype();
		}
		
		bool HasDefaultCompare(Context& /*context*/, SEM::TypeInstance* const typeInstance) {
			return typeInstance->isClassDef() ||
				typeInstance->isDatatype() ||
				typeInstance->isEnum() ||
				typeInstance->isException() ||
				typeInstance->isStruct() ||
				typeInstance->isUnionDatatype();
		}
		
		bool HasDefaultSetDead(Context& context, SEM::TypeInstance* const typeInstance) {
			if (typeInstance->isInterface() || typeInstance->isPrimitive()) {
				return false;
			}
			
			// There's only a default __setdead method if the user
			// hasn't specified a custom __setdead method.
			return !typeInstance->hasFunction(context.getCString("__setdead"));
		}
		
		bool HasDefaultIsLive(Context& context, SEM::TypeInstance* const typeInstance) {
			if (typeInstance->isInterface() || typeInstance->isPrimitive()) {
				return false;
			}
			
			// There's only a default islive method if the user
			// hasn't specified a custom islive method.
			return !typeInstance->hasFunction(context.getCString("__islive"));
		}
		
		void CreateDefaultConstructor(Context& /*context*/, SEM::TypeInstance* const typeInstance,
		                              SEM::Function* const /*function*/, const Debug::SourceLocation& /*location*/) {
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
		
		bool CreateDefaultImplicitCopy(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* /*function*/, const Debug::SourceLocation& location) {
			if (!supportsImplicitCopy(context, typeInstance->selfType())) {
				if (!typeInstance->isClassDef()) {
					// Auto-generated, so ignore the problem.
					return false;
				}
				context.issueDiag(CannotGenerateImplicitCopyDiag(), location);
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
		
		bool CreateDefaultExplicitCopy(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* /*function*/, const Debug::SourceLocation& location) {
			if (!supportsExplicitCopy(context, typeInstance->selfType())) {
				if (!typeInstance->isClassDef()) {
					// Auto-generated, so ignore the problem.
					return false;
				}
				context.issueDiag(CannotGenerateExplicitCopyDiag(), location);
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
		
		bool CreateDefaultCompare(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* /*function*/, const Debug::SourceLocation& location) {
			assert(!typeInstance->isUnion());
			if (!supportsCompare(context, typeInstance->selfType())) {
				if (!typeInstance->isClassDef()) {
					// Auto-generated, so ignore the problem.
					return false;
				}
				context.issueDiag(CannotGenerateCompareDiag(), location);
			}
			
			return true;
		}
		
		bool CreateDefaultMethod(Context& context, SEM::TypeInstance* const typeInstance, SEM::Function* const function, const Debug::SourceLocation& location) {
			assert(!typeInstance->isClassDecl() && !typeInstance->isInterface());
			assert(function->isDeclaration());
			
			PushScopeElement pushFunction(context.scopeStack(), ScopeElement::Function(*function));
			
			const auto& name = function->name();
			const auto canonicalName = CanonicalizeMethodName(name.last());
			if (canonicalName == "__moveto" ||
			    canonicalName == "__destroy" ||
			    canonicalName == "__dead" ||
			    canonicalName == "__islive") {
				// Generate by CodeGen.
				return true;
			} else if (canonicalName == "create") {
				assert(!typeInstance->isException());
				CreateDefaultConstructor(context, typeInstance, function, location);
				return true;
			} else if (canonicalName == "implicitcopy") {
				assert(HasDefaultImplicitCopy(context, typeInstance));
				return CreateDefaultImplicitCopy(context, typeInstance, function, location);
			} else if (canonicalName == "copy") {
				assert(HasDefaultExplicitCopy(context, typeInstance));
				return CreateDefaultExplicitCopy(context, typeInstance, function, location);
			} else if (canonicalName == "compare") {
				assert(HasDefaultCompare(context, typeInstance));
				return CreateDefaultCompare(context, typeInstance, function, location);
			} else {
				// Unknown default methods should be handled by now.
				return true;
			}
		}
		
	}
	
}

