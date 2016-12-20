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
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
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
		
		llvm::Type* ValueLvalPrimitive::getIRType(Module& module,
		                                          const TypeGenerator& /*typeGenerator*/,
		                                          llvm::ArrayRef<AST::Value> templateArguments) const {
			return genType(module, templateArguments.front().typeRefType());
		}
		
		namespace {
			
			llvm::Value* genValueLvalDeadMethod(Function& functionGenerator, const AST::Type* const targetType, llvm::Value* const hintResultValue) {
				IREmitter irEmitter(functionGenerator);
				const auto objectVar = irEmitter.emitAlloca(targetType, hintResultValue);
				genSetDeadState(functionGenerator, targetType, objectVar);
				return irEmitter.emitMoveLoad(objectVar, targetType);
			}
			
			llvm::Value* genValueLvalCreateMethod(Function& functionGenerator, const AST::Type* const targetType,
			                                      PendingResultArray args, llvm::Value* const hintResultValue) {
				IREmitter irEmitter(functionGenerator);
				const auto objectVar = irEmitter.emitAlloca(targetType, hintResultValue);
				const auto operand = args[0].resolve(functionGenerator, hintResultValue);
				
				// Store the object.
				irEmitter.emitMoveStore(operand, objectVar, targetType);
				return irEmitter.emitMoveLoad(objectVar, targetType);
			}
			
			llvm::Value* genValueLvalCopyMethod(Function& functionGenerator,
			                                    const MethodID methodID,
			                                    const AST::Type* const targetType,
			                                    PendingResultArray args,
			                                    llvm::Value* const hintResultValue) {
				IREmitter irEmitter(functionGenerator);
				return irEmitter.emitCopyCall(methodID,
				                              args[0].resolve(functionGenerator),
				                              targetType,
				                              hintResultValue);
			}
			
			llvm::Value* genValueLvalSetDeadMethod(Function& functionGenerator, const AST::Type* const targetType, PendingResultArray args) {
				auto& module = functionGenerator.module();
				const auto methodOwner = args[0].resolve(functionGenerator);
				genSetDeadState(functionGenerator, targetType, methodOwner);
				return ConstantGenerator(module).getVoidUndef();
			}
			
			llvm::Value* genValueLvalMoveToMethod(Function& functionGenerator, const AST::Type* const targetType, PendingResultArray args) {
				auto& module = functionGenerator.module();
				
				const auto destValue = args[1].resolve(functionGenerator);
				const auto positionValue = args[2].resolve(functionGenerator);
				const auto sourceValue = args[0].resolve(functionGenerator);
				
				genMoveCall(functionGenerator, targetType, sourceValue, destValue, positionValue);
				return ConstantGenerator(module).getVoidUndef();
			}
			
			llvm::Value* genValueLvalAddressMethod(Function& functionGenerator, PendingResultArray args) {
				return args[0].resolve(functionGenerator);
			}
			
			llvm::Value* genValueLvalDissolveMethod(Function& functionGenerator, PendingResultArray args) {
				return args[0].resolve(functionGenerator);
			}
			
			llvm::Value* genValueLvalMoveMethod(Function& functionGenerator, const AST::Type* const targetType, PendingResultArray args) {
				const auto methodOwner = args[0].resolve(functionGenerator);
				
				IREmitter irEmitter(functionGenerator);
				return irEmitter.emitMoveLoad(methodOwner, targetType);
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
		                                            llvm::Value* const hintResultValue) const {
			auto& functionGenerator = irEmitter.function();
			const auto targetType = typeTemplateArguments.front().typeRefType();
			
			switch (methodID) {
				case METHOD_DEAD:
					return genValueLvalDeadMethod(functionGenerator, targetType, hintResultValue);
				case METHOD_CREATE:
					return genValueLvalCreateMethod(functionGenerator, targetType, std::move(args), hintResultValue);
				case METHOD_DESTROY: {
					auto& module = functionGenerator.module();
					irEmitter.emitDestructorCall(args[0].resolve(functionGenerator), targetType);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY:
					return genValueLvalCopyMethod(functionGenerator, methodID, targetType, std::move(args), hintResultValue);
				case METHOD_ALIGNMASK:
					return genAlignMask(functionGenerator, targetType);
				case METHOD_SIZEOF:
					return genSizeOf(functionGenerator, targetType);
				case METHOD_SETDEAD:
					return genValueLvalSetDeadMethod(functionGenerator, targetType, std::move(args));
				case METHOD_MOVETO:
					return genValueLvalMoveToMethod(functionGenerator, targetType, std::move(args));
				case METHOD_ADDRESS:
					return genValueLvalAddressMethod(functionGenerator, std::move(args));
				case METHOD_DISSOLVE:
					return genValueLvalDissolveMethod(functionGenerator, std::move(args));
				case METHOD_MOVE:
					return genValueLvalMoveMethod(functionGenerator, targetType, std::move(args));
				case METHOD_ASSIGN:
					return genValueLvalAssignMethod(functionGenerator, targetType, std::move(args));
				default:
					llvm_unreachable("Unknown primitive value_lval method.");
			}
		}
		
	}
	
}

