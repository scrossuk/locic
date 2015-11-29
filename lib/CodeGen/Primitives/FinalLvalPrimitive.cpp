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
#include <locic/CodeGen/Primitives/FinalLvalPrimitive.hpp>
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
		
		FinalLvalPrimitive::FinalLvalPrimitive(const SEM::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) {
			(void) typeInstance_;
		}
		
		bool FinalLvalPrimitive::isSizeAlwaysKnown(const TypeInfo& typeInfo,
		                                           llvm::ArrayRef<SEM::Value> templateArguments) const {
			assert(templateArguments.size() == 1);
			return typeInfo.isSizeAlwaysKnown(templateArguments.front().typeRefType());
		}
		
		bool FinalLvalPrimitive::isSizeKnownInThisModule(const TypeInfo& typeInfo,
		                                                 llvm::ArrayRef<SEM::Value> templateArguments) const {
			assert(templateArguments.size() == 1);
			return typeInfo.isSizeKnownInThisModule(templateArguments.front().typeRefType());
		}
		
		bool FinalLvalPrimitive::hasCustomDestructor(const TypeInfo& typeInfo,
		                                             llvm::ArrayRef<SEM::Value> templateArguments) const {
			return typeInfo.hasCustomDestructor(templateArguments.front().typeRefType());
		}
		
		bool FinalLvalPrimitive::hasCustomMove(const TypeInfo& typeInfo,
		                                       llvm::ArrayRef<SEM::Value> templateArguments) const {
			return typeInfo.hasCustomMove(templateArguments.front().typeRefType());
		}
		
		llvm_abi::Type FinalLvalPrimitive::getABIType(Module& module,
		                                              const llvm_abi::TypeBuilder& /*abiTypeBuilder*/,
		                                              llvm::ArrayRef<SEM::Value> templateArguments) const {
			assert(templateArguments.size() == 1);
			return genABIType(module, templateArguments.front().typeRefType());
		}
		
		llvm::Type* FinalLvalPrimitive::getIRType(Module& module,
		                                          const TypeGenerator& /*typeGenerator*/,
		                                          llvm::ArrayRef<SEM::Value> templateArguments) const {
			assert(templateArguments.size() == 1);
			return genType(module, templateArguments.front().typeRefType());
		}
		
		llvm::Value* FinalLvalPrimitive::emitMethod(IREmitter& irEmitter,
		                                            const MethodID methodID,
		                                            llvm::ArrayRef<SEM::Value> typeTemplateArguments,
		                                            llvm::ArrayRef<SEM::Value> /*functionTemplateArguments*/,
		                                            PendingResultArray args) const {
			auto& function = irEmitter.function();
			auto& module = irEmitter.module();
			
			const auto targetType = typeTemplateArguments.front().typeRefType();
			
			switch (methodID) {
				case METHOD_EMPTY: {
					const auto objectVar = irEmitter.emitReturnAlloca(targetType);
					genSetDeadState(function, targetType, objectVar);
					return genMoveLoad(function, objectVar, targetType);
				}
				case METHOD_ALIGNMASK: {
					return genAlignMask(function, targetType);
				}
				case METHOD_SIZEOF: {
					return genSizeOf(function, targetType);
				}
				case METHOD_DESTROY: {
					irEmitter.emitDestructorCall(args[0].resolve(function),
					                             targetType);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_MOVETO: {
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					const auto methodOwner = args[0].resolve(function);
					
					const auto destPtr = irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
					                                               moveToPtr,
					                                               moveToPosition);
					const auto loadedValue = genMoveLoad(function, methodOwner, targetType);
					genMoveStore(function, loadedValue, destPtr, targetType);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_ADDRESS:
				case METHOD_DISSOLVE: {
					return args[0].resolve(function);
				}
				case METHOD_SETVALUE: {
					const auto operand = args[1].resolve(function);
					const auto methodOwner = args[0].resolve(function);
					
					// Assign new value.
					genMoveStore(function, operand, methodOwner, targetType);
					
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_EXTRACTVALUE: {
					const auto methodOwner = args[0].resolve(function);
					return genMoveLoad(function, methodOwner, targetType);
				}
				case METHOD_DESTROYVALUE: {
					const auto methodOwner = args[0].resolve(function);
					// Destroy existing value.
					irEmitter.emitDestructorCall(methodOwner, targetType);
					return ConstantGenerator(module).getVoidUndef();
				}
				default:
					llvm_unreachable("Unknown final_lval primitive method.");
			}
		}
		
	}
	
}

