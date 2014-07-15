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
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
	
		void createPrimitiveAlignOf(Module& module, SEM::Type* type, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto typeInstance = type->getObjectType();
			
			Function function(module, llvmFunction, alignMaskArgInfo(module, typeInstance), &(module.templateBuilder(TemplatedObject::TypeInstance(typeInstance))));
			
			const auto& name = typeInstance->name().first();
			
			if (name == "member_lval") {
				// member_lval is struct { T value; }.
				// Hence:
				//     alignof(member_lval) = alignof(T).
				function.getBuilder().CreateRet(genAlignMask(function, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0))));
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
				function.getBuilder().CreateRet(genAlignMask(function, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0))));
			} else if (name == "__ref") {
				size_t align = 0;
				if (hasVirtualTypeArgument(type)) {
					// If it has a virtual argument, then it will store
					// the type information internally.
					align = module.abi().typeAlign(interfaceStructType(module).first);
				} else {
					// If the argument is statically known, then the
					// type information is provided via template generators.
					align = module.abi().typeAlign(llvm_abi::Type::Pointer(module.abiContext()));
				}
				function.getBuilder().CreateRet(ConstantGenerator(module).getSizeTValue(align - 1));
			} else {
				// Everything else already has a known alignment.
				const auto abiType = genABIType(module, type);
				const auto align = module.abi().typeAlign(abiType);
				function.getBuilder().CreateRet(ConstantGenerator(module).getSizeTValue(align - 1));
			}
		}
		
		void createPrimitiveSizeOf(Module& module, SEM::Type* type, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			const auto typeInstance = type->getObjectType();
			
			Function function(module, llvmFunction, sizeOfArgInfo(module, typeInstance), &(module.templateBuilder(TemplatedObject::TypeInstance(typeInstance))));
			
			const auto& name = typeInstance->name().first();
			
			if (name == "member_lval") {
				// member_lval is struct { T value; }.
				// Hence:
				//     sizeof(member_lval) = sizeof(T).
				function.getBuilder().CreateRet(genSizeOf(function, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0))));
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
				const auto templateVarAlign = genAlignOf(function, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0)));
				const auto templateVarSize = genSizeOf(function, SEM::Type::TemplateVarRef(typeInstance->templateVariables().at(0)));
				function.getBuilder().CreateRet(function.getBuilder().CreateAdd(templateVarAlign, templateVarSize));
			} else if (name == "__ref") {
				size_t size = 0;
				if (hasVirtualTypeArgument(type)) {
					// If it has a virtual argument, then it will store
					// the type information internally.
					size = module.abi().typeSize(interfaceStructType(module).first);
				} else {
					// If the argument is statically known, then the
					// type information is provided via template generators.
					size = module.abi().typeSize(llvm_abi::Type::Pointer(module.abiContext()));
				}
				function.getBuilder().CreateRet(ConstantGenerator(module).getSizeTValue(size));
			} else {
				// Everything else already has a known size.
				const auto abiType = genABIType(module, type);
				const auto size = module.abi().typeSize(abiType);
				function.getBuilder().CreateRet(ConstantGenerator(module).getSizeTValue(size));
			}
		}
		
		llvm::Value* genPrimitiveAlignMask(Function& function, SEM::Type* type) {
			auto& module = function.module();
			auto& abi = module.abi();
			
			const auto typeName = type->getObjectType()->name().last();
			if (typeName == "value_lval") {
				const auto targetType = type->templateArguments().front();
				return genAlignMask(function, targetType);
			} else if (typeName == "member_lval") {
				const auto targetType = type->templateArguments().front();
				return genAlignMask(function, targetType);
			} else {
				return ConstantGenerator(module).getSizeTValue(abi.typeAlign(genABIType(module, type)) - 1);
			}
		}
		
		llvm::Value* genPrimitiveSizeOf(Function& function, SEM::Type* type) {
			auto& module = function.module();
			auto& abi = module.abi();
			
			const auto typeName = type->getObjectType()->name().last();
			if (typeName == "value_lval") {
				const auto targetType = type->templateArguments().front();
				return function.getBuilder().CreateAdd(genAlignOf(function, targetType), genSizeOf(function, targetType));
			} else if (typeName == "member_lval") {
				const auto targetType = type->templateArguments().front();
				return genSizeOf(function, targetType);
			} else {
				return ConstantGenerator(module).getSizeTValue(abi.typeSize(genABIType(module, type)));
			}
		}
		
	}
	
}

