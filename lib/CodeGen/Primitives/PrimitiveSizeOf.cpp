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

