#include <assert.h>

#include <stdexcept>
#include <string>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		void genStoreValueLval(Function& function, llvm::Value* value, llvm::Value* var, const SEM::Type* varType) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			// A value lval just contains its target type,
			// so just store that directly.
			const auto targetPtr = builder.CreatePointerCast(var, genPointerType(module, varType->lvalTarget()));
			genMoveStore(function, value, targetPtr, varType->lvalTarget());
		}
		
		void genStoreMemberLval(Function& function, llvm::Value* value, llvm::Value* var, const SEM::Type* varType) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			// A member lval just contains its target type,
			// so just store that directly.
			const auto targetPtr = builder.CreatePointerCast(var, genPointerType(module, varType->lvalTarget()));
			genMoveStore(function, value, targetPtr, varType->lvalTarget());
		}
		
		void genStoreFinalLval(Function& function, llvm::Value* value, llvm::Value* var, const SEM::Type* varType) {
			auto& module = function.module();
			auto& builder = function.getBuilder();
			
			// A final lval just contains its target type,
			// so just store that directly.
			const auto targetPtr = builder.CreatePointerCast(var, genPointerType(module, varType->lvalTarget()));
			genMoveStore(function, value, targetPtr, varType->lvalTarget());
		}
		
		void genStorePrimitiveLval(Function& function, llvm::Value* value, llvm::Value* var, const SEM::Type* varType) {
			assert(var->getType()->isPointerTy());
			
			const auto& typeName = varType->getObjectType()->name().last();
			if (typeName == "value_lval") {
				genStoreValueLval(function, value, var, varType);
			} else if (typeName == "member_lval") {
				genStoreMemberLval(function, value, var, varType);
			} else if (typeName == "final_lval") {
				genStoreFinalLval(function, value, var, varType);
			} else {
				llvm_unreachable("Unknown primitive lval kind.");
			}
		}
		
	}
	
}

