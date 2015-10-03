#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

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
#include <locic/CodeGen/Primitives/BoolPrimitive.hpp>
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
		
		llvm::Value* callCastMethod(Function& function, llvm::Value* const castFromValue, const SEM::Type* const castFromType,
				MethodID methodID, const SEM::Type* const rawCastToType, llvm::Value* const hintResultValue);
		
		BoolPrimitive::BoolPrimitive(const SEM::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) { }
		
		bool BoolPrimitive::isSizeAlwaysKnown(const TypeInfo& /*typeInfo*/,
		                                      llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool BoolPrimitive::isSizeKnownInThisModule(const TypeInfo& /*typeInfo*/,
		                                            llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool BoolPrimitive::hasCustomDestructor(const TypeInfo& /*typeInfo*/,
		                                        llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return false;
		}
		
		bool BoolPrimitive::hasCustomMove(const TypeInfo& /*typeInfo*/,
		                                  llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return false;
		}
		
		llvm_abi::Type* BoolPrimitive::getABIType(Module& /*module*/,
		                                          llvm_abi::Context& abiContext,
		                                          llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return llvm_abi::Type::Integer(abiContext, llvm_abi::Bool);
		}
		
		llvm::Type* BoolPrimitive::getIRType(Module& /*module*/,
		                                     const TypeGenerator& typeGenerator,
		                                     llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return typeGenerator.getI1Type();
		}
		
		llvm::Value* BoolPrimitive::emitMethod(IREmitter& irEmitter,
		                                       const MethodID methodID,
		                                       llvm::ArrayRef<SEM::Value> typeTemplateArguments,
		                                       llvm::ArrayRef<SEM::Value> functionTemplateArguments,
		                                       PendingResultArray args) const {
			auto& function = irEmitter.function();
			auto& module = irEmitter.module();
			
			SEM::ValueArray valueArray;
			for (const auto& value: typeTemplateArguments) {
				valueArray.push_back(value.copy());
			}
			const auto type = SEM::Type::Object(&typeInstance_,
			                                    std::move(valueArray));
			
			const auto& constantGenerator = irEmitter.constantGenerator();
			const auto& typeGenerator = irEmitter.typeGenerator();
			
			switch (methodID) {
				case METHOD_CREATE: {
					assert(args.empty());
					return constantGenerator.getI1(false);
				}
				case METHOD_ALIGNMASK: {
					const auto abiType = this->getABIType(module,
					                                      module.abiContext(),
					                                      typeTemplateArguments);
					return constantGenerator.getSizeTValue(module.abi().typeAlign(abiType) - 1);
				}
				case METHOD_SIZEOF: {
					const auto abiType = this->getABIType(module,
					                                      module.abiContext(),
					                                      typeTemplateArguments);
					return constantGenerator.getSizeTValue(module.abi().typeSize(abiType));
				}
				case METHOD_SETDEAD: {
					// Do nothing.
					(void) args[0].resolveWithoutBind(function);
					return constantGenerator.getVoidUndef();
				}
				case METHOD_MOVETO: {
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					const auto methodOwner = args[0].resolveWithoutBind(function);
					
					const auto irType = this->getIRType(module,
					                                    typeGenerator,
					                                    typeTemplateArguments);
					const auto destPtr = irEmitter.builder().CreateInBoundsGEP(moveToPtr,
					                                                           moveToPosition);
					const auto castedDestPtr = irEmitter.builder().CreatePointerCast(destPtr,
					                                                                 irType->getPointerTo());
					irEmitter.emitMoveStore(methodOwner,
					                        castedDestPtr,
					                        type);
					return constantGenerator.getVoidUndef();
				}
				case METHOD_IMPLICITCAST:
				case METHOD_CAST: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return callCastMethod(function, methodOwner, type, methodID,
					                      functionTemplateArguments.front().typeRefType(),
					                      irEmitter.hintResultValue());
				}
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY:
					return args[0].resolveWithoutBind(function);
				case METHOD_ISLIVE: {
					(void) args[0].resolveWithoutBind(function);
					return constantGenerator.getI1(false);
				}
				case METHOD_NOT: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return irEmitter.builder().CreateNot(methodOwner);
				}
				case METHOD_COMPARE: {
					const auto operand = args[1].resolveWithoutBind(function);
					const auto methodOwner = args[0].resolveWithoutBind(function);
					const auto isLessThan = irEmitter.builder().CreateICmpULT(methodOwner, operand);
					const auto isGreaterThan = irEmitter.builder().CreateICmpUGT(methodOwner, operand);
					const auto minusOneResult = constantGenerator.getI8(-1);
					const auto zeroResult = constantGenerator.getI8(0);
					const auto plusOneResult = constantGenerator.getI8(1);
					return irEmitter.builder().CreateSelect(isLessThan, minusOneResult,
						irEmitter.builder().CreateSelect(isGreaterThan, plusOneResult, zeroResult));
				}
				case METHOD_EQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return irEmitter.builder().CreateICmpEQ(methodOwner, operand);
				}
				case METHOD_NOTEQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return irEmitter.builder().CreateICmpNE(methodOwner, operand);
				}
				default:
					llvm_unreachable("Unknown bool primitive method.");
			}
		}
		
	}
	
}

