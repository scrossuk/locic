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
		
		static llvm::Instruction* addDebugLoc(llvm::Instruction* instruction, const Optional<llvm::DebugLoc>& debugLocation) {
			if (debugLocation) {
				instruction->setDebugLoc(*debugLocation);
			}
			return instruction;
		}
		
		llvm::Value* genAlloca(Function& function, const SEM::Type* type, Optional<llvm::DebugLoc> debugLoc) {
			SetUseEntryBuilder setUseEntryBuilder(function);
			
			auto& module = function.module();
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					return addDebugLoc(function.getBuilder().CreateAlloca(genType(module, type)), debugLoc);
				}
				
				case SEM::Type::OBJECT:
				case SEM::Type::TEMPLATEVAR: {
					if (isTypeSizeKnownInThisModule(function.module(), type)) {
						return addDebugLoc(function.getBuilder().CreateAlloca(genType(module, type)), debugLoc);
					} else {
						const auto alloca =
							addDebugLoc(function.getEntryBuilder().CreateAlloca(
								TypeGenerator(module).getI8Type(),
								genSizeOf(function, type)), debugLoc);
						return function.getBuilder().CreatePointerCast(alloca,
							genPointerType(module, type));
					}
				}
				
				case SEM::Type::ALIAS: {
					return genAlloca(function, type->resolveAliases(), debugLoc);
				}
				
				default: {
					throw std::runtime_error("Unknown type enum for generating alloca.");
				}
			}
		}
		
		llvm::Value* genLoad(Function& function, llvm::Value* var, const SEM::Type* type, Optional<llvm::DebugLoc> debugLoc) {
			assert(var->getType()->isPointerTy() || type->isInterface());
			assert(var->getType() == genPointerType(function.module(), type));
			
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					return addDebugLoc(function.getBuilder().CreateLoad(var), debugLoc);
				}
				
				case SEM::Type::OBJECT:
				case SEM::Type::TEMPLATEVAR: {
					if (isTypeSizeAlwaysKnown(function.module(), type)) {
						return addDebugLoc(function.getBuilder().CreateLoad(var), debugLoc);
					} else {
						return var;
					}
				}
				
				case SEM::Type::ALIAS: {
					return genLoad(function, var, type->resolveAliases(), debugLoc);
				}
				
				default: {
					llvm_unreachable("Unknown type enum for generating load.");
				}
			}
		}
		
		void genStore(Function& function, llvm::Value* value, llvm::Value* var, const SEM::Type* type, Optional<llvm::DebugLoc> debugLoc) {
			assert(var->getType()->isPointerTy());
			assert(var->getType() == genPointerType(function.module(), type));
			
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					(void) addDebugLoc(function.getBuilder().CreateStore(value, var), debugLoc);
					return;
				}
				
				case SEM::Type::OBJECT:
				case SEM::Type::TEMPLATEVAR: {
					if (isTypeSizeAlwaysKnown(function.module(), type)) {
						// Most primitives will be passed around as values,
						// rather than pointers.
						(void) addDebugLoc(function.getBuilder().CreateStore(value, var), debugLoc);
						return;
					} else {
						if (isTypeSizeKnownInThisModule(function.module(), type)) {
							// If the type size is known now, it's
							// better to generate an explicit load
							// and store (optimisations will be able
							// to make more sense of this).
							(void) addDebugLoc(function.getBuilder().CreateStore(function.getBuilder().CreateLoad(value), var), debugLoc);
						} else {
							// If the type size isn't known, then
							// a memcpy is unavoidable.
							(void) addDebugLoc(function.getBuilder().CreateMemCpy(var, value, genSizeOf(function, type), 1), debugLoc);
						}
						return;
					}
				}
				
				case SEM::Type::ALIAS: {
					return genStore(function, value, var, type->resolveAliases(), debugLoc);
				}
				
				default: {
					llvm_unreachable("Unknown type enum for generating store.");
				}
			}
		}
		
		void genStoreVar(Function& function, llvm::Value* const value, llvm::Value* const var, SEM::Var* const semVar, Optional<llvm::DebugLoc> debugLoc) {
			assert(semVar->isBasic());
			
			const auto valueType = semVar->constructType();
			const auto varType = semVar->type();
			
			if (valueType == varType) {
				genMoveStore(function, value, var, varType, debugLoc);
			} else {
				// If the variable type wasn't actually an lval
				// (very likely), then a value_lval will be created
				// to hold it, and this needs to be constructed.
				genStorePrimitiveLval(function, value, var, varType, debugLoc);
			}
		}
		
		llvm::Value* genValuePtr(Function& function, llvm::Value* value, const SEM::Type* type, Optional<llvm::DebugLoc> debugLoc) {
			// Members must have a pointer to the object, which
			// may require generating a fresh 'alloca'.
			const auto ptrValue = genAlloca(function, type, debugLoc);
			genMoveStore(function, value, ptrValue, type, debugLoc);
			
			// Call destructor for the object at the end of the current scope.
			scheduleDestructorCall(function, type, ptrValue);
			
			return ptrValue;
		}
		
	}
	
}

