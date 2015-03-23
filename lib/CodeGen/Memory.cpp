#include <stdexcept>

#include <locic/SEM.hpp>
#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Value* genAlloca(Function& function, const SEM::Type* type) {
			SetUseEntryBuilder setUseEntryBuilder(function);
			
			auto& module = function.module();
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					return function.getBuilder().CreateAlloca(genType(module, type));
				}
				
				case SEM::Type::OBJECT:
				case SEM::Type::TEMPLATEVAR: {
					if (isTypeSizeKnownInThisModule(function.module(), type)) {
						return function.getBuilder().CreateAlloca(genType(module, type));
					} else {
						const auto alloca =
							function.getEntryBuilder().CreateAlloca(
								TypeGenerator(module).getI8Type(),
								genSizeOf(function, type));
						return function.getBuilder().CreatePointerCast(alloca,
							genPointerType(module, type));
					}
				}
				
				case SEM::Type::ALIAS: {
					return genAlloca(function, type->resolveAliases());
				}
				
				default: {
					throw std::runtime_error("Unknown type enum for generating alloca.");
				}
			}
		}
		
		llvm::Value* genLoad(Function& function, llvm::Value* var, const SEM::Type* type) {
			assert(var->getType()->isPointerTy() || type->isInterface());
			assert(var->getType() == genPointerType(function.module(), type));
			
			switch (type->kind()) {
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
				
				case SEM::Type::ALIAS: {
					return genLoad(function, var, type->resolveAliases());
				}
				
				default: {
					llvm_unreachable("Unknown type enum for generating load.");
				}
			}
		}
		
		void genStore(Function& function, llvm::Value* value, llvm::Value* var, const SEM::Type* type) {
			assert(var->getType()->isPointerTy());
			assert(var->getType() == genPointerType(function.module(), type));
			
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					(void) function.getBuilder().CreateStore(value, var);
					return;
				}
				
				case SEM::Type::OBJECT:
				case SEM::Type::TEMPLATEVAR: {
					if (isTypeSizeAlwaysKnown(function.module(), type)) {
						// Most primitives will be passed around as values,
						// rather than pointers.
						(void) function.getBuilder().CreateStore(value, var);
						return;
					} else {
						if (isTypeSizeKnownInThisModule(function.module(), type)) {
							// If the type size is known now, it's
							// better to generate an explicit load
							// and store (optimisations will be able
							// to make more sense of this).
							(void) function.getBuilder().CreateStore(function.getBuilder().CreateLoad(value), var);
						} else {
							// If the type size isn't known, then
							// a memcpy is unavoidable.
							(void) function.getBuilder().CreateMemCpy(var, value, genSizeOf(function, type), 1);
						}
						return;
					}
				}
				
				case SEM::Type::ALIAS: {
					return genStore(function, value, var, type->resolveAliases());
				}
				
				default: {
					llvm_unreachable("Unknown type enum for generating store.");
				}
			}
		}
		
		void genStoreVar(Function& function, llvm::Value* const value, llvm::Value* const var, SEM::Var* const semVar) {
			assert(semVar->isBasic());
			
			const auto valueType = semVar->constructType();
			const auto varType = semVar->type();
			
			if (valueType == varType) {
				genMoveStore(function, value, var, varType);
			} else {
				// If the variable type wasn't actually an lval
				// (very likely), then a value_lval will be created
				// to hold it, and this needs to be constructed.
				genStorePrimitiveLval(function, value, var, varType);
			}
		}
		
		llvm::Value* genValuePtr(Function& function, llvm::Value* value, const SEM::Type* type) {
			// Members must have a pointer to the object, which
			// may require generating a fresh 'alloca'.
			const auto ptrValue = genAlloca(function, type);
			genMoveStore(function, value, ptrValue, type);
			
			// Call destructor for the object at the end of the current scope.
			scheduleDestructorCall(function, type, ptrValue);
			
			return ptrValue;
		}
		
	}
	
}

