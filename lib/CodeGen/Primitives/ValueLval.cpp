#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>
#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		llvm::Value* genValueLvalCreateMethod(Function& functionGenerator, const SEM::Type* const targetType, PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& builder = functionGenerator.getBuilder();
			auto& module = functionGenerator.module();
			
			const auto objectVar = genAlloca(functionGenerator, targetType, hintResultValue);
			const auto operand = args[0].resolve(functionGenerator, hintResultValue);
			
			// Store the object.
			const auto targetPtr = builder.CreatePointerCast(objectVar, genPointerType(module, targetType));
			genMoveStore(functionGenerator, operand, targetPtr, targetType);
			return genMoveLoad(functionGenerator, objectVar, targetType);
		}
		
		llvm::Value* genValueLvalSetDeadMethod(Function& functionGenerator, const SEM::Type* const targetType, PendingResultArray args) {
			auto& module = functionGenerator.module();
			const auto methodOwner = args[0].resolve(functionGenerator);
			genSetDeadState(functionGenerator, targetType, methodOwner);
			return ConstantGenerator(module).getVoidUndef();
		}
		
		llvm::Value* genValueLvalMoveToMethod(Function& functionGenerator, const SEM::Type* const targetType, PendingResultArray args) {
			auto& builder = functionGenerator.getBuilder();
			auto& module = functionGenerator.module();
			
			const bool typeSizeIsKnown = isTypeSizeKnownInThisModule(module, targetType);
			
			const auto sourceValue = args[0].resolve(functionGenerator);
			const auto destValue = args[1].resolve(functionGenerator);
			const auto positionValue = args[2].resolve(functionGenerator);
			
			const auto castType = typeSizeIsKnown ? genPointerType(module, targetType) : TypeGenerator(module).getI8PtrType();
			const auto sourceObjectPointer = builder.CreatePointerCast(sourceValue, castType);
			const auto destObjectPointer = builder.CreatePointerCast(destValue, castType);
			
			genMoveCall(functionGenerator, targetType, sourceObjectPointer, destObjectPointer, positionValue);
			return ConstantGenerator(module).getVoidUndef();
		}
		
		llvm::Value* genValueLvalAddressMethod(Function& functionGenerator, const SEM::Type* const targetType, PendingResultArray args) {
			auto& builder = functionGenerator.getBuilder();
			auto& module = functionGenerator.module();
			
			const auto methodOwner = args[0].resolve(functionGenerator);
			return builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
		}
		
		llvm::Value* genValueLvalDissolveMethod(Function& functionGenerator, const SEM::Type* const targetType, PendingResultArray args) {
			auto& builder = functionGenerator.getBuilder();
			auto& module = functionGenerator.module();
			
			const auto methodOwner = args[0].resolve(functionGenerator);
			return builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
		}
		
		llvm::Value* genValueLvalMoveMethod(Function& functionGenerator, const SEM::Type* const targetType, PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& builder = functionGenerator.getBuilder();
			auto& module = functionGenerator.module();
			
			const auto methodOwner = args[0].resolve(functionGenerator);
			
			const auto targetPointer = builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
			
			const auto returnValuePtr = genAlloca(functionGenerator, targetType, hintResultValue);
			const auto loadedValue = genMoveLoad(functionGenerator, targetPointer, targetType);
			genMoveStore(functionGenerator, loadedValue, returnValuePtr, targetType);
			
			return genMoveLoad(functionGenerator, returnValuePtr, targetType);
		}
		
		llvm::Value* genValueLvalAssignMethod(Function& functionGenerator, const SEM::Type* const targetType, PendingResultArray args) {
			auto& builder = functionGenerator.getBuilder();
			auto& module = functionGenerator.module();
			
			const auto methodOwner = args[0].resolve(functionGenerator);
			const auto operand = args[1].resolve(functionGenerator);
			
			const auto targetPointer = builder.CreatePointerCast(methodOwner, genPointerType(module, targetType));
			
			genDestructorCall(functionGenerator, targetType, targetPointer);
			genMoveStore(functionGenerator, operand, targetPointer, targetType);
			return ConstantGenerator(module).getVoidUndef();
		}
		
		llvm::Value* genValueLvalPrimitiveMethodCall(Function& functionGenerator, const SEM::Type* type, const String& methodName, const SEM::Type* const /*functionType*/,
				PendingResultArray args, llvm::Value* const hintResultValue) {
			auto& module = functionGenerator.module();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			const auto targetType = type->templateArguments().front().typeRefType();
			
			switch (methodID) {
				case METHOD_CREATE:
					return genValueLvalCreateMethod(functionGenerator, targetType, std::move(args), hintResultValue);
				case METHOD_SETDEAD:
					return genValueLvalSetDeadMethod(functionGenerator, targetType, std::move(args));
				case METHOD_MOVETO:
					return genValueLvalMoveToMethod(functionGenerator, targetType, std::move(args));
				case METHOD_ADDRESS:
					return genValueLvalAddressMethod(functionGenerator, targetType, std::move(args));
				case METHOD_DISSOLVE:
					return genValueLvalDissolveMethod(functionGenerator, targetType, std::move(args));
				case METHOD_MOVE:
					return genValueLvalMoveMethod(functionGenerator, targetType, std::move(args), hintResultValue);
				case METHOD_ASSIGN:
					return genValueLvalAssignMethod(functionGenerator, targetType, std::move(args));
				default:
					printf("%s\n", methodName.c_str());
					llvm_unreachable("Unknown primitive value_lval method.");
			}
		}
		
	}
	
}

