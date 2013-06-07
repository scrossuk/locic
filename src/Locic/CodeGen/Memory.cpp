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
			Module& module = function.getModule();
			SEM::Type* type = module.resolveType(unresolvedType);
			
			if (!type->isClass()) {
				return;
			}
			
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
				case SEM::Type::POINTER:
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
		
		llvm::Value* genStore(Function& function, llvm::Value* value, llvm::Value* var, SEM::Type* unresolvedType) {
			LOG(LOG_NOTICE, "Store.");
			value->dump();
			var->dump();
			
			SEM::Type* type = function.getModule().resolveType(unresolvedType);
			
			switch (type->kind()) {
				case SEM::Type::VOID:
				case SEM::Type::NULLT:
				case SEM::Type::POINTER:
				case SEM::Type::REFERENCE:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					return function.getBuilder().CreateStore(value, var);
				}
				
				case SEM::Type::OBJECT: {
					SEM::TypeInstance* typeInstance = type->getObjectType();
					
					if (typeInstance->isPrimitive() || typeInstance->isStruct()) {
						return function.getBuilder().CreateStore(value, var);
					} else {
						if (typeInstance->isDefinition()) {
							return function.getBuilder().CreateStore(function.getBuilder().CreateLoad(value), var);
						} else {
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
		
		void genMoveStore(Function& function, llvm::Value* source, llvm::Value* dest, SEM::Type* type) {
			// Destroy existing value in destination.
			genDestructorCall(function, type, dest);
			
			// Do store.
			(void) genStore(function, source, dest, type);
			
			// Zero out source.
			genZero(function, type, source);
		}
		
		llvm::Value* genLoad(Function& function, llvm::Value* var, SEM::Type* unresolvedType) {
			SEM::Type* type = function.getModule().resolveType(unresolvedType);
			
			switch (type->kind()) {
				case SEM::Type::VOID:
				case SEM::Type::NULLT:
				case SEM::Type::POINTER:
				case SEM::Type::REFERENCE:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD: {
					return function.getBuilder().CreateLoad(var);
				}
				
				case SEM::Type::OBJECT: {
					SEM::TypeInstance* typeInstance = type->getObjectType();
					
					if (typeInstance->isPrimitive() || typeInstance->isStruct()) {
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
		
	}
	
}

