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
		
		void genZero(Function& function, SEM::Type* unresolvedType, llvm::Value* value) {
			assert(value->getType()->isPointerTy());
			
			auto& module = function.module();
			const auto type = module.resolveType(unresolvedType);
			
			(void) function.getBuilder().CreateStore(
				ConstantGenerator(module).getNull(genType(module, type)),
				value);
		}
		
		llvm::Value* genUnzeroedAlloca(Function& function, SEM::Type* type) {
			auto& module = function.module();
			const auto rawType = genType(module, type);
			
			switch (type->kind()) {
				case SEM::Type::VOID:
				case SEM::Type::NULLT:
				case SEM::Type::REFERENCE:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					return function.getBuilder().CreateAlloca(rawType);
				}
				
				case SEM::Type::OBJECT: {
					const auto typeInstance = type->getObjectType();
					
					assert(!typeInstance->isInterface());
					
					if (typeInstance->isClassDecl()) {
						const auto alloca =
							function.getBuilder().CreateAlloca(
								TypeGenerator(module).getI8Type(),
								genSizeOf(function, type));
						return function.getBuilder().CreatePointerCast(alloca,
							rawType->getPointerTo());
					} else {
						return function.getBuilder().CreateAlloca(rawType);
					}
				}
				
				case SEM::Type::TEMPLATEVAR: {
					throw std::runtime_error("Can't alloca template var.");
				}
				
				default: {
					throw std::runtime_error("Unknown type enum for generating alloca.");
				}
			}
		}
		
		llvm::Value* genAlloca(Function& function, SEM::Type* unresolvedType) {
			auto& module = function.module();
			const auto type = module.resolveType(unresolvedType);
			
			const auto alloca = genUnzeroedAlloca(function, type);
			
			// Zero allocated memory.
			genZero(function, type, alloca);
			
			return alloca;
		}
		
		llvm::Value* genLoad(Function& function, llvm::Value* var, SEM::Type* unresolvedType) {
			const auto type = function.module().resolveType(unresolvedType);
			
			assert(var->getType()->isPointerTy() || type->isInterface());
			
			switch (type->kind()) {
				case SEM::Type::VOID:
				case SEM::Type::NULLT:
				case SEM::Type::REFERENCE:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					return function.getBuilder().CreateLoad(var);
				}
				
				case SEM::Type::OBJECT: {
					if (isTypeSizeAlwaysKnown(function.module(), type)) {
						return function.getBuilder().CreateLoad(var);
					} else {
						return var;
					}
				}
				
				case SEM::Type::TEMPLATEVAR: {
					throw std::runtime_error("Can't load template var.");
				}
				
				default: {
					throw std::runtime_error("Unknown type enum for generating load.");
				}
			}
		}
		
		void genStore(Function& function, llvm::Value* value, llvm::Value* var, SEM::Type* unresolvedType) {
			assert(var->getType()->isPointerTy());
			
			const auto type = function.module().resolveType(unresolvedType);
			
			switch (type->kind()) {
				case SEM::Type::VOID:
				case SEM::Type::NULLT:
				case SEM::Type::REFERENCE:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					function.getBuilder().CreateStore(value, var);
					return;
				}
				
				case SEM::Type::OBJECT: {
					if (isTypeSizeAlwaysKnown(function.module(), type)) {
						// Most primitives will be passed around as values,
						// rather than pointers.
						function.getBuilder().CreateStore(value, var);
						return;
					} else {	
						if (isTypeSizeKnownInThisModule(function.module(), type)) {
							// If the type size is known now, it's
							// better to generate an explicit load
							// and store (optimisations will be able
							// to make more sense of this).
							function.getBuilder().CreateStore(function.getBuilder().CreateLoad(value), var);
						} else {
							// If the type size isn't known, then
							// a memcpy is unavoidable.
							function.getBuilder().CreateMemCpy(var, value, genSizeOf(function, type), 1);
						}
						return;
					}
				}
				
				case SEM::Type::TEMPLATEVAR: {
					throw std::runtime_error("Can't store template var.");
				}
				
				default: {
					throw std::runtime_error("Unknown type enum for generating store.");
				}
			}
		}
		
		void genStoreVar(Function& function, llvm::Value* value, llvm::Value* var, SEM::Var* semVar) {
			assert(semVar->isBasic());
			
			auto& module = function.module();
			
			const auto valueType = module.resolveType(semVar->constructType());
			const auto varType = module.resolveType(semVar->type());
			
			// If the variable type wasn't actually an lval
			// (very likely), then a value_lval will be created
			// to hold it, and this needs to be constructed.
			if (*(valueType) == *(varType)) {
				genStore(function, value, var, varType);
			} else {
				genStorePrimitiveLval(function, value, var, varType);
			}
		}
		
	}
	
}

