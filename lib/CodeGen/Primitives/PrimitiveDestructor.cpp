#include <assert.h>

#include <string>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>
#include <locic/CodeGen/UnwindAction.hpp>

namespace locic {

	namespace CodeGen {
	
		void createPrimitiveDestructor(Module& module, const SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			Function functionGenerator(module, llvmFunction, destructorArgInfo(module, *typeInstance), &(module.templateBuilder(TemplatedObject::TypeInstance(typeInstance))));
			
			const auto debugInfo = genDebugDestructorFunction(module, *typeInstance, &llvmFunction);
			functionGenerator.attachDebugInfo(debugInfo);
			functionGenerator.setDebugPosition(getDebugDestructorPosition(module, *typeInstance));
			
			genPrimitiveDestructorCall(functionGenerator, typeInstance->selfType(), functionGenerator.getRawContextValue());
			functionGenerator.getBuilder().CreateRetVoid();
			
			functionGenerator.verify();
		}
		
		void genPrimitiveDestructorCall(Function& function, const SEM::Type* type, llvm::Value* value) {
			assert(value->getType()->isPointerTy());
			
			const auto& typeName = type->getObjectType()->name().last();
			
			if (typeName == "value_lval" || typeName == "final_lval") {
				const auto targetType = type->templateArguments().front().typeRefType();
				genDestructorCall(function, targetType, value);
			}
		}
		
		bool primitiveTypeHasDestructor(Module& module, const SEM::Type* type) {
			assert(type->isPrimitive());
			const auto& name = type->getObjectType()->name().first();
			const auto kind = module.primitiveKind(name);
			return (kind == PrimitiveValueLval || kind == PrimitiveFinalLval) && typeHasDestructor(module, type->templateArguments().front().typeRefType());
		}
		
		bool primitiveTypeInstanceHasDestructor(Module& module, const SEM::TypeInstance* typeInstance) {
			assert(typeInstance->isPrimitive());
			const auto& name = typeInstance->name().first();
			const auto kind = module.primitiveKind(name);
			return (kind == PrimitiveValueLval || kind == PrimitiveFinalLval);
		}
		
	}
	
}

