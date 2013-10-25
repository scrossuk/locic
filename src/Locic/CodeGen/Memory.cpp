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
		
		void genStore(Function& function, llvm::Value* value, llvm::Value* var, SEM::Type* unresolvedType) {
			assert(var->getType()->isPointerTy());
			
			SEM::Type* type = function.getModule().resolveType(unresolvedType);
			
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
					if (isTypeSizeAlwaysKnown(function.getModule(), type)) {
						// Most primitives will be passed around as values,
						// rather than pointers, and also don't need destructors
						// to be run.
						function.getBuilder().CreateStore(value, var);
						return;
					} else {	
						// Destroy existing value in destination.
						genDestructorCall(function, type, var);
						
						// Do store.
						if (isTypeSizeKnownInThisModule(function.getModule(), type)) {
							function.getBuilder().CreateStore(function.getBuilder().CreateLoad(value), var);
						} else {
							assert(false && "sizeof is currently broken, so memcpy won't work correctly...");
							function.getBuilder().CreateMemCpy(var, value,
								genSizeOf(function, type), 1);
						}
						
						// Zero out source.
						genZero(function, type, value);
						return;
					}
				}
				
				case SEM::Type::TEMPLATEVAR: {
					assert(false && "Can't store template var.");
					return;
				}
				
				default: {
					assert(false && "Unknown type enum for generating store.");
					return;
				}
			}
		}
		
	}
	
}

