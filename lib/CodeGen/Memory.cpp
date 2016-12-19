#include <stdexcept>

#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Var.hpp>
#include <locic/SEM.hpp>
#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>

namespace locic {

	namespace CodeGen {
		
		void genStoreVar(Function& function, llvm::Value* const value, llvm::Value* const varPtr, AST::Var* const var) {
			assert(var->isNamed());
			
			const auto valueType = var->constructType();
			const auto varType = var->lvalType();
			
			if (valueType == varType) {
				IREmitter irEmitter(function);
				irEmitter.emitMoveStore(value, varPtr, varType);
			} else {
				// If the variable type wasn't actually an lval
				// (very likely), then a value_lval will be created
				// to hold it, and this needs to be constructed.
				genStorePrimitiveLval(function, value, varPtr, varType);
			}
		}
		
		llvm::Value* genValuePtr(Function& function, llvm::Value* const value, const AST::Type* const type,
		                         llvm::Value* hintResultValue) {
			// Members must have a pointer to the object, which
			// may require generating a fresh 'alloca'.
			IREmitter irEmitter(function);
			const auto ptrValue = irEmitter.emitAlloca(type, hintResultValue);
			irEmitter.emitMoveStore(value, ptrValue, type);
			
			// Call destructor for the object at the end of the current scope.
			scheduleDestructorCall(function, type, ptrValue);
			
			return ptrValue;
		}
		
	}
	
}

