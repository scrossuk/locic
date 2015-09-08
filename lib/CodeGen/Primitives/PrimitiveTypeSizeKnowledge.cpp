#include <assert.h>

#include <stdexcept>
#include <string>

#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {

	namespace CodeGen {
		
		bool isPrimitiveTypeSizeAlwaysKnown(Module& module, const SEM::Type* const type) {
			switch (type->primitiveID()) {
				case PrimitiveFinalLval:
				case PrimitiveValueLval:
					return isTypeSizeAlwaysKnown(module, type->templateArguments().front().typeRefType());
				case PrimitiveRef: {
					const auto refTargetType = type->templateArguments().front().typeRefType();
					return !refTargetType->isTemplateVar()
						|| !refTargetType->getTemplateVar()->isVirtual();
				}
				case PrimitiveStaticArray: {
					assert(type->templateArguments().size() == 2);
					const auto targetType = type->templateArguments().front().typeRefType();
					const auto& elementCountValue = type->templateArguments().back();
					return isTypeSizeAlwaysKnown(module, targetType) &&
					       elementCountValue.isConstant();
				}
				default:
					return true;
			}
		}
		
		bool isPrimitiveTypeSizeKnownInThisModule(Module& module, const SEM::Type* const type) {
			switch (type->primitiveID()) {
				case PrimitiveFinalLval:
				case PrimitiveValueLval:
					return isTypeSizeKnownInThisModule(module, type->templateArguments().front().typeRefType());
				case PrimitiveRef: {
					const auto refTargetType = type->templateArguments().front().typeRefType();
					return !refTargetType->isTemplateVar()
						|| !refTargetType->getTemplateVar()->isVirtual();
				}
				case PrimitiveStaticArray: {
					assert(type->templateArguments().size() == 2);
					const auto targetType = type->templateArguments().front().typeRefType();
					const auto& elementCountValue = type->templateArguments().back();
					return isTypeSizeKnownInThisModule(module, targetType) &&
					       elementCountValue.isConstant();
				}
				default:
					return true;
			}
		}
		
	}
	
}

