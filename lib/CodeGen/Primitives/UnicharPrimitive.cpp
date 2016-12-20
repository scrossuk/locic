#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/AST/Type.hpp>

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
#include <locic/CodeGen/Primitives/UnicharPrimitive.hpp>
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
		
		llvm::Value* callCastMethod(Function& function, llvm::Value* const castFromValue, const AST::Type* const castFromType,
				MethodID methodID, const AST::Type* const rawCastToType, llvm::Value* const hintResultValue);
		
		UnicharPrimitive::UnicharPrimitive(const AST::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) { }
		
		bool UnicharPrimitive::isSizeAlwaysKnown(const TypeInfo& /*typeInfo*/,
		                                         llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool UnicharPrimitive::isSizeKnownInThisModule(const TypeInfo& /*typeInfo*/,
		                                               llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool UnicharPrimitive::hasCustomDestructor(const TypeInfo& /*typeInfo*/,
		                                           llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		bool UnicharPrimitive::hasCustomMove(const TypeInfo& /*typeInfo*/,
		                                     llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		llvm_abi::Type
		UnicharPrimitive::getABIType(Module& /*module*/,
		                             const llvm_abi::TypeBuilder& /*abiTypeBuilder*/,
		                             llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return llvm_abi::UInt32Ty;
		}
		
		llvm::Type*
		UnicharPrimitive::getIRType(Module& module,
		                            const TypeGenerator& typeGenerator,
		                            llvm::ArrayRef<AST::Value> templateArguments) const {
			const auto abiType = this->getABIType(module,
			                                      module.abiTypeBuilder(),
			                                      templateArguments);
			return typeGenerator.getIntType(module.abi().typeInfo().getTypeAllocSize(abiType).asBytes() * 8);
		}
		
		llvm::Value*
		UnicharPrimitive::emitMethod(IREmitter& irEmitter, const MethodID methodID,
		                             llvm::ArrayRef<AST::Value> typeTemplateArguments,
		                             llvm::ArrayRef<AST::Value> functionTemplateArguments,
		                             PendingResultArray args,
		                             llvm::Value* const hintResultValue) const {
			auto& builder = irEmitter.builder();
			auto& function = irEmitter.function();
			auto& module = irEmitter.module();
			
			const auto& constantGenerator = irEmitter.constantGenerator();
			const auto& typeGenerator = irEmitter.typeGenerator();
			
			const auto methodOwner = methodID.isConstructor() ? nullptr : args[0].resolveWithoutBind(function);
			
			const auto abiType = this->getABIType(module,
			                                      module.abiTypeBuilder(),
			                                      typeTemplateArguments);
			const size_t selfWidth = module.abi().typeInfo().getTypeAllocSize(abiType).asBytes() * 8;
			const auto selfType = typeGenerator.getIntType(selfWidth);
			
			switch (methodID) {
				case METHOD_IMPLICITCASTFROM:
				case METHOD_CASTFROM: {
					const auto operand = args[0].resolve(function);
					return builder.CreateZExtOrTrunc(operand, selfType);
				}
				case METHOD_ALIGNMASK: {
					return constantGenerator.getSizeTValue(module.abi().typeInfo().getTypeRequiredAlign(abiType).asBytes() - 1);
				}
				case METHOD_SIZEOF: {
					return constantGenerator.getSizeTValue(module.abi().typeInfo().getTypeAllocSize(abiType).asBytes());
				}
				case METHOD_IMPLICITCAST:
				case METHOD_CAST: {
					AST::ValueArray valueArray;
					for (const auto& value: typeTemplateArguments) {
						valueArray.push_back(value.copy());
					}
					const auto type = AST::Type::Object(&typeInstance_,
					                                    std::move(valueArray));
					return callCastMethod(function,
					                      methodOwner,
					                      type,
					                      methodID,
					                      functionTemplateArguments.front().typeRefType(),
					                      hintResultValue);
				}
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY:
					return methodOwner;
				case METHOD_ASCIIVALUE: {
					return builder.CreateTrunc(methodOwner, typeGenerator.getI8Type());
				}
				case METHOD_SETDEAD: {
					// Do nothing.
					return constantGenerator.getVoidUndef();
				}
				case METHOD_ISLIVE: {
					return constantGenerator.getBool(true);
				}
				case METHOD_COMPARE: {
					const auto operand = args[1].resolveWithoutBind(function);
					const auto isLessThan = builder.CreateICmpULT(methodOwner, operand);
					const auto isGreaterThan = builder.CreateICmpUGT(methodOwner, operand);
					const auto minusOneResult = constantGenerator.getI8(-1);
					const auto zeroResult = constantGenerator.getI8(0);
					const auto plusOneResult = constantGenerator.getI8(1);
					return builder.CreateSelect(isLessThan, minusOneResult,
							builder.CreateSelect(isGreaterThan, plusOneResult, zeroResult));
				}
				case METHOD_EQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return irEmitter.emitI1ToBool(builder.CreateICmpEQ(methodOwner, operand));
				}
				case METHOD_NOTEQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return irEmitter.emitI1ToBool(builder.CreateICmpNE(methodOwner, operand));
				}
				case METHOD_LESSTHAN: {
					const auto operand = args[1].resolveWithoutBind(function);
					return irEmitter.emitI1ToBool(builder.CreateICmpULT(methodOwner, operand));
				}
				case METHOD_LESSTHANOREQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return irEmitter.emitI1ToBool(builder.CreateICmpULE(methodOwner, operand));
				}
				case METHOD_GREATERTHAN: {
					const auto operand = args[1].resolveWithoutBind(function);
					return irEmitter.emitI1ToBool(builder.CreateICmpUGT(methodOwner, operand));
				}
				case METHOD_GREATERTHANOREQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					return irEmitter.emitI1ToBool(builder.CreateICmpUGE(methodOwner, operand));
				}
				
				case METHOD_MOVETO: {
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					
					const auto destPtr = irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
					                                               moveToPtr,
					                                               moveToPosition);
					irEmitter.emitRawStore(methodOwner, destPtr);
					return constantGenerator.getVoidUndef();
				}
				case METHOD_INRANGE: {
					const auto leftOperand = args[1].resolve(function);
					const auto rightOperand = args[2].resolve(function);
					
					return irEmitter.emitI1ToBool(builder.CreateAnd(
						builder.CreateICmpULE(leftOperand, methodOwner),
						builder.CreateICmpULE(methodOwner, rightOperand)
					));
				}
				default:
					llvm_unreachable("Unknown unichar primitive method.");
			}
		}
		
	}
	
}

