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

namespace locic {

	namespace CodeGen {
	
		void createPrimitiveDestructor(Module& module, SEM::TypeInstance* typeInstance, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			Function function(module, llvmFunction, destructorArgInfo(module, typeInstance), &(module.templateBuilder(TemplatedObject::TypeInstance(typeInstance))));
			genPrimitiveDestructorCall(function, typeInstance->selfType(), function.getRawContextValue());
			function.getBuilder().CreateRetVoid();
			
			function.verify();
		}
		
		void genPrimitiveDestructorCall(Function& function, const SEM::Type* type, llvm::Value* value) {
			assert(value->getType()->isPointerTy());
			
			auto& builder = function.getBuilder();
			auto& module = function.module();
			const auto& typeName = type->getObjectType()->name().last();
			
			if (typeName == "value_lval") {
				const auto targetType = type->templateArguments().front();
				const bool typeSizeIsKnown = isTypeSizeKnownInThisModule(module, targetType);
				
				const auto castType = typeSizeIsKnown ? genPointerType(module, type) : TypeGenerator(module).getI8PtrType();
				const auto objectPointer = builder.CreatePointerCast(value, castType);
				
				// Check the 'liveness indicator' which indicates whether
				// child value's destructor should be run.
				const auto livenessIndicatorPtr = typeSizeIsKnown ?
					builder.CreateConstInBoundsGEP2_32(objectPointer, 0, 1) :
					builder.CreateInBoundsGEP(objectPointer, genSizeOf(function, targetType));
				const auto castLivenessIndicatorPtr = builder.CreatePointerCast(livenessIndicatorPtr, TypeGenerator(module).getI1Type()->getPointerTo());
				const auto isLive = builder.CreateLoad(castLivenessIndicatorPtr);
				
				const auto isLiveBB = function.createBasicBlock("is_live");
				const auto afterBB = function.createBasicBlock("");
				
				builder.CreateCondBr(isLive, isLiveBB, afterBB);
				
				// If it is live, run the child value's destructor.
				function.selectBasicBlock(isLiveBB);
				genDestructorCall(function, targetType, objectPointer);
				builder.CreateBr(afterBB);
				
				function.selectBasicBlock(afterBB);
			} else if (typeName == "final_lval" || typeName == "member_lval") {
				const auto targetType = type->templateArguments().front();
				genDestructorCall(function, targetType, value);
			}
		}
		
		bool primitiveTypeHasDestructor(Module& module, const SEM::Type* type) {
			assert(type->isPrimitive());
			const auto& name = type->getObjectType()->name().first();
			const auto kind = module.primitiveKind(name);
			return (kind == PrimitiveMemberLval || kind == PrimitiveValueLval) && typeHasDestructor(module, type->templateArguments().front());
		}
		
		bool primitiveTypeInstanceHasDestructor(Module& module, SEM::TypeInstance* typeInstance) {
			assert(typeInstance->isPrimitive());
			const auto& name = typeInstance->name().first();
			const auto kind = module.primitiveKind(name);
			return (kind == PrimitiveMemberLval || kind == PrimitiveValueLval);
		}
		
	}
	
}

