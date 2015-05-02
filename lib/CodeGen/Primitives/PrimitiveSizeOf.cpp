#include <assert.h>

#include <stdexcept>
#include <string>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		namespace {
			
			const SEM::Type* getFirstTemplateVarRef(const SEM::TypeInstance* const typeInstance) {
				assert(!typeInstance->templateVariables().empty());
				
				const auto templateVar = typeInstance->templateVariables().front();
				return SEM::Type::TemplateVarRef(templateVar);
			}
			
		}
		
		void createPrimitiveAlignOf(Module& module, const SEM::TypeInstance* const typeInstance, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			Function function(module, llvmFunction, alignMaskArgInfo(module, typeInstance), &(module.templateBuilder(TemplatedObject::TypeInstance(typeInstance))));
			
			const auto& name = typeInstance->name().first();
			
			if (name == "final_lval" || name == "value_lval") {
				// lval is struct { T value; }.
				function.getBuilder().CreateRet(genAlignMask(function, getFirstTemplateVarRef(typeInstance)));
			} else if (name == "__ref") {
				// Look at our template argument to see if it's virtual.
				const auto argTypeInfo = function.getBuilder().CreateExtractValue(function.getTemplateArgs(), { 0 });
				const auto argVTablePointer = function.getBuilder().CreateExtractValue(argTypeInfo, { 0 });
				
				// If the VTable pointer is NULL, it's a virtual type, which
				// means we are larger (to store the type information etc.).
				const auto nullVtablePtr = ConstantGenerator(module).getNullPointer(vtableType(module)->getPointerTo());
				const auto isVirtualCondition = function.getBuilder().CreateICmpEQ(argVTablePointer, nullVtablePtr, "isVirtual");
				
				// If it has a virtual argument, then it will store
				// the type information internally.
				const auto virtualAlign = module.abi().typeAlign(interfaceStructType(module).first);
				const auto virtualAlignValue = ConstantGenerator(module).getSizeTValue(virtualAlign - 1);
				
				// If the argument is statically known, then the
				// type information is provided via template generators.
				const auto nonVirtualAlign = module.abi().typeAlign(llvm_abi::Type::Pointer(module.abiContext()));
				const auto nonVirtualAlignValue = ConstantGenerator(module).getSizeTValue(nonVirtualAlign - 1);
				
				function.getBuilder().CreateRet(function.getBuilder().CreateSelect(isVirtualCondition, virtualAlignValue, nonVirtualAlignValue));
			} else {
				// Everything else already has a known alignment.
				const auto abiType = genABIType(module, typeInstance->selfType());
				const auto align = module.abi().typeAlign(abiType);
				function.getBuilder().CreateRet(ConstantGenerator(module).getSizeTValue(align - 1));
			}
		}
		
		void createPrimitiveSizeOf(Module& module, const SEM::TypeInstance* const typeInstance, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			Function function(module, llvmFunction, sizeOfArgInfo(module, typeInstance), &(module.templateBuilder(TemplatedObject::TypeInstance(typeInstance))));
			
			const auto& name = typeInstance->name().first();
			
			if (name == "final_lval" || name == "value_lval") {
				// lval is struct { T value; }.
				function.getBuilder().CreateRet(genSizeOf(function, getFirstTemplateVarRef(typeInstance)));
			} else if (name == "__ref") {
				// Look at our template argument to see if it's virtual.
				const auto argTypeInfo = function.getBuilder().CreateExtractValue(function.getTemplateArgs(), { 0 });
				const auto argVTablePointer = function.getBuilder().CreateExtractValue(argTypeInfo, { 0 });
				
				// If the VTable pointer is NULL, it's a virtual type, which
				// means we are larger (to store the type information etc.).
				const auto nullVtablePtr = ConstantGenerator(module).getNullPointer(vtableType(module)->getPointerTo());
				const auto isVirtualCondition = function.getBuilder().CreateICmpEQ(argVTablePointer, nullVtablePtr, "isVirtual");
				
				// If it has a virtual argument, then it will store
				// the type information internally.
				const auto virtualSize = module.abi().typeSize(interfaceStructType(module).first);
				const auto virtualSizeValue = ConstantGenerator(module).getSizeTValue(virtualSize);
				
				// If the argument is statically known, then the
				// type information is provided via template generators.
				const auto nonVirtualSize = module.abi().typeSize(llvm_abi::Type::Pointer(module.abiContext()));
				const auto nonVirtualSizeValue = ConstantGenerator(module).getSizeTValue(nonVirtualSize);
				
				function.getBuilder().CreateRet(function.getBuilder().CreateSelect(isVirtualCondition, virtualSizeValue, nonVirtualSizeValue));
			} else {
				// Everything else already has a known size.
				const auto abiType = genABIType(module, typeInstance->selfType());
				const auto size = module.abi().typeSize(abiType);
				function.getBuilder().CreateRet(ConstantGenerator(module).getSizeTValue(size));
			}
		}
		
		llvm::Value* genPrimitiveAlignMask(Function& function, const SEM::Type* const type) {
			auto& module = function.module();
			auto& abi = module.abi();
			
			const auto& typeName = type->getObjectType()->name().last();
			if (typeName == "final_lval" || typeName == "value_lval") {
				const auto targetType = type->templateArguments().front().typeRefType();
				return genAlignMask(function, targetType);
			}
			
			if (typeName == "__ref") {
				const auto targetType = type->templateArguments().front().typeRefType();
				if (targetType->isTemplateVar() && targetType->getTemplateVar()->isVirtual()) {
					// Unknown alignment since don't know whether
					// target type is virtual or not.
					return nullptr;
				}
			}
			
			return ConstantGenerator(module).getSizeTValue(abi.typeAlign(genABIType(module, type)) - 1);
		}
		
		llvm::Value* genPrimitiveSizeOf(Function& function, const SEM::Type* const type) {
			auto& module = function.module();
			auto& abi = module.abi();
			
			const auto& typeName = type->getObjectType()->name().last();
			if (typeName == "final_lval" || typeName == "value_lval") {
				const auto targetType = type->templateArguments().front().typeRefType();
				return genSizeOf(function, targetType);
			}
			
			if (typeName == "__ref") {
				const auto targetType = type->templateArguments().front().typeRefType();
				if (targetType->isTemplateVar() && targetType->getTemplateVar()->isVirtual()) {
					// Unknown size since don't know whether
					// target type is virtual or not.
					return nullptr;
				}
			}
			
			return ConstantGenerator(module).getSizeTValue(abi.typeSize(genABIType(module, type)));
		}
		
	}
	
}

