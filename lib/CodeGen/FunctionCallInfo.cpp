#include <assert.h>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
		
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
		llvm::Function* genFunctionPtrStub(Module& module, llvm::Function* functionRefPtr, llvm::FunctionType* newFunctionType) {
			const auto stubIdPair = std::make_pair(functionRefPtr, newFunctionType);
			const auto iterator = module.functionPtrStubMap().find(stubIdPair);
			if (iterator != module.functionPtrStubMap().end()) {
				return iterator->second;
			}
			
			const auto oldFunctionType = llvm::cast<llvm::FunctionType>(functionRefPtr->getType()->getPointerElementType());
			
			const auto functionName = module.getCString("translateStub_") + functionRefPtr->getName();
			const auto llvmFunction = llvm::Function::Create(newFunctionType, llvm::Function::InternalLinkage, functionName.c_str(), module.getLLVMModulePtr());
			
			module.functionPtrStubMap().insert(std::make_pair(stubIdPair, llvmFunction));
			
			// Always inline if possible.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			const auto entryBB = llvm::BasicBlock::Create(module.getLLVMContext(), "", llvmFunction);
			llvm::IRBuilder<> builder(module.getLLVMContext());
			builder.SetInsertPoint(entryBB);
			
			assert(oldFunctionType->getNumParams() == newFunctionType->getNumParams() ||
				oldFunctionType->getNumParams() == (newFunctionType->getNumParams() + 1));
			
			const bool needsReturnVar = (oldFunctionType->getNumParams() != newFunctionType->getNumParams());
			
			assert(!(needsReturnVar && newFunctionType->getReturnType()->isVoidTy()));
			const auto returnVar = needsReturnVar ? builder.CreateAlloca(newFunctionType->getReturnType()) : nullptr;
			
			llvm::SmallVector<llvm::Value*, 10> args;
			
			if (needsReturnVar) {
				args.push_back(builder.CreatePointerCast(returnVar, TypeGenerator(module).getI8PtrType()));
			}
			
			size_t offset = needsReturnVar ? 1 : 0;
			
			for (auto it = llvmFunction->arg_begin(); it != llvmFunction->arg_end(); ++it) {
				const auto oldParamType = oldFunctionType->getParamType(offset);
				const auto newParamType = it->getType();
				
				if (oldParamType->isPointerTy()) {
					if (!newParamType->isPointerTy()) {
						assert(!newParamType->isVoidTy());
						const auto argAlloca = builder.CreateAlloca(newParamType);
						builder.CreateStore(it, argAlloca);
						args.push_back(builder.CreatePointerCast(argAlloca, oldParamType));
					} else {
						// Make sure our pointers have the right type.
						args.push_back(builder.CreatePointerCast(it, oldParamType));
					}
				} else {
					args.push_back(it);
				}
				offset++;
			}
			
			const auto result = builder.CreateCall(functionRefPtr, args);
			
			if (needsReturnVar) {
				builder.CreateRet(builder.CreateLoad(returnVar));
			} else {
				if (newFunctionType->getReturnType()->isVoidTy()) {
					builder.CreateRetVoid();
				} else {
					if (result->getType()->isPointerTy()) {
						builder.CreateRet(builder.CreatePointerCast(result, newFunctionType->getReturnType()));
					} else {
						builder.CreateRet(result);
					}
				}
			}
			
			return llvmFunction;
		}
		
		bool doFunctionTypesMatch(llvm::FunctionType* firstType, llvm::FunctionType* secondType) {
			if (firstType->getNumParams() != secondType->getNumParams()) {
				return false;
			}
			
			for (unsigned int i = 0; i < firstType->getNumParams(); i++) {
				const auto firstParamType = firstType->getParamType(i);
				const auto secondParamType = secondType->getParamType(i);
				
				if (firstParamType->isPointerTy() != secondParamType->isPointerTy()) {
					return false;
				}
			}
			
			return true;
		}
		
		llvm::Value* genFunctionPtr(Function& function, llvm::Function* functionRefPtr, llvm::Type* functionPtrType) {
			const auto oldFunctionType = llvm::cast<llvm::FunctionType>(functionRefPtr->getType()->getPointerElementType());
			const auto newFunctionType = llvm::cast<llvm::FunctionType>(functionPtrType->getPointerElementType());
			
			if (doFunctionTypesMatch(oldFunctionType, newFunctionType)) {
				return function.getBuilder().CreatePointerCast(functionRefPtr, functionPtrType);
			} else {
				// This case means that the function is templated on the
				// return type and a primitive has been passed for that
				// type argument, so the original function will have accepted
				// a return var but the function reference expects it
				// to return the primitive directly. We need to fix this
				// by creating a stub that translates between them.
				return genFunctionPtrStub(function.module(), functionRefPtr, newFunctionType);
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
			auto& module = function.module();
			const auto functionRefPtr = genFunctionDeclRef(module, parentType, semFunction);
			const auto functionPtrType = genFunctionType(module, functionType)->getPointerTo();
			
			// There may need to be a stub generated to handle issues with types being
			// passed/returned by value versus via a pointer (which happens when primitives
			// are used in templates).
			return genFunctionPtr(function, functionRefPtr, functionPtrType);
		}
		
		PendingResult genCallValue(Function& function, const SEM::Value& value) {
			switch (value.kind()) {
				case SEM::Value::CAST: {
					if (value.castOperand().type()->isDatatype()) {
						return PendingResult(genValue(function, value));
					} else {
						return genCallValue(function, value.castOperand());
					}
				}
				
				case SEM::Value::BIND_REFERENCE: {
					auto result = genCallValue(function, value.bindReferenceOperand());
					result.add(PendingAction::Bind(value.bindReferenceOperand().type()));
					return result;
				}
				
				default: {
					return PendingResult(genValue(function, value));
				}
			}
		}
		
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
		
		llvm::Value* genTrivialMethodCall(Function& function, const SEM::Value& value, llvm::ArrayRef<SEM::Value> valueArgs,
				Optional<PendingResult> contextValue, llvm::Value* const hintResultValue) {
			switch (value.kind()) {
				case SEM::Value::FUNCTIONREF: {
					PendingResultArray args;
					
					if (contextValue) {
						args.push_back(std::move(*contextValue));
					}
					
					for (const auto& valueArg: valueArgs) {
						args.push_back(genCallValue(function, valueArg));
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
					auto dataResult = genCallValue(function, dataValue);
					return genTrivialMethodCall(function, value.methodObject(), valueArgs, make_optional(std::move(dataResult)), hintResultValue);
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
					const auto functionPtrType = genFunctionType(module, value.type())->getPointerTo();
					callInfo.functionPtr = genFunctionPtr(function, functionRefPtr, functionPtrType);
					
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

