#include <cassert>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/FunctionTranslationStub.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SEMFunctionGenerator.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/VTable.hpp>

#include <locic/SEM/Function.hpp>
#include <locic/SEM/FunctionType.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/Value.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Function* genFunctionDeclRef(Module& module, const SEM::Type* const parentType, SEM::Function* function) {
			auto& semFunctionGenerator = module.semFunctionGenerator();
			if (parentType == nullptr) {
				return semFunctionGenerator.getDecl(nullptr,
				                                    *function);
			} else if (parentType->isObject()) {
				return semFunctionGenerator.getDecl(parentType->getObjectType(),
				                                    *function);
			} else {
				llvm_unreachable("Unknown parent type in function ref.");
			}
		}
		
		llvm::Value* genFunctionRef(Function& function, const SEM::Type* parentType, SEM::Function* const semFunction, const SEM::FunctionType functionType) {
			auto& module = function.module();
			const auto functionRefPtr = genFunctionDeclRef(module, parentType, semFunction);
			
			// There may need to be a stub generated to handle issues with types being
			// passed/returned by value versus via a pointer (which happens when primitives
			// are used in templates).
			return genTranslatedFunctionPointer(function, functionRefPtr, semFunction->type(), functionType);
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
							value.type()->asFunctionType(), value.functionRefTemplateArguments().copy());
						return genTrivialPrimitiveFunctionCall(function, std::move(methodInfo), std::move(args), hintResultValue);
					}
					
					llvm_unreachable("Unknown trivial function.");
				}
				
				case SEM::Value::METHODOBJECT: {
					const auto& dataValue = value.methodOwner();
					const CallValuePendingResult dataResult(dataValue);
					const PendingResult dataPendingResult(dataResult);
					return genTrivialMethodCall(function, value.methodObject(), valueArgs, make_optional(dataPendingResult), hintResultValue);
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
					const auto functionType = value.type()->asFunctionType();
					
					// There may need to be a stub generated to handle issues with types being
					// passed/returned by value versus via a pointer (which happens when primitives
					// are used in templates).
					callInfo.functionPtr = genFunctionRef(function, parentType, value.functionRefFunction(), functionType);
					
					if (functionType.attributes().isTemplated()) {
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
					const auto functionType = value.templateFunctionRefFunctionType()->asFunctionType();
					
					// Template function references involve creating a stub that performs
					// a virtual call to the actual call. Since the virtual call mechanism
					// doesn't actually allow us to get a pointer to the function we want
					// to call, we need to create the stub and refer to that instead.
					auto& semFunctionGenerator = module.semFunctionGenerator();
					const auto functionRefPtr = semFunctionGenerator.genTemplateFunctionStub(parentType->getTemplateVar(),
					                                                                         functionName,
					                                                                         functionType,
					                                                                         function.getDebugLoc());
					callInfo.functionPtr = genTranslatedFunctionPointer(function, functionRefPtr, functionType, value.type()->asFunctionType());
					
					// The stub will extract the template variable's value for the
					// current function's template context, so we can just use the
					// current function's template generator.
					callInfo.templateGenerator = function.getTemplateGenerator();
					return callInfo;
				}
				
				case SEM::Value::METHODOBJECT: {
					assert(value.type()->isCallableMethodObject());
					
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
					assert(value.type()->isCallable());
					
					const auto llvmValue = genValue(function, value);
					
					FunctionCallInfo callInfo;
					
					callInfo.contextPointer = value.type()->isCallableMethodObject() ?
						builder.CreateExtractValue(llvmValue, { 0 }) :
						nullptr;
					
					const auto functionValue = value.type()->isCallableMethodObject() ?
						builder.CreateExtractValue(llvmValue, { 1 }) :
						llvmValue;
					
					const auto functionType = value.type()->asFunctionType();
					const bool isTemplatedMethod = functionType.attributes().isTemplated();
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
			assert(value.type()->isBuiltInInterfaceMethod() || value.type()->isBuiltInStaticInterfaceMethod());
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
					const bool isStatic = value.type()->isBuiltInStaticInterfaceMethod();
					return getVirtualMethodComponents(function, isStatic, genValue(function, value));
				}
			}
		}
		
	}
	
}

