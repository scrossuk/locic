#include <locic/AST/Context.hpp>
#include <locic/AST/Function.hpp>
#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/TemplateVar.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/MethodInfo.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/CodeGen/VirtualCallABI.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		IREmitter::IREmitter(Function& functionGenerator)
		: functionGenerator_(functionGenerator) { }
		
		ConstantGenerator
		IREmitter::constantGenerator() {
			return ConstantGenerator(module());
		}
		
		TypeGenerator
		IREmitter::typeGenerator() {
			return TypeGenerator(module());
		}
		
		llvm::BasicBlock*
		IREmitter::createBasicBlock(const char* name) {
			return functionGenerator_.createBasicBlock(name);
		}
		
		bool
		IREmitter::lastInstructionTerminates() const {
			return functionGenerator_.lastInstructionTerminates();
		}
		
		llvm::BasicBlock*
		IREmitter::getBasicBlock() {
			return functionGenerator_.getBuilder().GetInsertBlock();
		}
		
		void
		IREmitter::selectBasicBlock(llvm::BasicBlock* basicBlock) {
			functionGenerator_.selectBasicBlock(basicBlock);
		}
		
		void
		IREmitter::emitBranch(llvm::BasicBlock* basicBlock) {
			functionGenerator_.getBuilder().CreateBr(basicBlock);
		}
		
		void
		IREmitter::emitCondBranch(llvm::Value* condition,
		                          llvm::BasicBlock* ifTrue,
		                          llvm::BasicBlock* ifFalse) {
			functionGenerator_.getBuilder().CreateCondBr(condition,
			                                             ifTrue,
			                                             ifFalse);
		}
		
		void
		IREmitter::emitUnreachable() {
			functionGenerator_.getBuilder().CreateUnreachable();
		}
		
		llvm::Value*
		IREmitter::getUndef(const llvm_abi::Type type) {
			const auto llvmType = module().getLLVMType(type);
			return constantGenerator().getUndef(llvmType);
		}
		
		llvm::Value*
		IREmitter::emitPointerCast(llvm::Value* const ptr,
		                           llvm::Type* const type) {
			assert(ptr->getType()->isPointerTy());
			assert(type->isPointerTy());
			return functionGenerator_.getBuilder().CreatePointerCast(ptr, type);
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
			const auto castVar = emitPointerCast(valuePtr, type->getPointerTo());
			return functionGenerator_.getBuilder().CreateLoad(castVar);
		}
		
		void
		IREmitter::emitRawStore(llvm::Value* const value,
		                        llvm::Value* const var) {
			assert(var->getType()->isPointerTy());
			const auto castVar = emitPointerCast(var, value->getType()->getPointerTo());
			(void) functionGenerator_.getBuilder().CreateStore(value,
			                                                   castVar);
		}
		
		llvm::Value*
		IREmitter::emitInBoundsGEP(llvm::Type* const type,
		                           llvm::Value* const ptrValue,
		                           llvm::Value* const indexValue) {
			assert(ptrValue->getType()->isPointerTy());
			assert(indexValue->getType()->isIntegerTy());
			const auto castValue = emitPointerCast(ptrValue, type->getPointerTo());
			return functionGenerator_.getBuilder().CreateInBoundsGEP(castValue,
			                                                         indexValue);
		}
		
		llvm::Value*
		IREmitter::emitInBoundsGEP(llvm::Type* const type,
		                           llvm::Value* const ptrValue,
		                           llvm::ArrayRef<llvm::Value*> indexArray) {
			assert(ptrValue->getType()->isPointerTy());
			const auto castValue = emitPointerCast(ptrValue, type->getPointerTo());
			return functionGenerator_.getBuilder().CreateInBoundsGEP(castValue,
			                                                         indexArray);
		}
		
		llvm::Value*
		IREmitter::emitConstInBoundsGEP2_32(llvm::Type* const type,
		                                    llvm::Value* const ptrValue,
		                                    const unsigned index0,
		                                    const unsigned index1) {
			assert(ptrValue->getType()->isPointerTy());
			const auto castValue = emitPointerCast(ptrValue, type->getPointerTo());
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
				const auto castValue = emitPointerCast(value, indexType);
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
			const auto castPtr = emitPointerCast(ptr, typeGenerator().getPtrType());
			builder().CreateMemSet(castPtr, value, size, align);
		}
		
		void
		IREmitter::emitMemSet(llvm::Value* const ptr,
		                      llvm::Value* const value,
		                      llvm::Value* const sizeValue,
		                      const unsigned align) {
			assert(ptr->getType()->isPointerTy());
			const auto castPtr = emitPointerCast(ptr, typeGenerator().getPtrType());
			builder().CreateMemSet(castPtr, value, sizeValue, align);
		}
		
		void
		IREmitter::emitMemCpy(llvm::Value* const dest,
		                      llvm::Value* const src,
		                      const uint64_t size,
		                      const unsigned align) {
			assert(dest->getType()->isPointerTy());
			assert(src->getType()->isPointerTy());
			const auto castDest = emitPointerCast(dest, typeGenerator().getPtrType());
			const auto castSrc = emitPointerCast(src, typeGenerator().getPtrType());
			builder().CreateMemCpy(castDest, castSrc, size, align);
		}
		
		void
		IREmitter::emitMemCpy(llvm::Value* const dest,
		                      llvm::Value* const src,
		                      llvm::Value* const sizeValue,
		                      const unsigned align) {
			assert(dest->getType()->isPointerTy());
			assert(src->getType()->isPointerTy());
			const auto castDest = emitPointerCast(dest, typeGenerator().getPtrType());
			const auto castSrc = emitPointerCast(src, typeGenerator().getPtrType());
			builder().CreateMemCpy(castDest, castSrc, sizeValue, align);
		}
		
		llvm::CallInst*
		IREmitter::emitCall(llvm::FunctionType* const functionType,
		                    llvm::Value* const callee,
		                    llvm::ArrayRef<llvm::Value*> args) {
			assert(callee->getType()->isPointerTy());
			const auto castCallee = emitPointerCast(callee, functionType->getPointerTo());
			
			// Cast all pointers to required types.
			llvm::SmallVector<llvm::Value*, 10> newArgs;
			newArgs.reserve(args.size());
			for (size_t i = 0; i < functionType->getNumParams(); i++) {
				const auto& arg = args[i];
				if (arg->getType()->isPointerTy()) {
					assert(functionType->getParamType(i)->isPointerTy());
					newArgs.push_back(emitPointerCast(arg, functionType->getParamType(i)));
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
			const auto castCallee = emitPointerCast(callee, functionType->getPointerTo());
			
			// Cast all pointers to required types.
			llvm::SmallVector<llvm::Value*, 10> newArgs;
			newArgs.reserve(args.size());
			for (size_t i = 0; i < functionType->getNumParams(); i++) {
				const auto& arg = args[i];
				if (arg->getType()->isPointerTy()) {
					assert(functionType->getParamType(i)->isPointerTy());
					newArgs.push_back(emitPointerCast(arg, functionType->getParamType(i)));
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
				return builder().CreateRet(emitPointerCast(value, type));
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
		IREmitter::emitAlignMask(const AST::Type* const type) {
			return genAlignMask(functionGenerator_, type);
		}
		
		llvm::Value*
		IREmitter::emitSizeOf(const AST::Type* const type) {
			return genSizeOf(functionGenerator_, type);
		}
		
		static llvm::Value*
		genRawAlloca(IREmitter& irEmitter, const AST::Type* const type, llvm::Value* const resultPtr) {
			if (resultPtr != nullptr) {
				assert(resultPtr->getType()->isPointerTy());
				return resultPtr;
			}
			
			auto& function = irEmitter.function();
			auto& module = function.module();
			
			SetUseEntryBuilder setUseEntryBuilder(function);
			
			TypeInfo typeInfo(module);
			if (typeInfo.isSizeKnownInThisModule(type)) {
				const auto llvmType = genType(module, type);
				assert(!llvmType->isVoidTy());
				return function.getBuilder().CreateAlloca(llvmType);
			} else {
				return function.getEntryBuilder().CreateAlloca(
						TypeGenerator(module).getI8Type(),
						irEmitter.emitSizeOf(type));
			}
		}
		
		static llvm::Value*
		genAlloca(IREmitter& irEmitter, const AST::Type* const type, llvm::Value* const resultPtr) {
			auto& module = irEmitter.module();
			const bool shouldZeroAlloca = module.buildOptions().zeroAllAllocas;
			
			const auto allocaValue = genRawAlloca(irEmitter,
			                                      type,
			                                      resultPtr);
			
			if (shouldZeroAlloca && resultPtr == nullptr) {
				const auto typeSizeValue = irEmitter.emitSizeOf(type);
				irEmitter.emitMemSet(allocaValue,
				                     ConstantGenerator(module).getI8(0),
				                     typeSizeValue,
				                     /*align=*/1);
			}
			
			return allocaValue;
		}
		
		llvm::Value*
		IREmitter::emitAlloca(const AST::Type* const type,
		                      llvm::Value* const resultPtr) {
			return genAlloca(*this, type, resultPtr);
		}
		
		llvm::Value*
		IREmitter::emitBind(llvm::Value* const value,
		                    const AST::Type* const type) {
			if (TypeInfo(module()).isPassedByValue(type)) {
				const auto ptr = emitAlloca(type);
				emitRawStore(value, ptr);
				return ptr;
			} else {
				assert(value->getType()->isPointerTy());
				return value;
			}
		}
		
		llvm::Value*
		IREmitter::emitLoad(llvm::Value* const ptr,
		                    const AST::Type* const type) {
			assert(ptr->getType()->isPointerTy());
			if (TypeInfo(module()).isPassedByValue(type)) {
				const auto llvmType = genType(module(), type);
				return emitRawLoad(ptr, llvmType);
			} else {
				return ptr;
			}
		}
		
		void
		IREmitter::emitStore(llvm::Value* const value,
		                     llvm::Value* const ptr,
		                     const AST::Type* const type) {
			assert(ptr->getType()->isPointerTy());
			if (TypeInfo(module()).isPassedByValue(type)) {
				emitRawStore(value, ptr);
			} else {
				assert(value->getType()->isPointerTy());
				assert(value->stripPointerCasts() == ptr->stripPointerCasts());
			}
		}
		
		void
		IREmitter::emitMove(llvm::Value* const sourcePtr,
		                    llvm::Value* const destPtr,
		                    const AST::Type* type) {
			assert(sourcePtr->getType()->isPointerTy());
			assert(destPtr->getType()->isPointerTy());
			assert(sourcePtr->stripPointerCasts() != destPtr->stripPointerCasts());
			
			RefPendingResult thisPendingResult(sourcePtr, type);
			const auto result = emitMoveCall(thisPendingResult, type, destPtr);
			emitStore(result, destPtr, type);
		}
		
		void
		IREmitter::emitMoveStore(llvm::Value* const value,
		                         llvm::Value* const ptr,
		                         const AST::Type* const type) {
			assert(ptr->getType()->isPointerTy());
			if (TypeInfo(module()).isPassedByValue(type)) {
				emitRawStore(value, ptr);
			} else {
				emitMove(value, ptr, type);
			}
		}
		
		AST::FunctionType moveFunctionType(const AST::Type* const type) {
			const bool hasTemplateArgs = type->isObject() && !type->templateArguments().empty();
			AST::FunctionAttributes attributes(/*isVarArg=*/false, /*isMethod=*/true,
			                                   /*isTemplated=*/hasTemplateArgs,
			                                   /*noexceptPredicate=*/AST::Predicate::True());
			return AST::FunctionType(std::move(attributes), type, {});
		}
		
		llvm::Value*
		IREmitter::emitMoveCall(PendingResult value,
		                        const AST::Type* const rawType,
		                        llvm::Value* const resultPtr) {
			const auto type = rawType->resolveAliases();
			const auto functionType = moveFunctionType(type);
			MethodInfo methodInfo(type, module().getCString("__move"),
			                      functionType, /*templateArgs=*/{});
			return genDynamicMethodCall(function(), methodInfo,
			                            std::move(value), /*args=*/{},
			                            resultPtr);
		}
		
		llvm::Value*
		IREmitter::emitInnerMoveCall(llvm::Value* const value,
		                             const AST::Type* const rawType,
		                             llvm::Value* const resultPtr) {
			const auto type = rawType->resolveAliases();
			assert(type->isObject());
			
			const auto& function = type->getObjectType()->getFunction(module().getCString("__move"));
			
			auto& astFunctionGenerator = module().astFunctionGenerator();
			const auto moveFunction = astFunctionGenerator.genDef(type->getObjectType(),
			                                                      function,
			                                                      /*isInnerMethod=*/true);
			
			FunctionCallInfo callInfo;
			callInfo.functionPtr = moveFunction;
			callInfo.contextPointer = value;
			
			// We're assuming that this call is being made from the **outer** move function,
			// so the template generator will be identical.
			callInfo.templateGenerator = functionGenerator_.getTemplateGeneratorOrNull();
			
			const auto functionType = moveFunctionType(type);
			return genFunctionCall(functionGenerator_, functionType, callInfo,
			                       /*args=*/{}, resultPtr);
		}
		
		llvm::Value*
		IREmitter::emitLoadVariantTag(llvm::Value* const variantPtr) {
			return emitRawLoad(variantPtr,
			                   TypeGenerator(module()).getI8Type());
		}
		
		void
		IREmitter::emitStoreVariantTag(llvm::Value* const tagValue,
		                                llvm::Value* const variantPtr) {
			assert(tagValue->getType()->isIntegerTy(8));
			assert(variantPtr->getType()->isPointerTy());
			emitRawStore(tagValue, variantPtr);
		}
		
		llvm::Value*
		IREmitter::emitGetVariantValuePtr(llvm::Value* const variantPtr,
		                                  const AST::Type* const variantType) {
			assert(variantPtr->getType()->isPointerTy());
			assert(variantType->isVariant());
			
			llvm::Value* valuePtr;
			
			// Try to use a plain GEP if possible.
			TypeInfo typeInfo(module());
			if (typeInfo.isSizeKnownInThisModule(variantType)) {
				valuePtr = emitConstInBoundsGEP2_32(genType(module(), variantType),
				                                    variantPtr, 0, 1);
			} else {
				const auto unionAlignValue = genAlignOf(functionGenerator_,
				                                        variantType);
				valuePtr = emitInBoundsGEP(typeGenerator().getI8Type(),
				                           variantPtr, unionAlignValue);
			}
			
			return valuePtr;
		}
		
		llvm::Value*
		IREmitter::emitConstructorCall(const AST::Type* const rawType,
		                               PendingResultArray args,
		                               llvm::Value* const resultPtr) {
			const auto type = rawType->resolveAliases();
			assert(type->isObject() && "Doesn't currently support template vars.");
			
			const auto name = module().getCString("create");
			const auto functionType = type->getObjectType()->getFunction(name).type();
			
			assert(functionType.returnType()->substitute(type->generateTemplateVarMap()) == type);
			assert(functionType.parameterTypes().size() == args.size());
			assert(!functionType.attributes().isMethod());
			
			const bool isTemplated = !type->templateArguments().empty();
			assert(functionType.attributes().isTemplated() == isTemplated);
			
			MethodInfo methodInfo(type, name, functionType, /*templateArgs=*/{});
			return genStaticMethodCall(functionGenerator_, methodInfo,
			                           std::move(args), resultPtr);
		}
		
		AST::FunctionType
		destroyFunctionType(Module& module, const AST::Type* const type) {
			const auto& voidTypeInstance = module.context().astContext().getPrimitive(PrimitiveVoid);
			const auto voidType = AST::Type::Object(&voidTypeInstance, {});
			
			const bool hasTemplateArgs = type->isObject() && !type->templateArguments().empty();
			AST::FunctionAttributes attributes(/*isVarArg=*/false, /*isMethod=*/true,
			                                   /*isTemplated=*/hasTemplateArgs,
			                                   /*noexceptPredicate=*/AST::Predicate::True());
			return AST::FunctionType(std::move(attributes), voidType, {});
		}
		
		void
		IREmitter::emitDestructorCall(llvm::Value* const value,
		                              const AST::Type* const rawType) {
			const auto type = rawType->resolveAliases();
			
			TypeInfo typeInfo(module());
			if (!typeInfo.hasCustomDestructor(type)) {
				return;
			}
			
			assert(value->getType()->isPointerTy());
			
			const auto functionType = destroyFunctionType(module(), type);
			
			MethodInfo methodInfo(type, module().getCString("__destroy"),
			                      functionType, /*templateArgs=*/{});
			
			RefPendingResult thisPendingResult(value, type);
			const auto result = genDynamicMethodCall(function(), methodInfo,
			                                         thisPendingResult, /*args=*/{});
			assert(result->getType()->isVoidTy());
			(void) result;
		}
		
		void
		IREmitter::emitInnerDestructorCall(llvm::Value* const value,
		                                   const AST::Type* const rawType) {
			assert(value->getType()->isPointerTy());
			
			const auto type = rawType->resolveAliases();
			assert(type->isObject());
			
			const auto& function = type->getObjectType()->getFunction(module().getCString("__destroy"));
			
			auto& astFunctionGenerator = module().astFunctionGenerator();
			const auto destroyFunction = astFunctionGenerator.genDef(type->getObjectType(),
			                                                         function,
			                                                         /*isInnerMethod=*/true);
			
			FunctionCallInfo callInfo;
			callInfo.functionPtr = destroyFunction;
			callInfo.contextPointer = value;
			
			// We're assuming that this call is being made from the **outer** destructor,
			// so the template generator will be identical.
			callInfo.templateGenerator = functionGenerator_.getTemplateGeneratorOrNull();
			
			const auto functionType = destroyFunctionType(module(), type);
			const auto result = genFunctionCall(functionGenerator_, functionType, callInfo,
			                                    /*args=*/{}, /*resultPtr=*/nullptr);
			assert(result->getType()->isVoidTy());
			(void) result;
		}
		
		void
		IREmitter::scheduleDestructorCall(llvm::Value* const value,
		                                  const AST::Type* const type) {
			TypeInfo typeInfo(module());
			if (!typeInfo.hasCustomDestructor(type)) {
				return;
			}
			
			assert(value->getType()->isPointerTy());
			function().pushUnwindAction(UnwindAction::Destructor(type, value));
		}
		
		llvm::Value*
		IREmitter::emitImplicitCopyCall(llvm::Value* value,
		                                const AST::Type* type,
		                                llvm::Value* resultPtr) {
			return emitCopyCall(METHOD_IMPLICITCOPY,
			                    value,
			                    type,
			                    resultPtr);
		}
		
		llvm::Value*
		IREmitter::emitExplicitCopyCall(llvm::Value* value,
		                                const AST::Type* type,
		                                llvm::Value* resultPtr) {
			return emitCopyCall(METHOD_COPY,
			                    value,
			                    type,
			                    resultPtr);
		}
		
		llvm::Value*
		IREmitter::emitCopyCall(const MethodID methodID,
		                        llvm::Value* value,
		                        const AST::Type* rawType,
		                        llvm::Value* resultPtr) {
			assert(methodID == METHOD_IMPLICITCOPY ||
			       methodID == METHOD_COPY);
			
			const auto type = rawType->resolveAliases();
			
			const bool isTemplated = type->isObject() &&
			                         !type->templateArguments().empty();
			
			AST::FunctionAttributes attributes(/*isVarArg=*/false,
			                                   /*isMethod=*/true,
			                                   isTemplated,
			                                   /*noExceptPredicate=*/AST::Predicate::False());
			
			AST::FunctionType functionType(std::move(attributes),
			                               type,
			                               {});
			
			const auto methodName = module().getCString(methodID.toCString());
			
			MethodInfo methodInfo(type,
			                      methodName,
			                      functionType,
			                      {});
			
			RefPendingResult thisPendingResult(value, type);
			
			return genMethodCall(functionGenerator_,
			                     methodInfo,
			                     Optional<PendingResult>(thisPendingResult),
			                     /*args=*/{},
			                     resultPtr);
		}
		
		static const AST::Type*
		createRefType(const AST::Type* const refTargetType,
		              const AST::TypeInstance& refTypeInstance) {
			AST::ValueArray templateArguments;
			templateArguments.push_back(refTargetType->asValue());
			return AST::Type::Object(&refTypeInstance, std::move(templateArguments));
		}
		
		llvm::Value*
		IREmitter::emitCompareCall(llvm::Value* const leftValue,
		                           llvm::Value* const rightValue,
		                           const AST::Type* const rawType) {
			const auto type = rawType->resolveAliases();
			
			const bool isTemplated = type->isObject() &&
			                         !type->templateArguments().empty();
			
			AST::FunctionAttributes attributes(/*isVarArg=*/false,
			                                   /*isMethod=*/true,
			                                   isTemplated,
			                                   /*noExceptPredicate=*/AST::Predicate::False());
			
			const auto compareResultType = module().context().astContext().getPrimitive(PrimitiveCompareResult).selfType();
			const auto& refTypeInstance = module().context().astContext().getPrimitive(PrimitiveRef);
			const auto thisRefType = createRefType(type, refTypeInstance);
			
			AST::FunctionType functionType(std::move(attributes),
			                               compareResultType,
			                               { thisRefType });
			
			MethodInfo methodInfo(type,
			                      module().getCString("compare"),
			                      functionType,
			                      {});
			
			RefPendingResult leftValuePendingResult(leftValue, type);
			RefPendingResult rightValuePendingResult(rightValue, type);
			
			return genMethodCall(functionGenerator_,
			                     methodInfo,
			                     Optional<PendingResult>(leftValuePendingResult),
			                     /*args=*/{ rightValuePendingResult });
		}
		
		llvm::Value*
		IREmitter::emitComparisonCall(const MethodID methodID,
		                              PendingResult leftValue,
		                              PendingResult rightValue,
		                              const AST::Type* const rawType) {
			const auto type = rawType->resolveAliases();
			
			const bool isTemplated = type->isObject() &&
			                         !type->templateArguments().empty();
			
			AST::FunctionAttributes attributes(/*isVarArg=*/false,
			                                   /*isMethod=*/true,
			                                   isTemplated,
			                                   /*noExceptPredicate=*/AST::Predicate::False());
			
			const auto boolType = module().context().astContext().getPrimitive(PrimitiveBool).selfType();
			const auto& refTypeInstance = module().context().astContext().getPrimitive(PrimitiveRef);
			const auto thisRefType = createRefType(type, refTypeInstance);
			
			AST::FunctionType functionType(std::move(attributes),
			                               boolType,
			                               { thisRefType });
			
			MethodInfo methodInfo(type,
			                      module().getCString(methodID.toCString()),
			                      functionType,
			                      {});
			
			return genMethodCall(functionGenerator_,
			                     methodInfo,
			                     Optional<PendingResult>(std::move(leftValue)),
			                     /*args=*/{ std::move(rightValue) });
		}
		
		llvm::Value*
		IREmitter::emitNoArgNoReturnCall(const MethodID methodID,
		                                 llvm::Value* const value,
		                                 const AST::Type* const rawType) {
			const auto type = rawType->resolveAliases();
			
			const bool isTemplated = type->isObject() &&
			                         !type->templateArguments().empty();
			
			AST::FunctionAttributes attributes(/*isVarArg=*/false,
			                                   /*isMethod=*/true,
			                                   isTemplated,
			                                   /*noExceptPredicate=*/AST::Predicate::False());
			
			const auto voidType = module().context().astContext().getPrimitive(PrimitiveVoid).selfType();
			
			AST::FunctionType functionType(std::move(attributes),
			                               voidType,
			                               {});
			
			MethodInfo methodInfo(type,
			                      module().getCString(methodID.toCString()),
			                      functionType,
			                      {});
			
			RefPendingResult objectPendingResult(value, type);
			
			return genMethodCall(functionGenerator_,
			                     methodInfo,
			                     Optional<PendingResult>(objectPendingResult),
			                     /*args=*/{});
		}
		
		llvm::Value*
		IREmitter::emitIsEmptyCall(llvm::Value* const value,
		                           const AST::Type* const rawType) {
			const auto type = rawType->resolveAliases();
			
			const bool isTemplated = type->isObject() &&
			                         !type->templateArguments().empty();
			
			AST::FunctionAttributes attributes(/*isVarArg=*/false,
			                                   /*isMethod=*/true,
			                                   isTemplated,
			                                   /*noExceptPredicate=*/AST::Predicate::False());
			
			const auto boolType = module().context().astContext().getPrimitive(PrimitiveBool).selfType();
			
			AST::FunctionType functionType(std::move(attributes),
			                               boolType, {});
			
			MethodInfo methodInfo(type, module().getCString("empty"),
			                      functionType, {});
			
			RefPendingResult objectPendingResult(value, type);
			
			return genMethodCall(functionGenerator_, methodInfo,
			                     Optional<PendingResult>(objectPendingResult),
			                     /*args=*/{});
		}
		
		llvm::Value*
		IREmitter::emitFrontCall(llvm::Value* const value,
		                         const AST::Type* const rawType,
		                         const AST::Type* const rawResultType,
		                         llvm::Value* const resultPtr) {
			const auto type = rawType->resolveAliases();
			const auto resultType = rawResultType->resolveAliases();
			
			const bool isTemplated = type->isObject() &&
			                         !type->templateArguments().empty();
			
			AST::FunctionAttributes attributes(/*isVarArg=*/false,
			                                   /*isMethod=*/true,
			                                   isTemplated,
			                                   /*noExceptPredicate=*/AST::Predicate::False());
			
			AST::FunctionType functionType(std::move(attributes),
			                               resultType, {});
			
			MethodInfo methodInfo(type, module().getCString("front"),
			                      functionType, {});
			
			RefPendingResult objectPendingResult(value, type);
			
			return genMethodCall(functionGenerator_, methodInfo,
			                     Optional<PendingResult>(objectPendingResult),
			                     /*args=*/{}, resultPtr);
		}
		
		void
		IREmitter::emitSkipFrontCall(llvm::Value* const value,
		                             const AST::Type* const rawType) {
			const auto type = rawType->resolveAliases();
			
			const bool isTemplated = type->isObject() &&
			                         !type->templateArguments().empty();
			
			AST::FunctionAttributes attributes(/*isVarArg=*/false,
			                                   /*isMethod=*/true,
			                                   isTemplated,
			                                   /*noExceptPredicate=*/AST::Predicate::False());
			
			const auto voidType = module().context().astContext().getPrimitive(PrimitiveVoid).selfType();
			
			AST::FunctionType functionType(std::move(attributes),
			                               voidType, {});
			
			MethodInfo methodInfo(type, module().getCString("skipfront"),
			                      functionType, {});
			
			RefPendingResult objectPendingResult(value, type);
			
			(void) genMethodCall(functionGenerator_, methodInfo,
			                     Optional<PendingResult>(objectPendingResult),
			                     /*args=*/{});
		}
		
		llvm::IRBuilder<>& IREmitter::builder() {
			return functionGenerator_.getBuilder();
		}
		
		Function& IREmitter::function() {
			return functionGenerator_;
		}
		
		Module& IREmitter::module() {
			return functionGenerator_.module();
		}
		
	}
	
}
