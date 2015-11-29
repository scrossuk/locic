#include <stdexcept>

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
		
		llvm::Value* genRawAlloca(Function& function, const SEM::Type* const type, llvm::Value* const hintResultValue) {
			if (hintResultValue != nullptr) {
				assert(hintResultValue->getType()->isPointerTy());
				return hintResultValue;
			}
			
			SetUseEntryBuilder setUseEntryBuilder(function);
			
			auto& module = function.module();
			switch (type->kind()) {
				case SEM::Type::OBJECT:
				case SEM::Type::TEMPLATEVAR: {
					TypeInfo typeInfo(module);
					if (typeInfo.isSizeKnownInThisModule(type)) {
						const auto llvmType = genType(module, type);
						assert(!llvmType->isVoidTy());
						return function.getBuilder().CreateAlloca(llvmType);
					} else {
						return function.getEntryBuilder().CreateAlloca(
								TypeGenerator(module).getI8Type(),
								genSizeOf(function, type));
					}
				}
				
				case SEM::Type::ALIAS: {
					return genRawAlloca(function, type->resolveAliases(), hintResultValue);
				}
				
				default: {
					throw std::runtime_error("Unknown type enum for generating alloca.");
				}
			}
		}
		
		llvm::Value* genAlloca(Function& function, const SEM::Type* const type, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			const bool shouldZeroAlloca = module.buildOptions().zeroAllAllocas;
			
			IREmitter irEmitter(function, hintResultValue);
			
			const auto allocaValue = genRawAlloca(function,
			                                      type,
			                                      hintResultValue);
			
			if (shouldZeroAlloca && hintResultValue == nullptr) {
				const auto typeSizeValue = genSizeOf(function, type);
				irEmitter.emitMemSet(allocaValue,
				                     ConstantGenerator(module).getI8(0),
				                     typeSizeValue,
				                     /*align=*/1);
			}
			
			return allocaValue;
		}
		
		llvm::Value* genLoad(Function& function, llvm::Value* const var, const SEM::Type* const type) {
			assert(var->getType()->isPointerTy());
			
			IREmitter irEmitter(function);
			
			switch (type->kind()) {
				case SEM::Type::OBJECT:
				case SEM::Type::TEMPLATEVAR: {
					TypeInfo typeInfo(function.module());
					if (typeInfo.isSizeAlwaysKnown(type)) {
						const auto valueType = genType(function.module(), type);
						return irEmitter.emitRawLoad(var,
						                             valueType);
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
		
		void genStore(Function& function, llvm::Value* const value, llvm::Value* const var, const SEM::Type* const type) {
			assert(var->getType()->isPointerTy());
			
			IREmitter irEmitter(function);
			
			switch (type->kind()) {
				case SEM::Type::OBJECT:
				case SEM::Type::TEMPLATEVAR: {
					TypeInfo typeInfo(function.module());
					if (typeInfo.isSizeAlwaysKnown(type)) {
						// Most primitives will be passed around as values,
						// rather than pointers.
						irEmitter.emitRawStore(value, var);
						return;
					} else {
						if (value->stripPointerCasts() == var->stripPointerCasts()) {
							// Source and destination are same pointer, so no
							// move operation required!
							return;
						}
						
						if (typeInfo.isSizeKnownInThisModule(type)) {
							// If the type size is known now, it's
							// better to generate an explicit load
							// and store (optimisations will be able
							// to make more sense of this).
							const auto loadedValue = irEmitter.emitRawLoad(value,
							                                               genType(function.module(), type));
							irEmitter.emitRawStore(loadedValue, var);
						} else {
							// If the type size isn't known, then
							// a memcpy is unavoidable.
							irEmitter.emitMemCpy(var,
							                     value,
							                     genSizeOf(function, type),
							                     1);
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
				IREmitter irEmitter(function);
				irEmitter.emitMoveStore(value, var, varType);
			} else {
				// If the variable type wasn't actually an lval
				// (very likely), then a value_lval will be created
				// to hold it, and this needs to be constructed.
				genStorePrimitiveLval(function, value, var, varType);
			}
		}
		
		llvm::Value* genValuePtr(Function& function, llvm::Value* const value, const SEM::Type* const type, llvm::Value* hintResultValue) {
			// Members must have a pointer to the object, which
			// may require generating a fresh 'alloca'.
			IREmitter irEmitter(function, hintResultValue);
			const auto ptrValue = irEmitter.emitReturnAlloca(type);
			irEmitter.emitMoveStore(value, ptrValue, type);
			
			// Call destructor for the object at the end of the current scope.
			scheduleDestructorCall(function, type, ptrValue);
			
			return ptrValue;
		}
		
	}
	
}

