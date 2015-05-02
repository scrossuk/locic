#include <assert.h>

#include <stdexcept>
#include <string>

#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
		
		bool isPrimitiveTypeSizeAlwaysKnown(Module& module, const SEM::Type* type) {
			const auto typeInstance = type->getObjectType();
			assert(typeInstance->isPrimitive());
			const auto& name = typeInstance->name().first();
			const auto kind = module.primitiveKind(name);
			
			switch (kind) {
				case PrimitiveFinalLval:
				case PrimitiveValueLval:
					return isTypeSizeAlwaysKnown(module, type->templateArguments().front().typeRefType());
				case PrimitiveRef: {
					const auto refTargetType = type->templateArguments().front().typeRefType();
					return !refTargetType->isTemplateVar()
						|| !refTargetType->getTemplateVar()->isVirtual();
				}
				default:
					return true;
			}
		}
		
		bool isPrimitiveTypeSizeKnownInThisModule(Module& module, const SEM::Type* type) {
			const auto typeInstance = type->getObjectType();
			assert(typeInstance->isPrimitive());
			const auto& name = typeInstance->name().first();
			const auto kind = module.primitiveKind(name);
			
			switch (kind) {
				case PrimitiveFinalLval:
				case PrimitiveValueLval:
					return isTypeSizeKnownInThisModule(module, type->templateArguments().front().typeRefType());
				case PrimitiveRef: {
					const auto refTargetType = type->templateArguments().front().typeRefType();
					return !refTargetType->isTemplateVar()
						|| !refTargetType->getTemplateVar()->isVirtual();
				}
				default:
					return true;
			}
		}
		
	}
	
}

