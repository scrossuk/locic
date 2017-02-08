#include <assert.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/TemplateVar.hpp>
#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/InternalContext.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Primitives/RefPrimitive.hpp>
#include <locic/CodeGen/Routines.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/VTable.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		RefPrimitive::RefPrimitive(const AST::TypeInstance& typeInstance)
		: typeInstance_(typeInstance) { }
		
		bool RefPrimitive::isAbstractnessKnown(const AST::Type* const targetType) const {
			if (!targetType->isTemplateVar()) {
				// We know statically whether the type is abstract.
				return true;
			}
			
			// We're assuming that all typename arguments to templated functions/types
			// other than ref_t are known to NOT be abstract.
			return targetType->getTemplateVar() != typeInstance_.templateVariables().front();
		}
		
		bool RefPrimitive::isAbstract(const AST::Type* const targetType) const {
			assert(isAbstractnessKnown(targetType));
			return targetType->isInterface();
		}
		
		bool RefPrimitive::isSizeAlwaysKnown(const TypeInfo& /*typeInfo*/,
		                                     llvm::ArrayRef<AST::Value> templateArguments) const {
			return isAbstractnessKnown(templateArguments.front().typeRefType());
		}
		
		bool RefPrimitive::isSizeKnownInThisModule(const TypeInfo& /*typeInfo*/,
		                                           llvm::ArrayRef<AST::Value> templateArguments) const {
			return isAbstractnessKnown(templateArguments.front().typeRefType());
		}
		
		bool RefPrimitive::hasCustomDestructor(const TypeInfo& /*typeInfo*/,
		                                        llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		bool RefPrimitive::hasCustomMove(const TypeInfo& /*typeInfo*/,
		                                  llvm::ArrayRef<AST::Value> /*templateArguments*/) const {
			return false;
		}
		
		llvm_abi::Type RefPrimitive::getAbstractABIType(Module& module) const {
			return interfaceStructType(module);
		}
		
		llvm_abi::Type RefPrimitive::getConcreteABIType() const {
			return llvm_abi::PointerTy;
		}
		
		llvm_abi::Type RefPrimitive::getABIType(Module& module,
		                                        const AST::Type* const targetType) const {
			if (isAbstract(targetType)) {
				return getAbstractABIType(module);
			} else {
				return getConcreteABIType();
			}
		}
		
		llvm_abi::Type RefPrimitive::getABIType(Module& module,
		                                        const llvm_abi::TypeBuilder& /*abiTypeBuilder*/,
		                                        llvm::ArrayRef<AST::Value> templateArguments) const {
			return getABIType(module, templateArguments.front().typeRefType());
		}
		
		namespace {
			
			template <typename Fn>
			llvm::Value*
			emitRefMethodForAbstractCases(IREmitter& irEmitter,
			                              const RefPrimitive& refPrimitive,
			                              const AST::Type* const refTarget,
			                              Fn f) {
				auto& function = irEmitter.function();
				auto& module = irEmitter.module();
				
				if (refPrimitive.isAbstractnessKnown(refTarget)) {
					// If the reference target type is not a virtual template variable,
					// we already know whether it's a simple pointer or a 'fat'
					// pointer (i.e. a struct containing a pointer to the object
					// as well as the vtable and the template generator), so we
					// only generate for this known case.
					return f(refPrimitive.getABIType(module, refTarget));
				}
				
				// If the reference target type is a template variable, we need
				// to query the abstract-ness of it at run-time and hence we must
				// emit code to handle both cases.
				
				// Look at our template argument to see if it's abstract.
				const auto argTypeInfo = function.getBuilder().CreateExtractValue(function.getTemplateArgs(), { (unsigned int) refTarget->getTemplateVar()->index() });
				const auto argVTablePointer = function.getBuilder().CreateExtractValue(argTypeInfo, { 0 });
				
				// If the VTable pointer is NULL, it's an abstract type, which
				// means we are larger (to store the type information etc.).
				const auto nullVtablePtr = ConstantGenerator(module).getNullPointer();
				const auto isAbstractCondition = function.getBuilder().CreateICmpEQ(argVTablePointer, nullVtablePtr, "isAbstract");
				
				const auto ifAbstractBlock = irEmitter.createBasicBlock("ifRefAbstract");
				const auto ifConcreteBlock = irEmitter.createBasicBlock("ifRefConcrete");
				const auto mergeBlock = irEmitter.createBasicBlock("mergeRefAbstract");
				
				irEmitter.emitCondBranch(isAbstractCondition, ifAbstractBlock,
				                         ifConcreteBlock);
				
				irEmitter.selectBasicBlock(ifAbstractBlock);
				const auto abstractResult = f(refPrimitive.getAbstractABIType(module));
				irEmitter.emitBranch(mergeBlock);
				
				irEmitter.selectBasicBlock(ifConcreteBlock);
				const auto concreteResult = f(refPrimitive.getConcreteABIType());
				irEmitter.emitBranch(mergeBlock);
				
				irEmitter.selectBasicBlock(mergeBlock);
				
				if (!abstractResult->getType()->isVoidTy()) {
					assert(!concreteResult->getType()->isVoidTy());
					if (abstractResult == concreteResult) {
						return abstractResult;
					} else {
						assert(abstractResult->getType() == concreteResult->getType());
						const auto phiNode = function.getBuilder().CreatePHI(abstractResult->getType(), 2);
						phiNode->addIncoming(abstractResult, ifAbstractBlock);
						phiNode->addIncoming(concreteResult, ifConcreteBlock);
						return phiNode;
					}
				} else {
					assert(concreteResult->getType()->isVoidTy());
					return ConstantGenerator(module).getVoidUndef();
				}
			}
			
		}
		
		llvm::Value*
		RefPrimitive::emitGetContextValue(IREmitter& irEmitter,
		                                  const AST::Type* const targetType,
		                                  PendingResultArray& args) const {
			// If the abstract-ness of the reference is known
			// then we can load the reference-to-reference
			// here, otherwise we need to wait until the
			// two cases are evaluated.
			if (isAbstractnessKnown(targetType)) {
				return args[0].resolveWithoutBind(irEmitter.function());
			} else {
				return args[0].resolve(irEmitter.function());
			}
		}
		
		llvm::Value*
		RefPrimitive::emitLoadRef(IREmitter& irEmitter,
		                          const AST::Type* const targetType,
		                          llvm::Value* const value,
		                          const llvm_abi::Type abiType) const {
			// If the abstract-ness of the reference is known
			// then the reference is already loaded, otherwise it
			// needs to be loaded here.
			if (isAbstractnessKnown(targetType)) {
				return value;
			} else {
				return irEmitter.emitRawLoad(value,
				                             abiType);
			}
		}
		
		llvm::Value* RefPrimitive::emitMethod(IREmitter& irEmitter,
		                                      const MethodID methodID,
		                                      llvm::ArrayRef<AST::Value> typeTemplateArguments,
		                                      llvm::ArrayRef<AST::Value> /*functionTemplateArguments*/,
		                                      PendingResultArray args,
		                                      llvm::Value* const resultPtr) const {
			auto& builder = irEmitter.builder();
			auto& function = irEmitter.function();
			auto& module = irEmitter.module();
			
			const auto targetType = typeTemplateArguments.front().typeRefType();
			
			switch (methodID) {
				case METHOD_ALIGNMASK: {
					return emitRefMethodForAbstractCases(irEmitter, *this, targetType,
						[&](const llvm_abi::Type abiType) {
							if (abiType.isPointer()) {
								const auto nonVirtualAlign = module.abi().typeInfo().getTypeRequiredAlign(llvm_abi::PointerTy);
								return ConstantGenerator(module).getSizeTValue(nonVirtualAlign.asBytes() - 1);
							} else {
								const auto virtualAlign = module.abi().typeInfo().getTypeRequiredAlign(interfaceStructType(module));
								return ConstantGenerator(module).getSizeTValue(virtualAlign.asBytes() - 1);
							}
						}
					);
				}
				case METHOD_SIZEOF: {
					return emitRefMethodForAbstractCases(irEmitter, *this, targetType,
						[&](const llvm_abi::Type abiType) {
							if (abiType.isPointer()) {
								const auto nonVirtualSize = module.abi().typeInfo().getTypeRawSize(llvm_abi::PointerTy);
								return ConstantGenerator(module).getSizeTValue(nonVirtualSize.asBytes());
							} else {
								const auto virtualSize = module.abi().typeInfo().getTypeRawSize(interfaceStructType(module));
								return ConstantGenerator(module).getSizeTValue(virtualSize.asBytes());
							}
						}
					);
				}
				case METHOD_COPY:
				case METHOD_IMPLICITCOPY:
				case METHOD_MOVE: {
					// If the virtualness of the reference is known, we
					// can load it, otherwise we have to keep accessing
					// it by pointer.
					if (!isAbstractnessKnown(targetType) && resultPtr == nullptr) {
						return args[0].resolve(function);
					}
					
					auto methodOwner = emitGetContextValue(irEmitter, targetType,
					                                       args);
					
					return emitRefMethodForAbstractCases(irEmitter, *this, targetType,
						[&](const llvm_abi::Type abiType) {
							const auto loadedValue = emitLoadRef(irEmitter, targetType,
							                                     methodOwner, abiType);
							if (!isAbstractnessKnown(targetType)) {
								assert(resultPtr != nullptr);
								irEmitter.emitRawStore(loadedValue,
								                       resultPtr);
								return resultPtr;
							} else {
								return loadedValue;
							}
						});
				}
				case METHOD_ISVALID: {
					auto methodOwner = emitGetContextValue(irEmitter, targetType,
					                                       args);
					
					return emitRefMethodForAbstractCases(irEmitter, *this, targetType,
						[&](const llvm_abi::Type abiType) {
							const auto methodOwnerValue = emitLoadRef(irEmitter, targetType,
							                                          methodOwner, abiType);
							if (abiType.isPointer()) {
								const auto nullValue = ConstantGenerator(module).getNull(abiType);
								return irEmitter.emitI1ToBool(builder.CreateICmpNE(methodOwnerValue, nullValue));
							} else {
								const auto pointerValue = builder.CreateExtractValue(methodOwnerValue, { 0 });
								const auto nullValue = ConstantGenerator(module).getNullPointer();
								return irEmitter.emitI1ToBool(builder.CreateICmpNE(pointerValue, nullValue));
							}
						});
				}
				case METHOD_SETINVALID: {
					auto methodOwnerPtr = args[0].resolve(irEmitter.function());
					
					return emitRefMethodForAbstractCases(irEmitter, *this, targetType,
						[&](const llvm_abi::Type abiType) {
							const auto nullValue = ConstantGenerator(module).getNull(abiType);
							irEmitter.emitRawStore(nullValue, methodOwnerPtr);
							return ConstantGenerator(module).getVoidUndef();
						});
				}
				case METHOD_ISLIVE: {
					(void) args[0].resolveWithoutBind(function);
					return ConstantGenerator(module).getBool(true);
				}
				case METHOD_DESTROY:
				case METHOD_SETDEAD: {
					// Do nothing.
					(void) args[0].resolveWithoutBind(function);
					return ConstantGenerator(module).getVoidUndef();
				}
				case METHOD_ADDRESS: {
					auto methodOwner = emitGetContextValue(irEmitter, targetType,
					                                       args);
					
					return emitRefMethodForAbstractCases(irEmitter, *this, targetType,
						[&](const llvm_abi::Type abiType) {
							const auto refValue = emitLoadRef(irEmitter, targetType,
							                                  methodOwner, abiType);
							if (abiType.isPointer()) {
								return refValue;
							}
							
							// Load first member (which is 'this' pointer) from interface ref struct.
							return function.getBuilder().CreateExtractValue(refValue, { 0 });
						}
					);
				}
				default:
					llvm_unreachable("Unknown ref primitive method.");
			}
		}
		
	}
	
}

