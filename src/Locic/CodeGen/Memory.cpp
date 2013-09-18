#include <Locic/SEM.hpp>
#include <Locic/CodeGen/ConstantGenerator.hpp>
#include <Locic/CodeGen/Destructor.hpp>
#include <Locic/CodeGen/Function.hpp>
#include <Locic/CodeGen/GenType.hpp>
#include <Locic/CodeGen/GenVTable.hpp>
#include <Locic/CodeGen/Module.hpp>
#include <Locic/CodeGen/SizeOf.hpp>
#include <Locic/CodeGen/TypeGenerator.hpp>

namespace Locic {

	namespace CodeGen {
		
		void genZeroStore(Function& function, llvm::Value* value, SEM::Type* type) {
			assert(value->getType()->isPointerTy());
			
			Module& module = function.getModule();
			(void) function.getBuilder().CreateStore(
				ConstantGenerator(module).getNull(genType(module, type)),
				value);
		}
	
		void genZero(Function& function, SEM::Type* unresolvedType, llvm::Value* value) {
			assert(value->getType()->isPointerTy());
			
			Module& module = function.getModule();
			SEM::Type* type = module.resolveType(unresolvedType);
			
			(void) function.getBuilder().CreateStore(
				ConstantGenerator(module).getNull(genType(module, type)),
				value);
		}
		
		llvm::Value* genUnzeroedAlloca(Function& function, SEM::Type* type) {
			Module& module = function.getModule();
			llvm::Type* rawType = genType(module, type);
			
			switch (type->kind()) {
				case SEM::Type::VOID:
				case SEM::Type::NULLT:
				case SEM::Type::REFERENCE:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					return function.getBuilder().CreateAlloca(rawType);
				}
				
				case SEM::Type::OBJECT: {
					SEM::TypeInstance* typeInstance = type->getObjectType();
					
					assert(!typeInstance->isInterface());
					
					if (typeInstance->isPrimitive() || typeInstance->isDefinition()) {
						return function.getBuilder().CreateAlloca(rawType);
					} else {
						llvm::Value* alloca =
							function.getBuilder().CreateAlloca(
								TypeGenerator(module).getI8Type(),
								genSizeOf(function, type));
						return function.getBuilder().CreatePointerCast(alloca,
							rawType->getPointerTo());
					}
				}
				
				case SEM::Type::TEMPLATEVAR: {
					assert(false && "Can't alloca template var.");
					return NULL;
				}
				
				default: {
					assert(false && "Unknown type enum for generating alloca.");
					return NULL;
				}
			}
		}
		
		llvm::Value* genAlloca(Function& function, SEM::Type* unresolvedType) {
			Module& module = function.getModule();
			SEM::Type* type = module.resolveType(unresolvedType);
			
			llvm::Value* alloca = genUnzeroedAlloca(function, type);
			
			// Zero allocated memory.
			genZero(function, type, alloca);
			
			assert(!function.destructorScopeStack().empty());
			function.destructorScopeStack().back().push_back(std::make_pair(type, alloca));
			
			return alloca;
		}
		
		llvm::Value* genLoad(Function& function, llvm::Value* var, SEM::Type* unresolvedType) {
			SEM::Type* type = function.getModule().resolveType(unresolvedType);
			
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
					if (isTypeSizeAlwaysKnown(function.getModule(), type)) {
						return function.getBuilder().CreateLoad(var);
					} else {
						return var;
					}
				}
				
				case SEM::Type::TEMPLATEVAR: {
					assert(false && "Can't load template var.");
					return NULL;
				}
				
				default: {
					assert(false && "Unknown type enum for generating load.");
					return NULL;
				}
			}
		}
		
		llvm::Value* genStore(Function& function, llvm::Value* value, llvm::Value* var, SEM::Type* unresolvedType) {
			assert(var->getType()->isPointerTy());
			
			SEM::Type* type = function.getModule().resolveType(unresolvedType);
			
			switch (type->kind()) {
				case SEM::Type::VOID:
				case SEM::Type::NULLT:
				case SEM::Type::REFERENCE:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					return function.getBuilder().CreateStore(value, var);
				}
				
				case SEM::Type::OBJECT: {
					if (isTypeSizeAlwaysKnown(function.getModule(), type)) {
						return function.getBuilder().CreateStore(value, var);
					} else {	
						if (isTypeSizeKnownInThisModule(function.getModule(), type)) {
							return function.getBuilder().CreateStore(function.getBuilder().CreateLoad(value), var);
						} else {
							assert(false && "sizeof is currently broken, so memcpy won't work correctly...");
							return function.getBuilder().CreateMemCpy(var, value,
								genSizeOf(function, type), 1);
						}
					}
				}
				
				case SEM::Type::TEMPLATEVAR: {
					assert(false && "Can't store template var.");
					return NULL;
				}
				
				default: {
					assert(false && "Unknown type enum for generating store.");
					return NULL;
				}
			}
		}
		
		void genMove(Function& function, llvm::Value* source, llvm::Value* dest, SEM::Type* type) {
			assert(source->getType()->isPointerTy());
			assert(dest->getType()->isPointerTy());
			
			// Destroy existing value in destination.
			genDestructorCall(function, type, dest);
			
			// Do store.
			(void) genStore(function, genLoad(function, source, type), dest, type);
			
			// Zero out source.
			genZero(function, type, source);
		}
		
	}
	
}

