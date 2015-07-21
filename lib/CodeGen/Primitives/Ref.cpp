#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Support.hpp>

#include <locic/SEM/TemplateVar.hpp>
#include <locic/SEM/Type.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		bool isVirtualnessKnown(const SEM::Type* const type) {
			// Virtual template variables may or may not be
			// instantiated with virtual types.
			return !type->isTemplateVar() ||
				!type->getTemplateVar()->isVirtual();
		}
		
		const SEM::Type* getRefTarget(const SEM::Type* const type) {
			const auto refTarget = type->templateArguments().at(0).typeRefType();
			assert(!refTarget->isAlias());
			return refTarget;
		}
		
		bool isRefVirtualnessKnown(const SEM::Type* const type) {
			return isVirtualnessKnown(getRefTarget(type));
		}
		
		bool isRefVirtual(const SEM::Type* const type) {
			assert(isRefVirtualnessKnown(type));
			return getRefTarget(type)->isInterface();
		}
		
		llvm::Type* getVirtualRefLLVMType(Module& module) {
			return interfaceStructType(module).second;
		}
		
		llvm::Type* getNotVirtualLLVMType(Module& module, const SEM::Type* const type) {
			return genPointerType(module, getRefTarget(type));
		}
		
		llvm::Type* getRefLLVMType(Module& module, const SEM::Type* const type) {
			return isRefVirtual(type) ?
				getVirtualRefLLVMType(module) :
				getNotVirtualLLVMType(module, type);
		}
		
		llvm::Value* fixRefType(Function& function, llvm::Value* const value, llvm::Type* const type, llvm::Type* const fixType) {
			if (value->getType()->isPointerTy() && value->getType()->getPointerElementType() == type) {
				return function.getBuilder().CreatePointerCast(value, fixType->getPointerTo());
			} else {
				return value;
			}
		}
		
		template <typename Fn>
		llvm::Value* genRefPrimitiveMethodForVirtualCases(Function& function, const SEM::Type* const type, Fn f) {
			auto& module = function.module();
			
			if (isRefVirtualnessKnown(type)) {
				// If the reference target type is not a virtual template variable,
				// we already know whether it's a simple pointer or a 'fat'
				// pointer (i.e. a struct containing a pointer to the object
				// as well as the vtable and the template generator), so we
				// only generate for this known case.
				return f(getRefLLVMType(module, type));
			}
			
			// If the reference target type is a template variable, we need
			// to query the virtual-ness of it at run-time and hence we must
			// emit code to handle both cases.
			
			const auto refTarget = getRefTarget(type);
			
			// Look at our template argument to see if it's virtual.
			const auto argTypeInfo = function.getBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) refTarget->getTemplateVar()->index() });
			const auto argVTablePointer = function.getBuilder().CreateExtractValue(argTypeInfo, { 0 });
			
			// If the VTable pointer is NULL, it's a virtual type, which
			// means we are larger (to store the type information etc.).
			const auto nullVtablePtr = ConstantGenerator(module).getNullPointer(vtableType(module)->getPointerTo());
			const auto isVirtualCondition = function.getBuilder().CreateICmpEQ(argVTablePointer, nullVtablePtr, "isVirtual");
			
			const auto ifVirtualBlock = function.createBasicBlock("ifRefVirtual");
			const auto ifNotVirtualBlock = function.createBasicBlock("ifRefNotVirtual");
			const auto mergeBlock = function.createBasicBlock("mergeRefVirtual");
			
			function.getBuilder().CreateCondBr(isVirtualCondition, ifVirtualBlock, ifNotVirtualBlock);
			
			function.selectBasicBlock(ifVirtualBlock);
			const auto virtualType = getVirtualRefLLVMType(module);
			const auto virtualResult = fixRefType(function, f(virtualType), virtualType, genType(module, type));
			function.getBuilder().CreateBr(mergeBlock);
			
			function.selectBasicBlock(ifNotVirtualBlock);
			const auto notVirtualType = getNotVirtualLLVMType(module, type);
			const auto notVirtualResult = fixRefType(function, f(notVirtualType), notVirtualType, genType(module, type));
			function.getBuilder().CreateBr(mergeBlock);
			
			function.selectBasicBlock(mergeBlock);
			
			if (!virtualResult->getType()->isVoidTy()) {
				assert(!notVirtualResult->getType()->isVoidTy());
				if (virtualResult == notVirtualResult) {
					return virtualResult;
				} else {
					const auto phiNode = function.getBuilder().CreatePHI(virtualResult->getType(), 2);
					phiNode->addIncoming(virtualResult, ifVirtualBlock);
					phiNode->addIncoming(notVirtualResult, ifNotVirtualBlock);
					return phiNode;
				}
			} else {
				assert(notVirtualResult->getType()->isVoidTy());
				return ConstantGenerator(module).getVoidUndef();
			}
		}
		
		class RefMethodOwner {
		public:
			static RefMethodOwner AsRef(Function& function, const SEM::Type* const type, PendingResultArray& args) {
				return RefMethodOwner(function, type, args, false);
			}
			
			static RefMethodOwner AsValue(Function& function, const SEM::Type* const type, PendingResultArray& args) {
				return RefMethodOwner(function, type, args, true);
			}
			
		private:
			RefMethodOwner(Function& function, const SEM::Type* const type, PendingResultArray& args, const bool loaded)
			: function_(function), type_(type), args_(args), loaded_(loaded), value_(nullptr) {
				if (loaded_) {
					// If the virtual-ness of the reference is known
					// then we can load the reference-to-reference
					// here, otherwise we need to wait until the
					// two cases are evaluated.
					if (isRefVirtualnessKnown(type_)) {
						value_ = args[0].resolveWithoutBind(function);
					} else {
						value_ = args[0].resolve(function);
					}
				} else {
					value_ = args[0].resolve(function);
				}
			}
			
		public:
			llvm::Value* get(llvm::Type* const llvmType) {
				if (loaded_) {
					// If the virtual-ness of the reference is known
					// then the reference is already loaded, otherwise it
					// needs to be loaded here.
					if (isRefVirtualnessKnown(type_)) {
						return value_;
					} else {
						auto& builder = function_.getBuilder();
						const auto pointerType = llvmType->getPointerTo();
						const auto result = builder.CreatePointerCast(value_, pointerType);
						return builder.CreateLoad(result);
					}
				} else {
					auto& builder = function_.getBuilder();
					const auto pointerType = llvmType->getPointerTo();
					return builder.CreatePointerCast(value_, pointerType);
				}
			}
			
		private:
			Function& function_;
			const SEM::Type* const type_;
			PendingResultArray& args_;
			bool loaded_;
			llvm::Value* value_;
			
		};
		
		llvm::Value* genRefPrimitiveMethodCall(Function& function,
		                                       const SEM::Type* const type,
		                                       const String& methodName,
		                                       SEM::FunctionType /*functionType*/,
		                                       PendingResultArray args,
		                                       llvm::Value* const hintResultValue) {
			auto& builder = function.getBuilder();
			auto& module = function.module();
			
			const auto methodID = module.context().getMethodID(CanonicalizeMethodName(methodName));
			
			switch (methodID) {
				case METHOD_ALIGNMASK: {
					return genRefPrimitiveMethodForVirtualCases(function, type,
						[&](llvm::Type* const llvmType) {
							if (llvmType->isPointerTy()) {
								const auto nonVirtualAlign = module.abi().typeAlign(llvm_abi::Type::Pointer(module.abiContext()));
								return ConstantGenerator(module).getSizeTValue(nonVirtualAlign - 1);
							} else {
								const auto virtualAlign = module.abi().typeAlign(interfaceStructType(module).first);
								return ConstantGenerator(module).getSizeTValue(virtualAlign - 1);
							}
						}
					);
				}
				case METHOD_SIZEOF: {
					return genRefPrimitiveMethodForVirtualCases(function, type,
						[&](llvm::Type* const llvmType) {
							if (llvmType->isPointerTy()) {
								const auto nonVirtualSize = module.abi().typeSize(llvm_abi::Type::Pointer(module.abiContext()));
								return ConstantGenerator(module).getSizeTValue(nonVirtualSize);
							} else {
								const auto virtualSize = module.abi().typeSize(interfaceStructType(module).first);
								return ConstantGenerator(module).getSizeTValue(virtualSize);
							}
						}
					);
				}
				case METHOD_COPY:
				case METHOD_IMPLICITCOPY: {
					// If the virtualness of the reference is known, we
					// can load it, otherwise we have to keep accessing
					// it by pointer.
					if (!isRefVirtualnessKnown(type) && hintResultValue == nullptr) {
						return args[0].resolve(function);
					}
					
					auto methodOwner = RefMethodOwner::AsValue(function, type, args);
					
					return genRefPrimitiveMethodForVirtualCases(function, type,
						[&](llvm::Type* const llvmType) {
							const auto loadedValue = methodOwner.get(llvmType);
							if (!isRefVirtualnessKnown(type)) {
								assert(hintResultValue != nullptr);
								const auto castPtr = builder.CreatePointerCast(hintResultValue, llvmType->getPointerTo());
								builder.CreateStore(loadedValue, castPtr);
								return hintResultValue;
							} else {
								return loadedValue;
							}
						});
				}
				case METHOD_ISVALID: {
					auto methodOwner = RefMethodOwner::AsValue(function, type, args);
					
					return genRefPrimitiveMethodForVirtualCases(function, type,
						[&](llvm::Type* const llvmType) {
							const auto methodOwnerValue = methodOwner.get(llvmType);
							if (llvmType->isPointerTy()) {
								const auto nullValue = ConstantGenerator(module).getNull(llvmType);
								return builder.CreateICmpNE(methodOwnerValue, nullValue);
							} else {
								const auto pointerValue = builder.CreateExtractValue(methodOwnerValue, { 0 });
								const auto nullValue = ConstantGenerator(module).getNull(pointerValue->getType());
								return builder.CreateICmpNE(pointerValue, nullValue);
							}
						});
				}
				case METHOD_SETINVALID: {
					auto methodOwner = RefMethodOwner::AsRef(function, type, args);
					
					return genRefPrimitiveMethodForVirtualCases(function, type,
						[&](llvm::Type* const llvmType) {
							const auto methodOwnerPtr = methodOwner.get(llvmType);
							const auto nullValue = ConstantGenerator(module).getNull(llvmType);
							builder.CreateStore(nullValue, methodOwnerPtr);
							return ConstantGenerator(module).getVoidUndef();
						});
				}
				case METHOD_ISLIVE: {
					return ConstantGenerator(module).getI1(true);
				}
				case METHOD_SETDEAD: {
					// Do nothing.
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_MOVETO: {
					auto methodOwner = RefMethodOwner::AsValue(function, type, args);
					const auto moveToPtr = args[1].resolve(function);
					const auto moveToPosition = args[2].resolve(function);
					const auto destPtr = builder.CreateInBoundsGEP(moveToPtr, moveToPosition);
					
					return genRefPrimitiveMethodForVirtualCases(function, type,
						[&](llvm::Type* const llvmType) {
							const auto castedDestPtr = builder.CreatePointerCast(destPtr, llvmType->getPointerTo());
							builder.CreateStore(methodOwner.get(llvmType), castedDestPtr);
							return ConstantGenerator(module).getVoidUndef();
						});
				}
				default:
					printf("%s\n", methodName.c_str());
					llvm_unreachable("Unknown ref primitive method.");
			}
		}
		
	}
	
}

