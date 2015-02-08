#include <assert.h>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Function* genFunctionPtrStub(Module& module, llvm::Function* functionRefPtr, llvm::FunctionType* newFunctionType) {
			const auto stubIdPair = std::make_pair(functionRefPtr, newFunctionType);
			const auto iterator = module.functionPtrStubMap().find(stubIdPair);
			if (iterator != module.functionPtrStubMap().end()) {
				return iterator->second;
			}
			
			const auto oldFunctionType = llvm::cast<llvm::FunctionType>(functionRefPtr->getType()->getPointerElementType());
			
			const auto llvmFunction = llvm::Function::Create(newFunctionType, llvm::Function::PrivateLinkage, "translateFunctionStub", module.getLLVMModulePtr());
			
			module.functionPtrStubMap().insert(std::make_pair(stubIdPair, llvmFunction));
			
			// Always inline if possible.
			llvmFunction->addFnAttr(llvm::Attribute::AlwaysInline);
			
			const auto entryBB = llvm::BasicBlock::Create(module.getLLVMContext(), "", llvmFunction);
			llvm::IRBuilder<> builder(module.getLLVMContext());
			builder.SetInsertPoint(entryBB);
			
			assert(oldFunctionType->getNumParams() == newFunctionType->getNumParams() ||
				oldFunctionType->getNumParams() == (newFunctionType->getNumParams() + 1));
			
			const bool needsReturnVar = (oldFunctionType->getNumParams() != newFunctionType->getNumParams());
			
			const auto returnVar = needsReturnVar ? builder.CreateAlloca(newFunctionType->getReturnType()) : nullptr;
			
			llvm::SmallVector<llvm::Value*, 10> args;
			
			if (needsReturnVar) {
				args.push_back(builder.CreatePointerCast(returnVar, TypeGenerator(module).getI8PtrType()));
			}
			
			size_t offset = needsReturnVar ? 1 : 0;
			
			for (auto it = llvmFunction->arg_begin(); it != llvmFunction->arg_end(); ++it) {
				const auto oldParamType = oldFunctionType->getParamType(offset);
				const auto newParamType = it->getType();
				
				if (oldParamType->isPointerTy() && !newParamType->isPointerTy()) {
					const auto argAlloca = builder.CreateAlloca(newParamType);
					builder.CreateStore(it, argAlloca);
					args.push_back(builder.CreatePointerCast(argAlloca, TypeGenerator(module).getI8PtrType()));
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
		
		llvm::Function* genFunctionRef(Module& module, const SEM::Type* parentType, SEM::Function* function) {
			if (parentType == nullptr) {
				return genFunctionDecl(module, nullptr, function);
			} else if (parentType->isObject()) {
				return genFunctionDecl(module, parentType->getObjectType(), function);
			} else {
				llvm_unreachable("Unknown parent type in function ref.");
			}
		}
		
		ArgPair genCallValue(Function& function, const SEM::Value& value) {
			switch (value.kind()) {
				case SEM::Value::REFVALUE: {
					const auto& dataValue = *(value.refValue.value);
					const bool isContextRef = dataValue.type()->isBuiltInReference()
						|| !canPassByValue(function.module(), dataValue.type());
					return std::make_pair(genValue(function, dataValue), isContextRef);
				}
				
				default:
					const bool isContextRef = value.type()->isBuiltInReference()
						|| !canPassByValue(function.module(), value.type());
					return std::make_pair(genValue(function, value), isContextRef);
			}
		}
		
		bool isTrivialFunction(Module& module, const SEM::Value& value) {
			switch (value.kind()) {
				case SEM::Value::FUNCTIONREF: {
					return value.functionRef.function->isPrimitive();
				}
				
				case SEM::Value::METHODOBJECT: {
					return isTrivialFunction(module, *(value.methodObject.method));
				}
				
				default: {
					return false;
				}
			}
		}
		
		llvm::Value* genTrivialFunctionCall(Function& function, const SEM::Value& value, llvm::ArrayRef<SEM::Value> args, ArgPair contextValue) {
			switch (value.kind()) {
				case SEM::Value::FUNCTIONREF: {
					llvm::SmallVector<ArgPair, 10> llvmArgs;
					
					if (contextValue.first != nullptr) {
						llvmArgs.push_back(contextValue);
					}
					
					for (const auto& arg: args) {
						llvmArgs.push_back(genCallValue(function, arg));
					}
					
					if (value.functionRef.function->isPrimitive()) {
						return genTrivialPrimitiveFunctionCall(function, value.functionRef.parentType,
							value.functionRef.function, llvmArgs);
					}
					
					llvm_unreachable("Unknown trivial function.");
				}
				
				case SEM::Value::METHODOBJECT: {
					const auto& dataValue = *(value.methodObject.methodOwner);
					const auto llvmDataValue = genValue(function, dataValue);
					const bool isContextRef = dataValue.type()->isBuiltInReference()
						|| !canPassByValue(function.module(), dataValue.type());
					
					if (isContextRef && !dataValue.type()->isRef()) {
						// Call destructor for the object at the end of the current scope.
						scheduleDestructorCall(function, dataValue.type(), llvmDataValue);
					}
					
					return genTrivialFunctionCall(function, *(value.methodObject.method), args, std::make_pair(llvmDataValue, isContextRef));
				}
				
				default: {
					llvm_unreachable("Unknown trivial function value kind.");
				}
			}
		}
		
		FunctionCallInfo genFunctionCallInfo(Function& function, const SEM::Value& value) {
			auto& builder = function.getBuilder();
			auto& module = function.module();
			const auto debugLoc = getDebugLocation(function, value);
			
			switch (value.kind()) {
				case SEM::Value::FUNCTIONREF: {
					FunctionCallInfo callInfo;
					
					const auto parentType = value.functionRef.parentType;
					const auto functionRefPtr = genFunctionRef(module, parentType, value.functionRef.function);
					const auto functionPtrType = genFunctionType(module, value.type())->getPointerTo();
					
					// There may need to be a stub generated to handle issues with types being
					// passed/returned by value versus via a pointer (which happens when primitives
					// are used in templates).
					callInfo.functionPtr = genFunctionPtr(function, functionRefPtr, functionPtrType);
					
					if (value.type()->isFunctionTemplated()) {
						if (!value.functionRef.templateArguments.empty()) {
							// The function is templated on the function (and potentially also the parent object).
							const auto templateInst = TemplateInst::Function(parentType, value.functionRef.function, value.functionRef.templateArguments);
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
					
					const auto parentType = value.templateFunctionRef.parentType;
					const auto& functionName = value.templateFunctionRef.name;
					const auto functionType = value.templateFunctionRef.functionType;
					
					// Template function references involve creating a stub that performs
					// a virtual call to the actual call. Since the virtual call mechanism
					// doesn't actually allow us to get a pointer to the function we want
					// to call, we need to create the stub and refer to that instead.
					const auto functionRefPtr = genTemplateFunctionStub(module, parentType->getTemplateVar(), functionName, functionType);
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
					
					const auto functionCallInfo = genFunctionCallInfo(function, *(value.methodObject.method));
					
					callInfo.functionPtr = functionCallInfo.functionPtr;
					callInfo.templateGenerator = functionCallInfo.templateGenerator;
					
					const auto& dataValue = *(value.methodObject.methodOwner);
					const auto llvmDataValue = genValue(function, dataValue);
					
					// Methods must have a pointer to the object, which
					// may require generating a fresh 'alloca'.
					const auto dataPointer = genValuePtr(function, llvmDataValue, dataValue.type());
					
					assert(dataPointer != nullptr && "MethodObject requires a valid data pointer");
					
					callInfo.contextPointer = function.getBuilder().CreatePointerCast(dataPointer, TypeGenerator(module).getI8PtrType());
					
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
		
	}
	
}

