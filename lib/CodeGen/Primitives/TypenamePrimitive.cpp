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
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Primitives/TypenamePrimitive.hpp>
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
		
		TypenamePrimitive::TypenamePrimitive(const AST::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) {
			(void) typeInstance_;
		}
		
		bool TypenamePrimitive::isSizeAlwaysKnown(const TypeInfo& /*typeInfo*/,
		                                          llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool TypenamePrimitive::isSizeKnownInThisModule(const TypeInfo& /*typeInfo*/,
		                                                llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool TypenamePrimitive::hasCustomDestructor(const TypeInfo& /*typeInfo*/,
		                                        llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		bool TypenamePrimitive::hasCustomMove(const TypeInfo& /*typeInfo*/,
		                                  llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		llvm_abi::Type TypenamePrimitive::getABIType(Module& module,
		                                             const llvm_abi::TypeBuilder& /*abiTypeBuilder*/,
		                                             llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return typeInfoType(module).first;
		}
		
		llvm::Type* TypenamePrimitive::getIRType(Module& module,
		                                         const TypeGenerator& /*typeGenerator*/,
		                                         llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return typeInfoType(module).second;
		}
		
		llvm::Value* TypenamePrimitive::emitMethod(IREmitter& irEmitter,
		                                           const MethodID methodID,
		                                           llvm::ArrayRef<AST::Value> typeTemplateArguments,
		                                           llvm::ArrayRef<AST::Value> /*functionTemplateArguments*/,
		                                           PendingResultArray args,
		                                           llvm::Value* /*resultPtr*/) const {
			auto& function = irEmitter.function();
			auto& module = irEmitter.module();
			
			const auto& constantGenerator = irEmitter.constantGenerator();
			
			switch (methodID) {
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
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY:
				case METHOD_MOVE:
					return args[0].resolveWithoutBind(function);
				case METHOD_DESTROY:
					(void) args[0].resolveWithoutBind(function);
					return ConstantGenerator(module).getVoidUndef();
				default:
					llvm_unreachable("Unknown typename primitive method.");
			}
		}
		
	}
	
}

