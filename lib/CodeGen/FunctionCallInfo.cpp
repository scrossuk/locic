#include <locic/CodeGen/FunctionCallInfo.hpp>

#include <cassert>

#include <locic/AST/Function.hpp>
#include <locic/AST/FunctionType.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/Value.hpp>

#include <locic/CodeGen/ASTFunctionGenerator.hpp>
#include <locic/CodeGen/CallEmitter.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionTranslationStub.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/PrimitiveFunctionEmitter.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/ValueEmitter.hpp>
#include <locic/CodeGen/VTable.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Function* genFunctionDeclRef(Module& module, const AST::Type* const parentType,
		                                   const AST::Function* function) {
			auto& astFunctionGenerator = module.astFunctionGenerator();
			if (parentType == nullptr) {
				return astFunctionGenerator.getDecl(nullptr,
				                                    *function);
			} else if (parentType->isObject()) {
				return astFunctionGenerator.getDecl(parentType->getObjectType(),
				                                    *function);
			} else {
				llvm_unreachable("Unknown parent type in function ref.");
			}
		}
		
		llvm::Value* genFunctionRef(Function& function, const AST::Type* parentType,
		                            const AST::Function* const astFunction, const AST::FunctionType functionType) {
			auto& module = function.module();
			const auto functionRefPtr = genFunctionDeclRef(module, parentType, astFunction);
			
			// There may need to be a stub generated to handle issues with types being
			// passed/returned by value versus via a pointer (which happens when primitives
			// are used in templates).
			return genTranslatedFunctionPointer(function, functionRefPtr, astFunction->type(), functionType);
		}
		
		class CallValuePendingResult: public PendingResultBase {
		public:
			CallValuePendingResult(const AST::Value& value)
			: value_(value) { }
			
			llvm::Value* generateValue(Function& function, llvm::Value* const resultPtr) const {
				IREmitter irEmitter(function);
				ValueEmitter valueEmitter(irEmitter);
				return valueEmitter.emitValue(value_, resultPtr);
			}
			
			llvm::Value* generateLoadedValue(Function& function) const {
				assert(value_.type()->isRef());
				if (value_.isCast() && !value_.castOperand().type()->isDatatype()) {
					// Some cast operations have no effect, so just pass
					// straight through them.
					return CallValuePendingResult(value_.castOperand()).generateLoadedValue(function);
				}
				
				IREmitter irEmitter(function);
				ValueEmitter valueEmitter(irEmitter);
				
				if (value_.isBindReference()) {
					// Just don't perform 'bind' operation and hence
					// avoid an unnecessary store-load pair.
					return valueEmitter.emitValue(value_.bindReferenceOperand());
				}
				
				const auto result = valueEmitter.emitValue(value_);
				
				return irEmitter.emitLoad(result, value_.type()->refTarget());
			}
			
		private:
			const AST::Value& value_;
			
		};
		
		bool isTrivialFunction(Module& module, const AST::Value& value) {
			switch (value.kind()) {
				case AST::Value::FUNCTIONREF: {
					return value.functionRefFunction().isPrimitive();
				}
				
				case AST::Value::METHODOBJECT: {
					return isTrivialFunction(module, value.methodObject());
				}
				
				default: {
					return false;
				}
			}
		}
		
		llvm::Value* genTrivialMethodCall(Function& function, const AST::Value& value,
		                                  llvm::ArrayRef<AST::Value> valueArgs,
		                                  Optional<PendingResult> contextValue,
		                                  llvm::Value* const resultPtr) {
			auto& module = function.module();
			
			switch (value.kind()) {
				case AST::Value::FUNCTIONREF: {
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
					
					if (!value.functionRefFunction().isPrimitive()) {
						llvm_unreachable("Can't perform trivial call to non-primitive function.");
					}
					
					const auto& methodName = value.functionRefFunction().fullName().last();
					const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
					
					IREmitter irEmitter(function);
					PrimitiveFunctionEmitter primitiveFunctionEmitter(irEmitter);
					return primitiveFunctionEmitter.emitFunction(methodID,
					                                             value.functionRefParentType(),
					                                             arrayRef(value.functionRefTemplateArguments()),
					                                             std::move(args),
					                                             resultPtr);
				}
				
				case AST::Value::METHODOBJECT: {
					const auto& dataValue = value.methodOwner();
					const CallValuePendingResult dataResult(dataValue);
					const PendingResult dataPendingResult(dataResult);
					return genTrivialMethodCall(function, value.methodObject(), valueArgs,
					                            make_optional(dataPendingResult), resultPtr);
				}
				
				default: {
					llvm_unreachable("Unknown trivial function value kind.");
				}
			}
		}
		
		llvm::Value* genTrivialFunctionCall(Function& function, const AST::Value& value, llvm::ArrayRef<AST::Value> valueArgs,
				llvm::Value* const resultPtr) {
			return genTrivialMethodCall(function, value, valueArgs, None, resultPtr);
		}
		
		FunctionCallInfo genFunctionCallInfo(Function& function, const AST::Value& value) {
			auto& builder = function.getBuilder();
			auto& module = function.module();
			
			IREmitter irEmitter(function);
			ValueEmitter valueEmitter(irEmitter);
			
			switch (value.kind()) {
				case AST::Value::FUNCTIONREF: {
					FunctionCallInfo callInfo;
					
					const auto parentType = value.functionRefParentType();
					const auto functionType = value.type()->asFunctionType();
					
					// There may need to be a stub generated to handle issues with types being
					// passed/returned by value versus via a pointer (which happens when primitives
					// are used in templates).
					callInfo.functionPtr = genFunctionRef(function, parentType, &(value.functionRefFunction()), functionType);
					
					if (functionType.attributes().isTemplated()) {
						if (!value.functionRefTemplateArguments().empty()) {
							// The function is templated on the function (and potentially also the parent object).
							const auto templateInst = TemplateInst::Function(parentType, &(value.functionRefFunction()), arrayRef(value.functionRefTemplateArguments()));
							callInfo.templateGenerator = getTemplateGenerator(function, templateInst);
						} else {
							// The function is only templated on the parent object.
							callInfo.templateGenerator = getTemplateGenerator(function, TemplateInst::Type(parentType));
						}
					}
					
					return callInfo;
				}
				
				case AST::Value::TEMPLATEFUNCTIONREF: {
					FunctionCallInfo callInfo;
					
					const auto parentType = value.templateFunctionRefParentType();
					const auto& functionName = value.templateFunctionRefName();
					const auto functionType = value.templateFunctionRefFunctionType()->asFunctionType();
					
					// Template function references involve creating a stub that performs
					// a virtual call to the actual call. Since the virtual call mechanism
					// doesn't actually allow us to get a pointer to the function we want
					// to call, we need to create the stub and refer to that instead.
					auto& astFunctionGenerator = module.astFunctionGenerator();
					const auto functionRefPtr = astFunctionGenerator.genTemplateFunctionStub(parentType->getTemplateVar(),
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
				
				case AST::Value::METHODOBJECT: {
					assert(value.type()->isCallableMethodObject());
					
					FunctionCallInfo callInfo;
					
					const auto functionCallInfo = genFunctionCallInfo(function, value.methodObject());
					
					callInfo.functionPtr = functionCallInfo.functionPtr;
					callInfo.templateGenerator = functionCallInfo.templateGenerator;
					
					const auto& dataRefValue = value.methodOwner();
					assert(dataRefValue.type()->isRef());
					
					const auto llvmDataRefValue = valueEmitter.emitValue(dataRefValue);
					
					assert(llvmDataRefValue != nullptr && "MethodObject requires a valid data pointer");
					
					callInfo.contextPointer = llvmDataRefValue;
					
					return callInfo;
				}
				
				default: {
					assert(value.type()->isCallable());
					
					const auto llvmValue = valueEmitter.emitValue(value);
					
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
		
		TypeInfoComponents genTypeInfoComponents(Function& function, const AST::Value& value) {
			auto& module = function.module();
			
			IREmitter irEmitter(function);
			ValueEmitter valueEmitter(irEmitter);
			
			switch (value.kind()) {
				case AST::Value::TYPEREF: {
					const auto targetType = value.typeRefType();
					const auto vtablePointer = genVTable(module, targetType->resolveAliases()->getObjectType());
					const auto templateGenerator = getTemplateGenerator(function, TemplateInst::Type(targetType));
					return TypeInfoComponents(vtablePointer, templateGenerator);
				}
				
				default: {
					return getTypeInfoComponents(function, valueEmitter.emitValue(value));
				}
			}
		}
		
		TypeInfoComponents genBoundTypeInfoComponents(Function& function, const AST::Value& value) {
			IREmitter irEmitter(function);
			ValueEmitter valueEmitter(irEmitter);
			
			switch (value.kind()) {
				case AST::Value::BIND_REFERENCE: {
					return genTypeInfoComponents(function, value.bindReferenceOperand());
				}
				
				default: {
					const auto typeInfoValue = irEmitter.emitRawLoad(valueEmitter.emitValue(value),
					                                                 typeInfoType(function.module()));
					return getTypeInfoComponents(function,
					                             typeInfoValue);
				}
			}
		}
		
		VirtualMethodComponents genVirtualMethodComponents(Function& function, const AST::Value& value) {
			assert(value.type()->isBuiltInInterfaceMethod() || value.type()->isBuiltInStaticInterfaceMethod());
			auto& module = function.module();
			
			IREmitter irEmitter(function);
			ValueEmitter valueEmitter(irEmitter);
			
			switch (value.kind()) {
				case AST::Value::INTERFACEMETHODOBJECT: {
					const auto& method = value.interfaceMethodObject();
					const auto methodOwner = valueEmitter.emitValue(value.interfaceMethodOwner());
					
					assert(method.kind() == AST::Value::FUNCTIONREF);
					
					const auto& interfaceFunction = method.functionRefFunction();
					const auto methodHash = CreateMethodNameHash(interfaceFunction.fullName().last());
					const auto methodHashValue = ConstantGenerator(module).getI64(methodHash);
					
					return VirtualMethodComponents(getVirtualObjectComponents(function, methodOwner), methodHashValue);
				}
				
				case AST::Value::STATICINTERFACEMETHODOBJECT: {
					const auto& method = value.staticInterfaceMethodObject();
					const auto typeInfoComponents = genBoundTypeInfoComponents(function, value.staticInterfaceMethodOwner());
					
					assert(method.kind() == AST::Value::FUNCTIONREF);
					
					const auto& interfaceFunction = method.functionRefFunction();
					
					const auto contextPointer = irEmitter.constantGenerator().getNullPointer();
					
					const auto methodHash = CreateMethodNameHash(interfaceFunction.fullName().last());
					const auto methodHashValue = ConstantGenerator(module).getI64(methodHash);
					
					return VirtualMethodComponents(VirtualObjectComponents(typeInfoComponents, contextPointer), methodHashValue);
				}
				
				default: {
					const bool isStatic = value.type()->isBuiltInStaticInterfaceMethod();
					return getVirtualMethodComponents(function, isStatic,
					                                  valueEmitter.emitValue(value));
				}
			}
		}
		
	}
	
}

