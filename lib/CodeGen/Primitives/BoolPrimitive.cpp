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
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
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
		
		llvm::Value* callCastMethod(Function& function, llvm::Value* const castFromValue, const AST::Type* const castFromType,
				MethodID methodID, const AST::Type* const rawCastToType, llvm::Value* const resultPtr);
		
		BoolPrimitive::BoolPrimitive(const AST::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) { }
		
		bool BoolPrimitive::isSizeAlwaysKnown(const TypeInfo& /*typeInfo*/,
		                                      llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool BoolPrimitive::isSizeKnownInThisModule(const TypeInfo& /*typeInfo*/,
		                                            llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool BoolPrimitive::hasCustomDestructor(const TypeInfo& /*typeInfo*/,
		                                        llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		bool BoolPrimitive::hasCustomMove(const TypeInfo& /*typeInfo*/,
		                                  llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		llvm_abi::Type BoolPrimitive::getABIType(Module& /*module*/,
		                                         const llvm_abi::TypeBuilder& /*abiTypeBuilder*/,
		                                         llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return llvm_abi::BoolTy;
		}
		
		llvm::Type* BoolPrimitive::getIRType(Module& /*module*/,
		                                     const TypeGenerator& typeGenerator,
		                                     llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return typeGenerator.getI8Type();
		}
		
		llvm::Value* BoolPrimitive::emitMethod(IREmitter& irEmitter,
		                                       const MethodID methodID,
		                                       llvm::ArrayRef<AST::Value> typeTemplateArguments,
		                                       llvm::ArrayRef<AST::Value> functionTemplateArguments,
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
			
			const auto& constantGenerator = irEmitter.constantGenerator();
			
			switch (methodID) {
				case METHOD_CREATE: {
					assert(args.empty());
					return constantGenerator.getBool(false);
				}
				case METHOD_ALIGNMASK: {
					const auto abiType = this->getABIType(module,
					                                      module.abiTypeBuilder(),
					                                      typeTemplateArguments);
					return constantGenerator.getSizeTValue(module.abi().typeInfo().getTypeRequiredAlign(abiType).asBytes() - 1);
				}
				case METHOD_SIZEOF: {
					const auto abiType = this->getABIType(module,
					                                      module.abiTypeBuilder(),
					                                      typeTemplateArguments);
					return constantGenerator.getSizeTValue(module.abi().typeInfo().getTypeAllocSize(abiType).asBytes());
				}
				case METHOD_DESTROY:
				case METHOD_SETDEAD: {
					// Do nothing.
					(void) args[0].resolveWithoutBind(function);
					return constantGenerator.getVoidUndef();
				}
				case METHOD_MOVE:
					return args[0].resolveWithoutBind(function);
				case METHOD_IMPLICITCAST:
				case METHOD_CAST: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return callCastMethod(function, methodOwner, type, methodID,
					                      functionTemplateArguments.front().typeRefType(),
					                      resultPtr);
				}
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY:
					return args[0].resolveWithoutBind(function);
				case METHOD_ISLIVE: {
					(void) args[0].resolveWithoutBind(function);
					return constantGenerator.getBool(false);
				}
				case METHOD_NOT: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					const auto i1Value = irEmitter.emitBoolToI1(methodOwner);
					const auto i1NotValue = irEmitter.builder().CreateNot(i1Value);
					return irEmitter.emitI1ToBool(i1NotValue);
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
					return irEmitter.emitI1ToBool(irEmitter.builder().CreateICmpEQ(methodOwner, operand));
				}
				case METHOD_NOTEQUAL: {
					const auto operand = args[1].resolveWithoutBind(function);
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return irEmitter.emitI1ToBool(irEmitter.builder().CreateICmpNE(methodOwner, operand));
				}
				default:
					llvm_unreachable("Unknown bool primitive method.");
			}
		}
		
	}
	
}

