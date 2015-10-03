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
#include <locic/CodeGen/Primitives/CompareResultPrimitive.hpp>
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
		
		CompareResultPrimitive::CompareResultPrimitive(const SEM::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) {
			(void) typeInstance_;
		}
		
		bool CompareResultPrimitive::isSizeAlwaysKnown(const TypeInfo& /*typeInfo*/,
		                                               llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool CompareResultPrimitive::isSizeKnownInThisModule(const TypeInfo& /*typeInfo*/,
		                                                     llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool CompareResultPrimitive::hasCustomDestructor(const TypeInfo& /*typeInfo*/,
		                                                 llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return false;
		}
		
		bool CompareResultPrimitive::hasCustomMove(const TypeInfo& /*typeInfo*/,
		                                           llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return false;
		}
		
		llvm_abi::Type* CompareResultPrimitive::getABIType(Module& /*module*/,
		                                                   llvm_abi::Context& abiContext,
		                                                   llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			 // Compare results represented with 8 bits.
			return llvm_abi::Type::Integer(abiContext,  llvm_abi::Int8);
		}
		
		llvm::Type* CompareResultPrimitive::getIRType(Module& /*module*/,
		                                              const TypeGenerator& typeGenerator,
		                                              llvm::ArrayRef<SEM::Value> /*templateArguments*/) const {
			return typeGenerator.getI8Type();
		}
		
		llvm::Value* CompareResultPrimitive::emitMethod(IREmitter& irEmitter,
		                                                const MethodID methodID,
		                                                llvm::ArrayRef<SEM::Value> typeTemplateArguments,
		                                                llvm::ArrayRef<SEM::Value> /*functionTemplateArguments*/,
		                                                PendingResultArray args) const {
			auto& function = irEmitter.function();
			auto& module = irEmitter.module();
			
			const auto& constantGenerator = irEmitter.constantGenerator();
			
			const auto lessThanValue = constantGenerator.getI8(-1);
			const auto equalValue = constantGenerator.getI8(0);
			const auto greaterThanValue = constantGenerator.getI8(1);
			
			switch (methodID) {
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
				case METHOD_LESSTHAN:
					assert(args.empty());
					return lessThanValue;
				case METHOD_EQUAL:
					assert(args.empty());
					return equalValue;
				case METHOD_GREATERTHAN:
					assert(args.empty());
					return greaterThanValue;
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY:
					return args[0].resolveWithoutBind(function);
				case METHOD_ISEQUAL: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return irEmitter.builder().CreateICmpEQ(methodOwner, equalValue);
				}
				case METHOD_ISNOTEQUAL: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return irEmitter.builder().CreateICmpNE(methodOwner, equalValue);
				}
				case METHOD_ISLESSTHAN: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return irEmitter.builder().CreateICmpEQ(methodOwner, lessThanValue);
				}
				case METHOD_ISLESSTHANOREQUAL: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return irEmitter.builder().CreateICmpNE(methodOwner, greaterThanValue);
				}
				case METHOD_ISGREATERTHAN: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return irEmitter.builder().CreateICmpEQ(methodOwner, greaterThanValue);
				}
				case METHOD_ISGREATERTHANOREQUAL: {
					const auto methodOwner = args[0].resolveWithoutBind(function);
					return irEmitter.builder().CreateICmpNE(methodOwner, lessThanValue);
				}
				default:
					llvm_unreachable("Unknown compare_result_t method.");
			}
		}
		
	}
	
}

