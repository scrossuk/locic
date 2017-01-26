#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/CallEmitter.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Primitives/FunctionPtrPrimitive.hpp>
#include <locic/CodeGen/Routines.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/VTable.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		FunctionPtrPrimitive::FunctionPtrPrimitive(const AST::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) { }
		
		bool FunctionPtrPrimitive::isSizeAlwaysKnown(const TypeInfo& /*typeInfo*/,
		                                             llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool FunctionPtrPrimitive::isSizeKnownInThisModule(const TypeInfo& /*typeInfo*/,
		                                                   llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool FunctionPtrPrimitive::hasCustomDestructor(const TypeInfo& /*typeInfo*/,
		                                               llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		bool FunctionPtrPrimitive::hasCustomMove(const TypeInfo& /*typeInfo*/,
		                                         llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		llvm_abi::Type FunctionPtrPrimitive::getABIType(Module& module,
		                                                const llvm_abi::TypeBuilder& abiTypeBuilder,
		                                                llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			switch (typeInstance_.primitiveID()) {
				CASE_CALLABLE_ID(PrimitiveFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveMethodFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveVarArgFunctionPtr):
					return llvm_abi::PointerTy;
				CASE_CALLABLE_ID(PrimitiveTemplatedFunctionPtr):
				CASE_CALLABLE_ID(PrimitiveTemplatedMethodFunctionPtr): {
					std::vector<llvm_abi::Type> types;
					types.reserve(2);
					types.push_back(llvm_abi::PointerTy);
					types.push_back(templateGeneratorType(module));
					return abiTypeBuilder.getStructTy(types);
				}
				CASE_CALLABLE_ID(PrimitiveMethod): {
					std::vector<llvm_abi::Type> types;
					types.reserve(2);
					types.push_back(llvm_abi::PointerTy);
					types.push_back(getBasicPrimitiveABIType(module, PrimitiveMethodFunctionPtr0));
					return abiTypeBuilder.getStructTy(types);
				}
				CASE_CALLABLE_ID(PrimitiveTemplatedMethod): {
					std::vector<llvm_abi::Type> types;
					types.reserve(2);
					types.push_back(llvm_abi::PointerTy);
					types.push_back(getBasicPrimitiveABIType(module, PrimitiveTemplatedMethodFunctionPtr0));
					return abiTypeBuilder.getStructTy(types);
				}
				CASE_CALLABLE_ID(PrimitiveInterfaceMethod): {
					return interfaceMethodType(module);
				}
				CASE_CALLABLE_ID(PrimitiveStaticInterfaceMethod): {
					return staticInterfaceMethodType(module);
				}
				default:
					llvm_unreachable("Invalid functionptr primitive ID.");
			}
		}
		
		namespace {
			
			llvm::Value* genFunctionPtrNullMethod(Function& function, const AST::Type* const type) {
				auto& module = function.module();
				
				const auto abiType = genABIType(module, type);
				return ConstantGenerator(module).getNull(abiType);
			}
			
			llvm::Value* genFunctionPtrCopyMethod(Function& function, PendingResultArray args) {
				return args[0].resolveWithoutBind(function);
			}
			
			llvm::Value* genFunctionPtrCompareMethod(Function& function, PendingResultArray args) {
				auto& builder = function.getBuilder();
				auto& module = function.module();
				
				const auto methodOwner = args[0].resolveWithoutBind(function);
				const auto operand = args[1].resolveWithoutBind(function);
				
				const auto minusOneResult = ConstantGenerator(module).getI8(-1);
				const auto zeroResult = ConstantGenerator(module).getI8(0);
				const auto plusOneResult = ConstantGenerator(module).getI8(1);
				
				const auto isLessThan = builder.CreateICmpULT(methodOwner, operand);
				const auto isGreaterThan = builder.CreateICmpUGT(methodOwner, operand);
				
				return builder.CreateSelect(isLessThan, minusOneResult,
					builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
			}
			
			llvm::Value* genFunctionPtrSetDeadMethod(Function& function) {
				auto& module = function.module();
				// Do nothing.
				return ConstantGenerator(module).getVoidUndef();
			}
			
			llvm::Value* genFunctionPtrIsLiveMethod(Function& function) {
				auto& module = function.module();
				return ConstantGenerator(module).getBool(true);
			}
			
			llvm::Value* genFunctionPtrCallMethod(Function& function, const AST::Type* type, PendingResultArray args, llvm::Value* resultPtr) {
				const auto functionValue = args[0].resolveWithoutBind(function);
				
				FunctionCallInfo callInfo;
				if (type->isBuiltInTemplatedFunctionPtr()) {
					callInfo.functionPtr = function.getBuilder().CreateExtractValue(functionValue, { 0 });
					callInfo.templateGenerator = function.getBuilder().CreateExtractValue(functionValue, { 1 });
				} else {
					callInfo.functionPtr = functionValue;
				}
				
				PendingResultArray callArgs;
				for (size_t i = 1; i < args.size(); i++) {
					callArgs.push_back(std::move(args[i]));
				}
				
				IREmitter irEmitter(function);
				CallEmitter callEmitter(irEmitter);
				return callEmitter.emitNonVarArgsCall(type->asFunctionType(),
				                                      callInfo,
				                                      std::move(callArgs),
				                                      resultPtr);
			}
			
		}
		
		llvm::Value* FunctionPtrPrimitive::emitMethod(IREmitter& irEmitter,
		                                              const MethodID methodID,
		                                              llvm::ArrayRef<AST::Value> typeTemplateArguments,
		                                              llvm::ArrayRef<AST::Value> /*functionTemplateArguments*/,
		                                              PendingResultArray args,
		                                              llvm::Value* const resultPtr) const {
			auto& function = irEmitter.function();
			auto& module = irEmitter.module();
			
			AST::ValueArray valueArray;
			for (const auto& value: typeTemplateArguments) {
				valueArray.push_back(value.copy());
			}
			const auto type = AST::Type::Object(&typeInstance_,
			                                    std::move(valueArray));
			
			switch (methodID) {
				case METHOD_NULL:
					return genFunctionPtrNullMethod(function, type);
				case METHOD_ALIGNMASK: {
					const auto abiType = genABIType(module, type);
					return ConstantGenerator(module).getSizeTValue(module.abi().typeInfo().getTypeRequiredAlign(abiType).asBytes() - 1);
				}
				case METHOD_SIZEOF: {
					const auto abiType = genABIType(module, type);
					return ConstantGenerator(module).getSizeTValue(module.abi().typeInfo().getTypeAllocSize(abiType).asBytes());
				}
				case METHOD_COPY:
				case METHOD_IMPLICITCOPY:
				case METHOD_MOVE:
					return genFunctionPtrCopyMethod(function, std::move(args));
				case METHOD_COMPARE:
					return genFunctionPtrCompareMethod(function, std::move(args));
				case METHOD_SETDEAD:
					return genFunctionPtrSetDeadMethod(function);
				case METHOD_ISLIVE:
					return genFunctionPtrIsLiveMethod(function);
				case METHOD_CALL:
					return genFunctionPtrCallMethod(function,
					                                type,
					                                std::move(args),
					                                resultPtr);
				default:
					llvm_unreachable("Unknown function_ptr primitive method.");
			}
		}
		
	}
	
}

