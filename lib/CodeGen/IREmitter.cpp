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
#include <locic/CodeGen/VirtualCallABI.hpp>

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
		IREmitter::emitI1ToBool(llvm::Value* const value) {
			assert(value->getType()->isIntegerTy(1));
			return functionGenerator_.getBuilder().CreateZExt(value,
			                                                  typeGenerator().getI8Type());
		}
		
		llvm::Value*
		IREmitter::emitBoolToI1(llvm::Value* const value) {
			assert(value->getType()->isIntegerTy(8));
			return functionGenerator_.getBuilder().CreateICmpNE(value,
			                                                    constantGenerator().getBool(false));
		}
		
		llvm::Value*
		IREmitter::emitRawAlloca(llvm::Type* const type) {
			return functionGenerator_.getEntryBuilder().CreateAlloca(type);
		}
		
		llvm::Value*
		IREmitter::emitRawLoad(llvm::Value* const valuePtr,
		                       llvm::Type* const type) {
			assert(valuePtr->getType()->isPointerTy());
			const auto castVar = functionGenerator_.getBuilder().CreatePointerCast(valuePtr,
			                                                                       type->getPointerTo());
			return functionGenerator_.getBuilder().CreateLoad(castVar);
		}
		
		void
		IREmitter::emitRawStore(llvm::Value* const value,
		                        llvm::Value* const var) {
			assert(var->getType()->isPointerTy());
			const auto castVar = functionGenerator_.getBuilder().CreatePointerCast(var,
			                                                                       value->getType()->getPointerTo());
			(void) functionGenerator_.getBuilder().CreateStore(value,
			                                                   castVar);
		}
		
		llvm::Value*
		IREmitter::emitInBoundsGEP(llvm::Type* const type,
		                           llvm::Value* const ptrValue,
		                           llvm::Value* const indexValue) {
			assert(ptrValue->getType()->isPointerTy());
			assert(indexValue->getType()->isIntegerTy());
			const auto castValue = functionGenerator_.getBuilder().CreatePointerCast(ptrValue,
			                                                                         type->getPointerTo());
			return functionGenerator_.getBuilder().CreateInBoundsGEP(castValue,
			                                                         indexValue);
		}
		
		llvm::Value*
		IREmitter::emitInBoundsGEP(llvm::Type* const type,
		                           llvm::Value* const ptrValue,
		                           llvm::ArrayRef<llvm::Value*> indexArray) {
			assert(ptrValue->getType()->isPointerTy());
			const auto castValue = functionGenerator_.getBuilder().CreatePointerCast(ptrValue,
			                                                                         type->getPointerTo());
			return functionGenerator_.getBuilder().CreateInBoundsGEP(castValue,
			                                                         indexArray);
		}
		
		llvm::Value*
		IREmitter::emitConstInBoundsGEP2_32(llvm::Type* const type,
		                                    llvm::Value* const ptrValue,
		                                    const unsigned index0,
		                                    const unsigned index1) {
			assert(ptrValue->getType()->isPointerTy());
			const auto castValue = functionGenerator_.getBuilder().CreatePointerCast(ptrValue,
			                                                                         type->getPointerTo());
#if LOCIC_LLVM_VERSION >= 307
			return functionGenerator_.getBuilder().CreateConstInBoundsGEP2_32(type,
			                                                                  castValue,
			                                                                  index0,
			                                                                  index1);
#else
			return functionGenerator_.getBuilder().CreateConstInBoundsGEP2_32(castValue,
			                                                                  index0,
			                                                                  index1);
#endif
		}
		
		llvm::Value*
		IREmitter::emitInsertValue(llvm::Value* const aggregate,
		                           llvm::Value* const value,
		                           llvm::ArrayRef<unsigned> indexArray) {
			assert(aggregate->getType()->isAggregateType());
			const auto indexType = llvm::ExtractValueInst::getIndexedType(aggregate->getType(),
			                                                              indexArray);
			if (indexType->isPointerTy()) {
				assert(value->getType()->isPointerTy());
				const auto castValue = builder().CreatePointerCast(value,
				                                                   indexType);
				return builder().CreateInsertValue(aggregate,
				                                   castValue,
				                                   indexArray);
			} else {
				assert(!value->getType()->isPointerTy());
				return builder().CreateInsertValue(aggregate,
				                                   value,
				                                   indexArray);
			}
		}
		
		void
		IREmitter::emitMemSet(llvm::Value* const ptr,
		                      llvm::Value* const value,
		                      const uint64_t size,
		                      const unsigned align) {
			assert(ptr->getType()->isPointerTy());
			const auto castPtr = builder().CreatePointerCast(ptr,
			                                                 typeGenerator().getPtrType());
			builder().CreateMemSet(castPtr, value, size, align);
		}
		
		void
		IREmitter::emitMemSet(llvm::Value* const ptr,
		                      llvm::Value* const value,
		                      llvm::Value* const sizeValue,
		                      const unsigned align) {
			assert(ptr->getType()->isPointerTy());
			const auto castPtr = builder().CreatePointerCast(ptr,
			                                                 typeGenerator().getPtrType());
			builder().CreateMemSet(castPtr, value, sizeValue, align);
		}
		
		void
		IREmitter::emitMemCpy(llvm::Value* const dest,
		                      llvm::Value* const src,
		                      const uint64_t size,
		                      const unsigned align) {
			assert(dest->getType()->isPointerTy());
			assert(src->getType()->isPointerTy());
			const auto castDest = builder().CreatePointerCast(dest,
			                                                  typeGenerator().getPtrType());
			const auto castSrc = builder().CreatePointerCast(src,
			                                                 typeGenerator().getPtrType());
			builder().CreateMemCpy(castDest, castSrc, size, align);
		}
		
		void
		IREmitter::emitMemCpy(llvm::Value* const dest,
		                      llvm::Value* const src,
		                      llvm::Value* const sizeValue,
		                      const unsigned align) {
			assert(dest->getType()->isPointerTy());
			assert(src->getType()->isPointerTy());
			const auto castDest = builder().CreatePointerCast(dest,
			                                                  typeGenerator().getPtrType());
			const auto castSrc = builder().CreatePointerCast(src,
			                                                 typeGenerator().getPtrType());
			builder().CreateMemCpy(castDest, castSrc, sizeValue, align);
		}
		
		llvm::CallInst*
		IREmitter::emitCall(llvm::FunctionType* const functionType,
		                    llvm::Value* const callee,
		                    llvm::ArrayRef<llvm::Value*> args) {
			assert(callee->getType()->isPointerTy());
			const auto castCallee = functionGenerator_.getBuilder().CreatePointerCast(callee,
			                                                                          functionType->getPointerTo());
			
			// Cast all pointers to required types.
			llvm::SmallVector<llvm::Value*, 10> newArgs;
			newArgs.reserve(args.size());
			for (size_t i = 0; i < functionType->getNumParams(); i++) {
				const auto& arg = args[i];
				if (arg->getType()->isPointerTy()) {
					assert(functionType->getParamType(i)->isPointerTy());
					newArgs.push_back(builder().CreatePointerCast(arg,
					                                              functionType->getParamType(i)));
				} else {
					assert(!functionType->getParamType(i)->isPointerTy());
					newArgs.push_back(arg);
				}
			}
			
			for (size_t i = functionType->getNumParams(); i < args.size(); i++) {
				newArgs.push_back(args[i]);
			}
			
			return builder().CreateCall(castCallee, newArgs);
		}
		
		llvm::InvokeInst*
		IREmitter::emitInvoke(llvm::FunctionType* const functionType,
		                      llvm::Value* const callee,
		                      llvm::BasicBlock* const normalDest,
		                      llvm::BasicBlock* const unwindDest,
		                      llvm::ArrayRef<llvm::Value*> args) {
			assert(callee->getType()->isPointerTy());
			const auto castCallee = functionGenerator_.getBuilder().CreatePointerCast(callee,
			                                                                          functionType->getPointerTo());
			
			// Cast all pointers to required types.
			llvm::SmallVector<llvm::Value*, 10> newArgs;
			newArgs.reserve(args.size());
			for (size_t i = 0; i < functionType->getNumParams(); i++) {
				const auto& arg = args[i];
				if (arg->getType()->isPointerTy()) {
					assert(functionType->getParamType(i)->isPointerTy());
					newArgs.push_back(builder().CreatePointerCast(arg,
					                                              functionType->getParamType(i)));
				} else {
					assert(!functionType->getParamType(i)->isPointerTy());
					newArgs.push_back(arg);
				}
			}
			
			for (size_t i = functionType->getNumParams(); i < args.size(); i++) {
				newArgs.push_back(args[i]);
			}
			
			return builder().CreateInvoke(castCallee,
			                              normalDest,
			                              unwindDest,
			                              newArgs);
		}
		
		llvm::ReturnInst*
		IREmitter::emitReturn(llvm::Type* const type,
		                      llvm::Value* const value) {
			if (value->getType()->isPointerTy()) {
				assert(type->isPointerTy());
				return builder().CreateRet(builder().CreatePointerCast(value, type));
			} else {
				assert(!type->isPointerTy());
				return builder().CreateRet(value);
			}
		}
		
		llvm::ReturnInst*
		IREmitter::emitReturnVoid() {
			return builder().CreateRetVoid();
		}
		
		llvm::LandingPadInst*
		IREmitter::emitLandingPad(llvm::StructType* const type,
		                          const unsigned numClauses) {
			assert(functionGenerator_.personalityFunction() != nullptr);
#if LOCIC_LLVM_VERSION >= 307
			return functionGenerator_.getBuilder().CreateLandingPad(type,
			                                                        numClauses);
#else
			return functionGenerator_.getBuilder().CreateLandingPad(type,
			                                                        functionGenerator_.personalityFunction(),
			                                                        numClauses);
#endif
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
		
		// TODO: move this inline.
		void genMoveStore(Function& function, llvm::Value* value, llvm::Value* var, const SEM::Type* type);
		
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
			return emitRawLoad(datatypePtr,
			                   TypeGenerator(module()).getI8Type());
		}
		
		void
		IREmitter::emitStoreDatatypeTag(llvm::Value* const tagValue,
		                                llvm::Value* const datatypePtr) {
			assert(tagValue->getType()->isIntegerTy(8));
			assert(datatypePtr->getType()->isPointerTy());
			emitRawStore(tagValue, datatypePtr);
		}
		
		llvm::Value*
		IREmitter::emitGetDatatypeVariantPtr(llvm::Value* const datatypePtr,
		                                     const SEM::Type* const datatypeType,
		                                     const SEM::Type* const variantType) {
			assert(datatypePtr->getType()->isPointerTy());
			assert(datatypeType->isUnionDatatype());
			assert(variantType->isDatatype());
			
			llvm::Value* datatypeVariantPtr = nullptr;
			
			// Try to use a plain GEP if possible.
			TypeInfo typeInfo(module());
			if (typeInfo.isSizeKnownInThisModule(datatypeType)) {
				datatypeVariantPtr = emitConstInBoundsGEP2_32(genType(module(), datatypeType),
				                                              datatypePtr,
				                                              0, 1);
			} else {
				const auto unionAlignValue = genAlignOf(functionGenerator_,
				                                        datatypeType);
				datatypeVariantPtr = emitInBoundsGEP(typeGenerator().getI8Type(),
				                                     datatypePtr,
				                                     unionAlignValue);
			}
			
			return datatypeVariantPtr;
		}
		
		void
		IREmitter::emitDestructorCall(llvm::Value* const value,
		                              const SEM::Type* const type) {
			if (type->isObject()) {
				TypeInfo typeInfo(module());
				if (!typeInfo.hasCustomDestructor(type)) {
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
				
				llvm::SmallVector<llvm::Value*, 2> args;
				if (!type->templateArguments().empty()) {
					args.push_back(getTemplateGenerator(functionGenerator_, TemplateInst::Type(type)));
				}
				args.push_back(value);
				
				(void) genRawFunctionCall(functionGenerator_, argInfo, destructorFunction, args);
			} else if (type->isTemplateVar()) {
				const auto typeInfo = functionGenerator_.getEntryBuilder().CreateExtractValue(functionGenerator_.getTemplateArgs(), { (unsigned int) type->getTemplateVar()->index() });
				module().virtualCallABI().emitDestructorCall(*this,
				                                             typeInfo,
				                                             value);
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
