#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/MethodInfo.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

#include <locic/SEM/Type.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		IREmitter::IREmitter(Function& functionGenerator)
		: functionGenerator_(functionGenerator) { }
		
		llvm::Value*
		IREmitter::emitAlignMask(const SEM::Type* const type) {
			return genAlignMask(functionGenerator_, type);
		}
		
		llvm::Value*
		IREmitter::emitSizeOf(const SEM::Type* const type) {
			return genSizeOf(functionGenerator_, type);
		}
		
		llvm::Value*
		IREmitter::emitAlloca(const SEM::Type* const type,
		                      llvm::Value* const hintResultValue) {
			return genAlloca(functionGenerator_,
			                 type,
			                 hintResultValue);
		}
		
		llvm::Value*
		IREmitter::emitMoveLoad(llvm::Value* const value,
		                        const SEM::Type* const type) {
			return genMoveLoad(functionGenerator_,
			                   value,
			                   type);
		}
		
		void
		IREmitter::emitMoveStore(llvm::Value* const value,
		                         llvm::Value* const memDest,
		                         const SEM::Type* type) {
			return genMoveStore(functionGenerator_,
			                    value,
			                    memDest,
			                    type);
		}
		
		llvm::Value*
		IREmitter::emitLoadDatatypeTag(llvm::Value* const datatypePtr) {
			auto& module = functionGenerator_.module();
			const auto tagPtr = functionGenerator_.getBuilder().CreatePointerCast(datatypePtr,
			                                                                      TypeGenerator(module).getI8PtrType());
			return functionGenerator_.getBuilder().CreateLoad(tagPtr);
		}
		
		void
		IREmitter::emitStoreDatatypeTag(llvm::Value* const tagValue,
		                                llvm::Value* const datatypePtr) {
			auto& module = functionGenerator_.module();
			const auto tagPtr = functionGenerator_.getBuilder().CreatePointerCast(datatypePtr,
			                                                                      TypeGenerator(module).getI8PtrType());
			functionGenerator_.getBuilder().CreateStore(tagValue, tagPtr);
		}
		
		llvm::Value*
		IREmitter::emitGetDatatypeVariantPtr(llvm::Value* const datatypePtr,
		                                     const SEM::Type* const datatypeType,
		                                     const SEM::Type* const variantType) {
			assert(datatypeType->isUnionDatatype());
			assert(variantType->isDatatype());
			
			auto& module = functionGenerator_.module();
			
			llvm::Value* datatypeVariantPtr = nullptr;
			
			// Try to use a plain GEP if possible.
			if (isTypeSizeKnownInThisModule(module, datatypeType)) {
				datatypeVariantPtr = functionGenerator_.getBuilder().CreateConstInBoundsGEP2_32(datatypePtr, 0, 1);
			} else {
				const auto castObjectPtr =
					functionGenerator_.getBuilder().CreatePointerCast(datatypePtr,
					                                                  TypeGenerator(module).getI8PtrType());
				const auto unionAlignValue = genAlignOf(functionGenerator_,
				                                        datatypeType);
				datatypeVariantPtr = functionGenerator_.getBuilder().CreateInBoundsGEP(castObjectPtr,
				                                                                       unionAlignValue);
			}
			
			return functionGenerator_.getBuilder().CreatePointerCast(datatypeVariantPtr,
			                                                         genType(module, variantType)->getPointerTo());
		}
		
		llvm::Value*
		IREmitter::emitImplicitCopyCall(llvm::Value* value,
		                                const SEM::Type* type,
		                                llvm::Value* hintResultValue) {
			return emitCopyCall(METHOD_IMPLICITCOPY,
			                    value,
			                    type,
			                    hintResultValue);
		}
		
		llvm::Value*
		IREmitter::emitExplicitCopyCall(llvm::Value* value,
		                                const SEM::Type* type,
		                                llvm::Value* hintResultValue) {
			return emitCopyCall(METHOD_COPY,
			                    value,
			                    type,
			                    hintResultValue);
		}
		
		llvm::Value*
		IREmitter::emitCopyCall(const MethodID methodID,
		                        llvm::Value* value,
		                        const SEM::Type* rawType,
		                        llvm::Value* hintResultValue) {
			assert(methodID == METHOD_IMPLICITCOPY ||
			       methodID == METHOD_COPY);
			
			const auto& module = functionGenerator_.module();
			
			const auto type = rawType->resolveAliases();
			
			const bool isTemplated = type->isObject() &&
			                         !type->templateArguments().empty();
			
			SEM::FunctionAttributes attributes(/*isVarArg=*/false,
			                                   /*isMethod=*/true,
			                                   isTemplated,
			                                   /*noExceptPredicate=*/SEM::Predicate::False());
			
			SEM::FunctionType functionType(std::move(attributes),
			                               type,
			                               {});
			
			const auto methodName =
				methodID == METHOD_IMPLICITCOPY ?
					module.getCString("implicitcopy") :
					module.getCString("copy");
			
			MethodInfo methodInfo(type,
			                      methodName,
			                      functionType,
			                      {});
			
			RefPendingResult thisPendingResult(value, type);
			
			return genMethodCall(functionGenerator_,
			                     methodInfo,
			                     Optional<PendingResult>(thisPendingResult),
			                     /*args=*/{},
			                     hintResultValue);
		}
		
		llvm::Value*
		IREmitter::emitCompareCall(llvm::Value* const leftValue,
		                           llvm::Value* const rightValue,
		                           const SEM::Type* const compareResultType,
		                           const SEM::Type* const rawThisType,
		                           const SEM::Type* const rawThisRefType) {
			const auto& module = functionGenerator_.module();
			
			const auto thisType = rawThisType->resolveAliases();
			const auto thisRefType = rawThisRefType->resolveAliases();
			
			const bool isTemplated = thisType->isObject() &&
			                         !thisType->templateArguments().empty();
			
			SEM::FunctionAttributes attributes(/*isVarArg=*/false,
			                                   /*isMethod=*/true,
			                                   isTemplated,
			                                   /*noExceptPredicate=*/SEM::Predicate::False());
			
			SEM::FunctionType functionType(std::move(attributes),
			                               compareResultType,
			                               { thisRefType });
			
			MethodInfo methodInfo(thisType,
			                      module.getCString("compare"),
			                      functionType,
			                      {});
			
			RefPendingResult leftValuePendingResult(leftValue, thisType);
			RefPendingResult rightValuePendingResult(rightValue, thisType);
			
			return genMethodCall(functionGenerator_,
			                     methodInfo,
			                     Optional<PendingResult>(leftValuePendingResult),
			                     /*args=*/{ rightValuePendingResult });
		}
		
	}
	
}
