#include <assert.h>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
		
		static bool checkImplies(const bool a, const bool b) {
			return !a || b;
		}
		
		/**
		 * \brief Create translation function stub.
		 * 
		 * This creates a function that performs a translation between
		 * accepting/returning values by pointer versus by value.
		 * 
		 * For example:
		 * 
		 * template <typename T : movable>
		 * void f(T value);
		 * 
		 * f<int>(10);
		 * 
		 * Here the function is called with an integer value and as
		 * with all primitives it will be passed by value. However
		 * the function is templated and therefore must accept its
		 * argument by pointer (to support non-primitive types).
		 * 
		 * This problem is resolved by creating a translation
		 * function stub that is roughly:
		 * 
		 * void translateFunctionForF(int value) {
		 *   int* stackMemoryPtr = alloca(sizeof(int));
		 *   *stackMemoryPtr = value;
		 *   f(stackMemoryPtr);
		 * }
		 * 
		 * This translation could be performed inline while the
		 * code is generated, but this would prevent referencing
		 * the function without calling it; for example:
		 * 
		 * auto theFunction = f<int>;
		 * theFunction(10);
		 * 
		 * It is of course notable that the function reference
		 * here actually refers to the *stub* and not to the
		 * templated function.
		 * 
		 * \param module The module in which code is being generated.
		 * \param functionRefPtr The function we ultimately want to call.
		 * \param newFunctionType The function signature we'd like to use.
		 * \return The translation function stub.
		 */
		llvm::Function* genFunctionPtrStub(Module& module, llvm::Function* functionRefPtr,
		                                   const SEM::Type* const oldFunctionType,
		                                   const SEM::Type* const newFunctionType) {
			assert(oldFunctionType->isFunction());
			assert(newFunctionType->isFunction());
			
			const auto llvmNewFunctionType = genFunctionType(module, newFunctionType);
			
			const auto stubIdPair = std::make_pair(functionRefPtr, llvmNewFunctionType);
			const auto iterator = module.functionPtrStubMap().find(stubIdPair);
			if (iterator != module.functionPtrStubMap().end()) {
				return iterator->second;
			}
			
			const auto functionName = module.getCString("translateStub_") + functionRefPtr->getName();
			const auto linkage = llvm::Function::InternalLinkage;
			
			const auto oldArgInfo = getFunctionArgInfo(module, oldFunctionType);
			const auto newArgInfo = getFunctionArgInfo(module, newFunctionType);
			const auto llvmFunction = createLLVMFunction(module, newArgInfo, linkage, functionName);
			
			module.functionPtrStubMap().insert(std::make_pair(stubIdPair, llvmFunction));
			
			// Always inline if possible.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			Function functionGenerator(module, *llvmFunction, newArgInfo);
			auto& builder = functionGenerator.getBuilder();
			
			const auto& oldParameterTypes = oldFunctionType->getFunctionParameterTypes();
			const auto& newParameterTypes = newFunctionType->getFunctionParameterTypes();
			assert(oldParameterTypes.size() == newParameterTypes.size());
			
			const auto returnVar =
				oldArgInfo.hasReturnVarArgument() ?
					newArgInfo.hasReturnVarArgument() ?
						functionGenerator.getReturnVar() :
						genAlloca(functionGenerator, newFunctionType->getFunctionReturnType())
					: nullptr;
			
			llvm::SmallVector<llvm::Value*, 10> args;
			
			if (oldArgInfo.hasReturnVarArgument()) {
				const auto llvmOldReturnType = genArgType(module, oldFunctionType->getFunctionReturnType());
				args.push_back(builder.CreatePointerCast(returnVar, llvmOldReturnType));
			}
			
			if (newArgInfo.hasTemplateGeneratorArgument()) {
				args.push_back(functionGenerator.getTemplateGenerator());
			}
			
			if (newArgInfo.hasContextArgument()) {
				args.push_back(functionGenerator.getRawContextValue());
			}
			
			for (size_t i = 0; i < oldParameterTypes.size(); i++) {
				const auto oldParameterType = oldParameterTypes[i];
				const auto newParameterType = newParameterTypes[i];
				
				const auto llvmOldParameterType = genArgType(module, oldParameterType);
				const auto llvmNewParameterType = genArgType(module, newParameterType);
				
				// Being able to pass the old parameter type by value must imply
				// that the new parameter type can be passed by value.
				assert(checkImplies(canPassByValue(module, oldParameterType), canPassByValue(module, newParameterType)));
				
				const auto argValue = functionGenerator.getArg(i);
				
				if (!canPassByValue(module, oldParameterType) && canPassByValue(module, newParameterType)) {
					// Create an alloca to hold the parameter so it can be passed by pointer
					// into the target function.
					const auto argAlloca = genAlloca(functionGenerator, newParameterType);
					genStore(functionGenerator, argValue, argAlloca, newParameterType);
					args.push_back(builder.CreatePointerCast(argAlloca, llvmOldParameterType));
				} else if (llvmOldParameterType->isPointerTy() && llvmNewParameterType->isPointerTy()) {
					// Make sure our pointers have the right type.
					args.push_back(builder.CreatePointerCast(argValue, llvmOldParameterType));
				} else {
					args.push_back(argValue);
				}
			}
			
			const auto result = genRawFunctionCall(functionGenerator, oldArgInfo, functionRefPtr, args);
			
			if (oldArgInfo.hasReturnVarArgument() && !newArgInfo.hasReturnVarArgument()) {
				builder.CreateRet(builder.CreateLoad(returnVar));
			} else {
				if (llvmNewFunctionType->getReturnType()->isVoidTy()) {
					builder.CreateRetVoid();
				} else {
					if (result->getType()->isPointerTy()) {
						builder.CreateRet(builder.CreatePointerCast(result, llvmNewFunctionType->getReturnType()));
					} else {
						builder.CreateRet(result);
					}
				}
			}
			
			return llvmFunction;
		}
		
		bool areTypesFunctionallyEquivalent(Module& module,
		                                    const SEM::Type* const firstType,
		                                    const SEM::Type* const secondType) {
			return firstType == secondType || genArgType(module, firstType) == genArgType(module, secondType);
		}
		
		bool doFunctionTypesMatch(Module& module,
		                          const SEM::Type* const firstType,
		                          const SEM::Type* const secondType) {
			assert(firstType->isFunction());
			assert(secondType->isFunction());
			assert(firstType->isFunctionTemplated() == secondType->isFunctionTemplated());
			
			const auto& firstParameterTypes = firstType->getFunctionParameterTypes();
			const auto& secondParameterTypes = secondType->getFunctionParameterTypes();
			assert(firstParameterTypes.size() == secondParameterTypes.size());
			
			for (size_t i = 0; i < firstParameterTypes.size(); i++) {
				const auto& firstParameterType = firstParameterTypes[i];
				const auto& secondParameterType = secondParameterTypes[i];
				if (!areTypesFunctionallyEquivalent(module, firstParameterType, secondParameterType)) {
					return false;
				}
			}
			
			return areTypesFunctionallyEquivalent(module, firstType->getFunctionReturnType(), secondType->getFunctionReturnType());
		}
		
		llvm::Value* genFunctionPtr(Function& function, llvm::Function* functionRefPtr,
		                            const SEM::Type* const oldFunctionType,
		                            const SEM::Type* const newFunctionType) {
			assert(oldFunctionType->isFunction());
			assert(newFunctionType->isFunction());
			assert(oldFunctionType->isFunctionTemplated() == newFunctionType->isFunctionTemplated());
			
			auto& module = function.module();
			
			if (doFunctionTypesMatch(module, oldFunctionType, newFunctionType)) {
				const auto functionPtrType = genFunctionType(module, newFunctionType)->getPointerTo();
				return function.getBuilder().CreatePointerCast(functionRefPtr, functionPtrType);
			} else {
				// This case means that the function is templated on the
				// return type and a primitive has been passed for that
				// type argument, so the original function will have accepted
				// a return var but the function reference expects it
				// to return the primitive directly. We need to fix this
				// by creating a stub that translates between them.
				return genFunctionPtrStub(module, functionRefPtr, oldFunctionType, newFunctionType);
			}
		}
		
		llvm::Function* genFunctionDeclRef(Module& module, const SEM::Type* parentType, SEM::Function* function) {
			if (parentType == nullptr) {
				return genFunctionDecl(module, nullptr, function);
			} else if (parentType->isObject()) {
				return genFunctionDecl(module, parentType->getObjectType(), function);
			} else {
				llvm_unreachable("Unknown parent type in function ref.");
			}
		}
		
		llvm::Value* genFunctionRef(Function& function, const SEM::Type* parentType, SEM::Function* const semFunction, const SEM::Type* const functionType) {
			assert(functionType->isFunction());
			
			auto& module = function.module();
			const auto functionRefPtr = genFunctionDeclRef(module, parentType, semFunction);
			
			// There may need to be a stub generated to handle issues with types being
			// passed/returned by value versus via a pointer (which happens when primitives
			// are used in templates).
			return genFunctionPtr(function, functionRefPtr, semFunction->type(), functionType);
		}
		
		class CallValuePendingResult: public PendingResultBase {
		public:
			CallValuePendingResult(const SEM::Value& value)
			: value_(value) { }
			
			llvm::Value* generateValue(Function& function, llvm::Value* const hintResultValue) const {
				return genValue(function, value_, hintResultValue);
			}
			
			llvm::Value* generateLoadedValue(Function& function) const {
				assert(value_.type()->isRef());
				if (value_.isCast() && !value_.castOperand().type()->isDatatype()) {
					// Some cast operations have no effect, so just pass
					// straight through them.
					return CallValuePendingResult(value_.castOperand()).generateLoadedValue(function);
				}
				
				if (value_.isBindReference()) {
					// Just don't perform 'bind' operation and hence
					// avoid an unnecessary store-load pair.
					return genValue(function, value_.bindReferenceOperand());
				}
				
				const auto result = genValue(function, value_);
				return genMoveLoad(function, result, value_.type()->refTarget());
			}
			
		private:
			const SEM::Value& value_;
			
		};
		
		bool isTrivialFunction(Module& module, const SEM::Value& value) {
			switch (value.kind()) {
				case SEM::Value::FUNCTIONREF: {
					return value.functionRefFunction()->isPrimitive();
				}
				
				case SEM::Value::METHODOBJECT: {
					return isTrivialFunction(module, value.methodObject());
				}
				
				default: {
					return false;
				}
			}
		}
		
		llvm::Value* genTrivialMethodCall(Function& function, const SEM::Value& value,
		                                  llvm::ArrayRef<SEM::Value> valueArgs,
		                                  Optional<PendingResult> contextValue,
		                                  llvm::Value* const hintResultValue) {
			switch (value.kind()) {
				case SEM::Value::FUNCTIONREF: {
					PendingResultArray args;
					
					if (contextValue) {
						args.push_back(*contextValue);
					}
					
					// Need an array to store all the pending results
					// being referred to in 'genTrivialPrimitiveFunctionCall'.
					Array<CallValuePendingResult, 10> pendingResultArgs;
					
					for (const auto& valueArg: valueArgs) {
						pendingResultArgs.push_back(CallValuePendingResult(valueArg));
						args.push_back(pendingResultArgs.back());
					}
					
					if (value.functionRefFunction()->isPrimitive()) {
						MethodInfo methodInfo(value.functionRefParentType(), value.functionRefFunction()->name().last(),
							value.type(), value.functionRefTemplateArguments().copy());
						return genTrivialPrimitiveFunctionCall(function, std::move(methodInfo), std::move(args), hintResultValue);
					}
					
					llvm_unreachable("Unknown trivial function.");
				}
				
				case SEM::Value::METHODOBJECT: {
					const auto& dataValue = value.methodOwner();
					const PendingResult dataResult = CallValuePendingResult(dataValue);
					return genTrivialMethodCall(function, value.methodObject(), valueArgs, make_optional(dataResult), hintResultValue);
				}
				
				default: {
					llvm_unreachable("Unknown trivial function value kind.");
				}
			}
		}
		
		llvm::Value* genTrivialFunctionCall(Function& function, const SEM::Value& value, llvm::ArrayRef<SEM::Value> valueArgs,
				llvm::Value* const hintResultValue) {
			return genTrivialMethodCall(function, value, valueArgs, None, hintResultValue);
		}
		
		FunctionCallInfo genFunctionCallInfo(Function& function, const SEM::Value& value) {
			auto& builder = function.getBuilder();
			auto& module = function.module();
			
			switch (value.kind()) {
				case SEM::Value::FUNCTIONREF: {
					FunctionCallInfo callInfo;
					
					const auto parentType = value.functionRefParentType();
					
					// There may need to be a stub generated to handle issues with types being
					// passed/returned by value versus via a pointer (which happens when primitives
					// are used in templates).
					callInfo.functionPtr = genFunctionRef(function, parentType, value.functionRefFunction(), value.type());
					
					if (value.type()->isFunctionTemplated()) {
						if (!value.functionRefTemplateArguments().empty()) {
							// The function is templated on the function (and potentially also the parent object).
							const auto templateInst = TemplateInst::Function(parentType, value.functionRefFunction(), arrayRef(value.functionRefTemplateArguments()));
							callInfo.templateGenerator = getTemplateGenerator(function, templateInst);
						} else {
							// The function is only templated on the parent object.
							callInfo.templateGenerator = getTemplateGenerator(function, TemplateInst::Type(parentType));
						}
					}
					
					return callInfo;
				}
				
				case SEM::Value::TEMPLATEFUNCTIONREF: {
					FunctionCallInfo callInfo;
					
					const auto parentType = value.templateFunctionRefParentType();
					const auto& functionName = value.templateFunctionRefName();
					const auto functionType = value.templateFunctionRefFunctionType();
					
					// Template function references involve creating a stub that performs
					// a virtual call to the actual call. Since the virtual call mechanism
					// doesn't actually allow us to get a pointer to the function we want
					// to call, we need to create the stub and refer to that instead.
					const auto functionRefPtr = genTemplateFunctionStub(module, parentType->getTemplateVar(), functionName, functionType, function.getDebugLoc());
					callInfo.functionPtr = genFunctionPtr(function, functionRefPtr, functionType, value.type());
					
					// The stub will extract the template variable's value for the
					// current function's template context, so we can just use the
					// current function's template generator.
					callInfo.templateGenerator = function.getTemplateGenerator();
					return callInfo;
				}
				
				case SEM::Value::METHODOBJECT: {
					assert(value.type()->isMethod());
					
					FunctionCallInfo callInfo;
					
					const auto functionCallInfo = genFunctionCallInfo(function, value.methodObject());
					
					callInfo.functionPtr = functionCallInfo.functionPtr;
					callInfo.templateGenerator = functionCallInfo.templateGenerator;
					
					const auto& dataRefValue = value.methodOwner();
					assert(dataRefValue.type()->isRef() && dataRefValue.type()->isBuiltInReference());
					
					const auto llvmDataRefValue = genValue(function, dataRefValue);
					
					assert(llvmDataRefValue != nullptr && "MethodObject requires a valid data pointer");
					
					callInfo.contextPointer = function.getBuilder().CreatePointerCast(llvmDataRefValue, TypeGenerator(module).getI8PtrType());
					
					return callInfo;
				}
				
				default: {
					assert(value.type()->isFunction() || value.type()->isMethod());
					
					const auto llvmValue = genValue(function, value);
					
					FunctionCallInfo callInfo;
					
					callInfo.contextPointer = value.type()->isMethod() ?
						builder.CreateExtractValue(llvmValue, { 0 }) :
						nullptr;
					
					const auto functionValue = value.type()->isMethod() ?
						builder.CreateExtractValue(llvmValue, { 1 }) :
						llvmValue;
					
					const auto functionType = value.type()->isMethod() ?
						value.type()->getMethodFunctionType() : value.type();
					const bool isTemplatedMethod = functionType->isFunctionTemplated();
					callInfo.functionPtr = isTemplatedMethod ? builder.CreateExtractValue(functionValue, { 0 }) : functionValue;
					callInfo.templateGenerator = isTemplatedMethod ? builder.CreateExtractValue(functionValue, { 1 }) : nullptr;
					return callInfo;
				}
			}
		}
		
		TypeInfoComponents genTypeInfoComponents(Function& function, const SEM::Value& value) {
			auto& module = function.module();
			
			switch (value.kind()) {
				case SEM::Value::TYPEREF: {
					const auto targetType = value.typeRefType();
					const auto vtablePointer = genVTable(module, targetType->resolveAliases()->getObjectType());
					const auto templateGenerator = getTemplateGenerator(function, TemplateInst::Type(targetType));
					return TypeInfoComponents(vtablePointer, templateGenerator);
				}
				
				default: {
					return getTypeInfoComponents(function, genValue(function, value));
				}
			}
		}
		
		TypeInfoComponents genBoundTypeInfoComponents(Function& function, const SEM::Value& value) {
			switch (value.kind()) {
				case SEM::Value::BIND_REFERENCE: {
					return genTypeInfoComponents(function, value.bindReferenceOperand());
				}
				
				default: {
					return getTypeInfoComponents(function, function.getBuilder().CreateLoad(genValue(function, value)));
				}
			}
		}
		
		VirtualMethodComponents genVirtualMethodComponents(Function& function, const SEM::Value& value) {
			assert(value.type()->isInterfaceMethod() || value.type()->isStaticInterfaceMethod());
			auto& module = function.module();
			
			switch (value.kind()) {
				case SEM::Value::INTERFACEMETHODOBJECT: {
					const auto& method = value.interfaceMethodObject();
					const auto methodOwner = genValue(function, value.interfaceMethodOwner());
					
					assert(method.kind() == SEM::Value::FUNCTIONREF);
					
					const auto interfaceFunction = method.functionRefFunction();
					const auto methodHash = CreateMethodNameHash(interfaceFunction->name().last());
					const auto methodHashValue = ConstantGenerator(module).getI64(methodHash);
					
					return VirtualMethodComponents(getVirtualObjectComponents(function, methodOwner), methodHashValue);
				}
				
				case SEM::Value::STATICINTERFACEMETHODOBJECT: {
					const auto& method = value.staticInterfaceMethodObject();
					const auto typeInfoComponents = genBoundTypeInfoComponents(function, value.staticInterfaceMethodOwner());
					
					assert(method.kind() == SEM::Value::FUNCTIONREF);
					
					const auto interfaceFunction = method.functionRefFunction();
					
					const auto contextPointer = ConstantGenerator(function.module()).getNull(TypeGenerator(function.module()).getI8PtrType());
					
					const auto methodHash = CreateMethodNameHash(interfaceFunction->name().last());
					const auto methodHashValue = ConstantGenerator(module).getI64(methodHash);
					
					return VirtualMethodComponents(VirtualObjectComponents(typeInfoComponents, contextPointer), methodHashValue);
				}
				
				default: {
					const bool isStatic = value.type()->isStaticInterfaceMethod();
					return getVirtualMethodComponents(function, isStatic, genValue(function, value));
				}
			}
		}
		
	}
	
}

