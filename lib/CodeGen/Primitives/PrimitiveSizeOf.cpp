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
			
			if (name == "member_lval" || name == "final_lval") {
				// member_lval is struct { T value; }.
				// Hence:
				//     alignof(member_lval) = alignof(T).
				function.getBuilder().CreateRet(genAlignMask(function, getFirstTemplateVarRef(typeInstance)));
			} else if (name == "value_lval") {
				// value_lval is struct { bool isLive; T value; }.
				// Hence:
				//     alignof(value_lval) = max(alignof(bool), alignof(T))
				// 
				// and given alignof(bool) = 1:
				//     alignof(value_lval) = max(1, alignof(T))
				// 
				// and given alignof(T) >= 1:
				//     alignof(value_lval) = alignof(T).
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
				const auto virtualAlignValue = ConstantGenerator(module).getSizeTValue(virtualAlign);
				
				// If the argument is statically known, then the
				// type information is provided via template generators.
				const auto nonVirtualAlign = module.abi().typeAlign(llvm_abi::Type::Pointer(module.abiContext()));
				const auto nonVirtualAlignValue = ConstantGenerator(module).getSizeTValue(nonVirtualAlign);
				
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
			
			if (name == "member_lval" || name == "final_lval") {
				// member_lval is struct { T value; }.
				// Hence:
				//     sizeof(member_lval) = sizeof(T).
				function.getBuilder().CreateRet(genSizeOf(function, getFirstTemplateVarRef(typeInstance)));
			} else if (name == "value_lval") {
				// value_lval is struct { T value; bool isLive; }.
				// Hence:
				//     sizeof(value_lval) = makealigned(makealigned(sizeof(T), alignof(bool)) + sizeof(bool), alignof(T))
				// 
				// and given sizeof(bool) = 1:
				//     sizeof(value_lval) = makealigned(makealigned(sizeof(T), 1) + sizeof(bool), alignof(T))
				// 
				// so:
				//     sizeof(value_lval) = makealigned(sizeof(T) + 1, alignof(T))
				// 
				// and given that:
				//     makealigned(position, align) = align * ((position + (align - 1)) div align)
				// 
				// so that:
				//     makealigned(sizeof(T) + 1, alignof(T)) = alignof(T) * ((sizeof(T) + 1 + (alignof(T) - 1)) div alignof(T))
				// 
				// and:
				//     makealigned(sizeof(T) + 1, alignof(T)) = alignof(T) * ((sizeof(T) + alignof(T)) div alignof(T))
				// 
				// let sizeof(T) = M * alignof(T) where M is integer >= 1:
				//     makealigned(sizeof(T) + 1, alignof(T)) = alignof(T) * ((M * alignof(T) + alignof(T)) div alignof(T))
				// 
				// so:
				//     makealigned(sizeof(T) + 1, alignof(T)) = alignof(T) * (((M + 1) * alignof(T)) div alignof(T))
				// 
				// hence:
				//     makealigned(sizeof(T) + 1, alignof(T)) = alignof(T) * (M + 1)
				// 
				// so...:
				//     makealigned(sizeof(T) + 1, alignof(T)) = alignof(T) + M * alignof(T)
				// 
				// so this can be reduced to:
				//     sizeof(value_lval) = alignof(T) + sizeof(T).
				const auto templateVarRef = getFirstTemplateVarRef(typeInstance);
				const auto templateVarAlign = genAlignOf(function, templateVarRef);
				const auto templateVarSize = genSizeOf(function, templateVarRef);
				function.getBuilder().CreateRet(function.getBuilder().CreateAdd(templateVarAlign, templateVarSize));
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
			if (typeName == "value_lval") {
				const auto targetType = type->templateArguments().front();
				return genAlignMask(function, targetType);
			} else if (typeName == "member_lval" || typeName == "final_lval") {
				const auto targetType = type->templateArguments().front();
				return genAlignMask(function, targetType);
			} else {
				return ConstantGenerator(module).getSizeTValue(abi.typeAlign(genABIType(module, type)) - 1);
			}
		}
		
		llvm::Value* genPrimitiveSizeOf(Function& function, const SEM::Type* const type) {
			auto& module = function.module();
			auto& abi = module.abi();
			
			const auto& typeName = type->getObjectType()->name().last();
			if (typeName == "value_lval") {
				const auto targetType = type->templateArguments().front();
				return function.getBuilder().CreateAdd(genAlignOf(function, targetType), genSizeOf(function, targetType));
			} else if (typeName == "member_lval" || typeName == "final_lval") {
				const auto targetType = type->templateArguments().front();
				return genSizeOf(function, targetType);
			} else {
				return ConstantGenerator(module).getSizeTValue(abi.typeSize(genABIType(module, type)));
			}
		}
		
	}
	
}

