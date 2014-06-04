#include <stdexcept>

#include <locic/SEM.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
		
		void genZero(Function& function, SEM::Type* type, llvm::Value* value) {
			assert(value->getType()->isPointerTy());
			
			auto& module = function.module();
			
			if (isTypeSizeKnownInThisModule(module, type)) {
				(void) function.getBuilder().CreateStore(
					ConstantGenerator(module).getNull(genType(module, type)),
					value);
			} else {
				(void) function.getBuilder().CreateMemSet(
					value, ConstantGenerator(module).getI8(0),
					genSizeOf(function, type), 1);
			}
		}
		
		llvm::Value* genUnzeroedAlloca(Function& function, SEM::Type* type) {
			auto& module = function.module();
			switch (type->kind()) {
				case SEM::Type::VOID:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					return function.getEntryBuilder().CreateAlloca(genType(module, type));
				}
				
				case SEM::Type::OBJECT:
				case SEM::Type::TEMPLATEVAR: {
					if (isTypeSizeKnownInThisModule(function.module(), type)) {
						return function.getEntryBuilder().CreateAlloca(genType(module, type));
					} else {
						const auto alloca =
							function.getEntryBuilder().CreateAlloca(
								TypeGenerator(module).getI8Type(),
								genSizeOf(function, type));
						return function.getBuilder().CreatePointerCast(alloca,
							genPointerType(module, type));
					}
				}
				
				default: {
					throw std::runtime_error("Unknown type enum for generating alloca.");
				}
			}
		}
		
		llvm::Value* genAlloca(Function& function, SEM::Type* type) {
			const auto alloca = genUnzeroedAlloca(function, type);
			
			// Zero allocated memory.
			genZero(function, type, alloca);
			
			return alloca;
		}
		
		llvm::Value* genLoad(Function& function, llvm::Value* var, SEM::Type* type) {
			assert(var->getType()->isPointerTy() || type->isInterface());
			
			switch (type->kind()) {
				case SEM::Type::VOID:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					return function.getBuilder().CreateLoad(var);
				}
				
				case SEM::Type::OBJECT:
				case SEM::Type::TEMPLATEVAR: {
					if (isTypeSizeAlwaysKnown(function.module(), type)) {
						return function.getBuilder().CreateLoad(var);
					} else {
						return var;
					}
				}
				
				default: {
					throw std::runtime_error("Unknown type enum for generating load.");
				}
			}
		}
		
		void genStore(Function& function, llvm::Value* value, llvm::Value* var, SEM::Type* type) {
			assert(var->getType()->isPointerTy());
			const auto castVar = function.getBuilder().CreatePointerCast(var, genPointerType(function.module(), type));
			
			switch (type->kind()) {
				case SEM::Type::VOID:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					function.getBuilder().CreateStore(value, castVar);
					return;
				}
				
				case SEM::Type::OBJECT:
				case SEM::Type::TEMPLATEVAR: {
					if (isTypeSizeAlwaysKnown(function.module(), type)) {
						// Most primitives will be passed around as values,
						// rather than pointers.
						function.getBuilder().CreateStore(value, castVar);
						return;
					} else {
						if (isTypeSizeKnownInThisModule(function.module(), type)) {
							// If the type size is known now, it's
							// better to generate an explicit load
							// and store (optimisations will be able
							// to make more sense of this).
							function.getBuilder().CreateStore(function.getBuilder().CreateLoad(value), castVar);
						} else {
							// If the type size isn't known, then
							// a memcpy is unavoidable.
							function.getBuilder().CreateMemCpy(castVar, value, genSizeOf(function, type), 1);
						}
						return;
					}
				}
				
				default: {
					throw std::runtime_error("Unknown type enum for generating store.");
				}
			}
		}
		
		void genStoreVar(Function& function, llvm::Value* value, llvm::Value* var, SEM::Var* semVar) {
			assert(semVar->isBasic());
			
			const auto valueType = semVar->constructType();
			const auto varType = semVar->type();
			
			if (*(valueType) == *(varType)) {
				genStore(function, value, var, varType);
			} else {
				// If the variable type wasn't actually an lval
				// (very likely), then a value_lval will be created
				// to hold it, and this needs to be constructed.
				genStorePrimitiveLval(function, value, var, varType);
			}
		}
		
	}
	
}

