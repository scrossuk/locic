#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/MethodInfo.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/CodeGen/VirtualCall.hpp>

#include <locic/SEM/Type.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		IREmitter::IREmitter(Function& functionGenerator,
		                     llvm::Value* argHintResultValue)
		: functionGenerator_(functionGenerator),
		  hintResultValue_(argHintResultValue) { }
		
		ConstantGenerator
		IREmitter::constantGenerator() {
			return ConstantGenerator(module());
		}
		
		TypeGenerator
		IREmitter::typeGenerator() {
			return TypeGenerator(module());
		}
		
		llvm::Value*
		IREmitter::emitAlignMask(const SEM::Type* const type) {
			return genAlignMask(functionGenerator_, type);
		}
		
		llvm::Value*
		IREmitter::emitSizeOf(const SEM::Type* const type) {
			return genSizeOf(functionGenerator_, type);
		}
		
		llvm::Value*
		IREmitter::emitReturnAlloca(const SEM::Type* const type) {
			return genAlloca(functionGenerator_,
			                 type,
			                 hintResultValue_);
		}
		
		llvm::Value*
		IREmitter::emitAlloca(const SEM::Type* const type) {
			return genAlloca(functionGenerator_,
			                 type);
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
			const auto tagPtr = functionGenerator_.getBuilder().CreatePointerCast(datatypePtr,
			                                                                      TypeGenerator(module()).getI8PtrType());
			return functionGenerator_.getBuilder().CreateLoad(tagPtr);
		}
		
		void
		IREmitter::emitStoreDatatypeTag(llvm::Value* const tagValue,
		                                llvm::Value* const datatypePtr) {
			const auto tagPtr = functionGenerator_.getBuilder().CreatePointerCast(datatypePtr,
			                                                                      TypeGenerator(module()).getI8PtrType());
			functionGenerator_.getBuilder().CreateStore(tagValue, tagPtr);
		}
		
		llvm::Value*
		IREmitter::emitGetDatatypeVariantPtr(llvm::Value* const datatypePtr,
		                                     const SEM::Type* const datatypeType,
		                                     const SEM::Type* const variantType) {
			assert(datatypeType->isUnionDatatype());
			assert(variantType->isDatatype());
			
			llvm::Value* datatypeVariantPtr = nullptr;
			
			// Try to use a plain GEP if possible.
			TypeInfo typeInfo(module());
			if (typeInfo.isSizeKnownInThisModule(datatypeType)) {
				datatypeVariantPtr = functionGenerator_.getBuilder().CreateConstInBoundsGEP2_32(datatypePtr, 0, 1);
			} else {
				const auto castObjectPtr =
					functionGenerator_.getBuilder().CreatePointerCast(datatypePtr,
					                                                  TypeGenerator(module()).getI8PtrType());
				const auto unionAlignValue = genAlignOf(functionGenerator_,
				                                        datatypeType);
				datatypeVariantPtr = functionGenerator_.getBuilder().CreateInBoundsGEP(castObjectPtr,
				                                                                       unionAlignValue);
			}
			
			return functionGenerator_.getBuilder().CreatePointerCast(datatypeVariantPtr,
			                                                         genType(module(), variantType)->getPointerTo());
		}
		
		void
		IREmitter::emitDestructorCall(llvm::Value* const value,
		                              const SEM::Type* const type) {
			if (type->isObject()) {
				if (!typeHasDestructor(module(), type)) {
					return;
				}
				
				if (type->isPrimitive()) {
					genPrimitiveDestructorCall(functionGenerator_, type, value);
					return;
				}
				
				const auto& typeInstance = *(type->getObjectType());
				
				// Call destructor.
				const auto argInfo = destructorArgInfo(module(), typeInstance);
				const auto destructorFunction = genDestructorFunctionDecl(module(), typeInstance);
				
				const auto castValue = functionGenerator_.getBuilder().CreatePointerCast(value, TypeGenerator(module()).getI8PtrType());
				
				llvm::SmallVector<llvm::Value*, 2> args;
				if (!type->templateArguments().empty()) {
					args.push_back(getTemplateGenerator(functionGenerator_, TemplateInst::Type(type)));
				}
				args.push_back(castValue);
				
				(void) genRawFunctionCall(functionGenerator_, argInfo, destructorFunction, args);
			} else if (type->isTemplateVar()) {
				const auto typeInfo = functionGenerator_.getEntryBuilder().CreateExtractValue(functionGenerator_.getTemplateArgs(), { (unsigned int) type->getTemplateVar()->index() });
				const auto castValue = functionGenerator_.getBuilder().CreatePointerCast(value, TypeGenerator(module()).getI8PtrType());
				VirtualCall::generateDestructorCall(functionGenerator_, typeInfo, castValue);
			} else {
				llvm_unreachable("Unknown type kind.");
			}
		}
		
		llvm::Value*
		IREmitter::emitImplicitCopyCall(llvm::Value* value,
		                                const SEM::Type* type,
		                                llvm::Value* callHintResultValue) {
			return emitCopyCall(METHOD_IMPLICITCOPY,
			                    value,
			                    type,
			                    callHintResultValue);
		}
		
		llvm::Value*
		IREmitter::emitExplicitCopyCall(llvm::Value* value,
		                                const SEM::Type* type,
		                                llvm::Value* callHintResultValue) {
			return emitCopyCall(METHOD_COPY,
			                    value,
			                    type,
			                    callHintResultValue);
		}
		
		llvm::Value*
		IREmitter::emitCopyCall(const MethodID methodID,
		                        llvm::Value* value,
		                        const SEM::Type* rawType,
		                        llvm::Value* callHintResultValue) {
			assert(methodID == METHOD_IMPLICITCOPY ||
			       methodID == METHOD_COPY);
			
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
					module().getCString("implicitcopy") :
					module().getCString("copy");
			
			MethodInfo methodInfo(type,
			                      methodName,
			                      functionType,
			                      {});
			
			RefPendingResult thisPendingResult(value, type);
			
			return genMethodCall(functionGenerator_,
			                     methodInfo,
			                     Optional<PendingResult>(thisPendingResult),
			                     /*args=*/{},
			                     callHintResultValue);
		}
		
		llvm::Value*
		IREmitter::emitCompareCall(llvm::Value* const leftValue,
		                           llvm::Value* const rightValue,
		                           const SEM::Type* const compareResultType,
		                           const SEM::Type* const rawThisType,
		                           const SEM::Type* const rawThisRefType) {
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
			                      module().getCString("compare"),
			                      functionType,
			                      {});
			
			RefPendingResult leftValuePendingResult(leftValue, thisType);
			RefPendingResult rightValuePendingResult(rightValue, thisType);
			
			return genMethodCall(functionGenerator_,
			                     methodInfo,
			                     Optional<PendingResult>(leftValuePendingResult),
			                     /*args=*/{ rightValuePendingResult });
		}
		
		llvm::IRBuilder<>& IREmitter::builder() {
			return functionGenerator_.getBuilder();
		}
		
		Function& IREmitter::function() {
			return functionGenerator_;
		}
		
		llvm::Value* IREmitter::hintResultValue() {
			return hintResultValue_;
		}
		
		Module& IREmitter::module() {
			return functionGenerator_.module();
		}
		
	}
	
}
