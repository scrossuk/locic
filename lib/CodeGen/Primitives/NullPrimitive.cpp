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
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Primitives/NullPrimitive.hpp>
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
		
		llvm::Value* callRawCastMethod(Function& function, llvm::Value* const castFromValue, const AST::Type* const castFromType,
				const String& targetMethodName, const AST::Type* const castToType, llvm::Value* const resultPtr);
		
		NullPrimitive::NullPrimitive(const AST::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) { }
		
		bool NullPrimitive::isSizeAlwaysKnown(const TypeInfo& /*typeInfo*/,
		                                      llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool NullPrimitive::isSizeKnownInThisModule(const TypeInfo& /*typeInfo*/,
		                                            llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return true;
		}
		
		bool NullPrimitive::hasCustomDestructor(const TypeInfo& /*typeInfo*/,
		                                        llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		bool NullPrimitive::hasCustomMove(const TypeInfo& /*typeInfo*/,
		                                  llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		llvm_abi::Type NullPrimitive::getABIType(Module& /*module*/,
		                                          const llvm_abi::TypeBuilder& /*abiTypeBuilder*/,
		                                          llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return llvm_abi::PointerTy;
		}
		
		llvm::Value* NullPrimitive::emitMethod(IREmitter& irEmitter,
		                                       const MethodID methodID,
		                                       llvm::ArrayRef<AST::Value> typeTemplateArguments,
		                                       llvm::ArrayRef<AST::Value> functionTemplateArguments,
		                                       PendingResultArray /*args*/,
		                                       llvm::Value* const resultPtr) const {
			auto& function = irEmitter.function();
			auto& module = irEmitter.module();
			
			switch (methodID) {
				case METHOD_CREATE:
					return ConstantGenerator(module).getNull(TypeGenerator(module).getPtrType());
				case METHOD_ALIGNMASK: {
					const auto abiType = this->getABIType(module,
					                                      module.abiTypeBuilder(),
					                                      typeTemplateArguments);
					return ConstantGenerator(module).getSizeTValue(module.abi().typeInfo().getTypeRequiredAlign(abiType).asBytes() - 1);
				}
				case METHOD_SIZEOF: {
					const auto abiType = this->getABIType(module,
					                                      module.abiTypeBuilder(),
					                                      typeTemplateArguments);
					return ConstantGenerator(module).getSizeTValue(module.abi().typeInfo().getTypeAllocSize(abiType).asBytes());
				}
				case METHOD_IMPLICITCOPY:
				case METHOD_COPY: {
					return ConstantGenerator(module).getNull(TypeGenerator(module).getPtrType());
				}
				case METHOD_IMPLICITCAST:
				case METHOD_CAST: {
					AST::ValueArray valueArray;
					for (const auto& value: typeTemplateArguments) {
						valueArray.push_back(value.copy());
					}
					const auto type = AST::Type::Object(&typeInstance_,
					                                    std::move(valueArray));
					const auto targetType = functionTemplateArguments.front().typeRefType();
					return callRawCastMethod(function, nullptr, type, module.getCString("null"),
					                         targetType, resultPtr);
				}
				default:
					llvm_unreachable("Unknown null_t primitive method.");
			}
		}
		
	}
	
}

