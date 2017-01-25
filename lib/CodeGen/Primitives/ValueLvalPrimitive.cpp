#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
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
#include <locic/CodeGen/LivenessEmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Primitives/ValueLvalPrimitive.hpp>
#include <locic/CodeGen/Routines.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/VTable.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		ValueLvalPrimitive::ValueLvalPrimitive(const AST::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) {
			(void) typeInstance_;
		}
		
		bool ValueLvalPrimitive::isSizeAlwaysKnown(const TypeInfo& typeInfo,
		                                           llvm::ArrayRef<AST::Value> templateArguments) const {
			return typeInfo.isSizeAlwaysKnown(templateArguments.front().typeRefType());
		}
		
		bool ValueLvalPrimitive::isSizeKnownInThisModule(const TypeInfo& typeInfo,
		                                                 llvm::ArrayRef<AST::Value> templateArguments) const {
			return typeInfo.isSizeKnownInThisModule(templateArguments.front().typeRefType());
		}
		
		bool ValueLvalPrimitive::hasCustomDestructor(const TypeInfo& typeInfo,
		                                             llvm::ArrayRef<AST::Value> templateArguments) const {
			return typeInfo.hasCustomDestructor(templateArguments.front().typeRefType());
		}
		
		bool ValueLvalPrimitive::hasCustomMove(const TypeInfo& typeInfo,
		                                       llvm::ArrayRef<AST::Value> templateArguments) const {
			return typeInfo.hasCustomMove(templateArguments.front().typeRefType());
		}
		
		llvm_abi::Type ValueLvalPrimitive::getABIType(Module& module,
		                                              const llvm_abi::TypeBuilder& /*abiTypeBuilder*/,
		                                              llvm::ArrayRef<AST::Value> templateArguments) const {
			return genABIType(module, templateArguments.front().typeRefType());
		}
		
		namespace {
			
			llvm::Value* genValueLvalDeadMethod(Function& functionGenerator, const AST::Type* const targetType, llvm::Value* const resultPtr) {
				IREmitter irEmitter(functionGenerator);
				const auto objectVar = irEmitter.emitAlloca(targetType, resultPtr);
				LivenessEmitter(irEmitter).emitSetDeadCall(targetType, objectVar);
				return irEmitter.emitLoad(objectVar, targetType);
			}
			
			llvm::Value* genValueLvalCreateMethod(Function& functionGenerator, const AST::Type* const /*targetType*/,
			                                      PendingResultArray args, llvm::Value* const resultPtr) {
				return args[0].resolve(functionGenerator, resultPtr);
			}
			
			llvm::Value* genValueLvalCopyMethod(Function& functionGenerator,
			                                    const MethodID methodID,
			                                    const AST::Type* const targetType,
			                                    PendingResultArray args,
			                                    llvm::Value* const resultPtr) {
				IREmitter irEmitter(functionGenerator);
				return irEmitter.emitCopyCall(methodID,
				                              args[0].resolve(functionGenerator),
				                              targetType,
				                              resultPtr);
			}
			
			llvm::Value* genValueLvalSetDeadMethod(Function& functionGenerator, const AST::Type* const targetType, PendingResultArray args) {
				auto& module = functionGenerator.module();
				const auto methodOwner = args[0].resolve(functionGenerator);
				IREmitter irEmitter(functionGenerator);
				LivenessEmitter(irEmitter).emitSetDeadCall(targetType, methodOwner);
				return ConstantGenerator(module).getVoidUndef();
			}
			
			llvm::Value* genValueLvalMoveMethod(Function& functionGenerator, const AST::Type* const targetType, PendingResultArray args, llvm::Value* resultPtr) {
				IREmitter irEmitter(functionGenerator);
				const auto thisPtr = args[0].resolve(functionGenerator);
				RefPendingResult thisPendingResult(thisPtr, targetType);
				return irEmitter.emitMoveCall(thisPendingResult, targetType, resultPtr);
			}
			
			llvm::Value*
			genValueLvalSetValueMethod(Function& functionGenerator,
			                           const AST::Type* const targetType,
			                           PendingResultArray args) {
				auto& module = functionGenerator.module();
				IREmitter irEmitter(functionGenerator);
				const auto operand = args[1].resolve(functionGenerator);
				const auto thisPtr = args[0].resolve(functionGenerator);
				irEmitter.emitMoveStore(operand, thisPtr, targetType);
				return ConstantGenerator(module).getVoidUndef();
			}
			
			llvm::Value* genValueLvalAddressMethod(Function& functionGenerator, PendingResultArray args) {
				return args[0].resolve(functionGenerator);
			}
			
			llvm::Value* genValueLvalDissolveMethod(Function& functionGenerator, PendingResultArray args) {
				return args[0].resolve(functionGenerator);
			}
			
			llvm::Value* genValueLvalAssignMethod(Function& functionGenerator, const AST::Type* const targetType, PendingResultArray args) {
				auto& module = functionGenerator.module();
				
				const auto operand = args[1].resolve(functionGenerator);
				const auto methodOwner = args[0].resolve(functionGenerator);
				
				IREmitter irEmitter(functionGenerator);
				irEmitter.emitDestructorCall(methodOwner, targetType);
				irEmitter.emitMoveStore(operand, methodOwner, targetType);
				return ConstantGenerator(module).getVoidUndef();
			}
			
		}
		
		llvm::Value* ValueLvalPrimitive::emitMethod(IREmitter& irEmitter,
		                                            const MethodID methodID,
		                                            llvm::ArrayRef<AST::Value> typeTemplateArguments,
		                                            llvm::ArrayRef<AST::Value> /*functionTemplateArguments*/,
		                                            PendingResultArray args,
		                                            llvm::Value* const resultPtr) const {
			auto& functionGenerator = irEmitter.function();
			const auto targetType = typeTemplateArguments.front().typeRefType();
			
			switch (methodID) {
				case METHOD_DEAD:
					return genValueLvalDeadMethod(functionGenerator, targetType, resultPtr);
				case METHOD_CREATE:
					return genValueLvalCreateMethod(functionGenerator, targetType, std::move(args), resultPtr);
				case METHOD_DESTROY: {
					auto& module = functionGenerator.module();
					irEmitter.emitDestructorCall(args[0].resolve(functionGenerator), targetType);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY:
					return genValueLvalCopyMethod(functionGenerator, methodID, targetType, std::move(args), resultPtr);
				case METHOD_ALIGNMASK:
					return genAlignMask(functionGenerator, targetType);
				case METHOD_SIZEOF:
					return genSizeOf(functionGenerator, targetType);
				case METHOD_SETDEAD:
					return genValueLvalSetDeadMethod(functionGenerator, targetType, std::move(args));
				case METHOD_MOVE:
				case METHOD_LVALMOVE:
					return genValueLvalMoveMethod(functionGenerator, targetType, std::move(args), resultPtr);
				case METHOD_SETVALUE:
					return genValueLvalSetValueMethod(functionGenerator, targetType, std::move(args));
				case METHOD_ADDRESS:
					return genValueLvalAddressMethod(functionGenerator, std::move(args));
				case METHOD_DISSOLVE:
					return genValueLvalDissolveMethod(functionGenerator, std::move(args));
				case METHOD_ASSIGN:
					return genValueLvalAssignMethod(functionGenerator, targetType, std::move(args));
				default:
					llvm_unreachable("Unknown primitive value_lval method.");
			}
		}
		
	}
	
}

