#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/DefaultMethodEmitter.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/SizeOf.hpp>

#include <locic/SEM/Function.hpp>
#include <locic/SEM/FunctionType.hpp>
#include <locic/SEM/Predicate.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>

#include <locic/Support/MethodID.hpp>

namespace locic {
	
	namespace CodeGen {
		
		DefaultMethodEmitter::DefaultMethodEmitter(Function& functionGenerator)
		: functionGenerator_(functionGenerator) { }
		
		llvm::Value*
		DefaultMethodEmitter::emitMethod(const MethodID methodID,
		                                 const SEM::Type* const type,
		                                 const SEM::FunctionType functionType,
		                                 PendingResultArray args,
		                                 llvm::Value* const hintResultValue) {
			switch (methodID) {
				case METHOD_CREATE:
					llvm_unreachable("TODO!");
				case METHOD_COMPARE:
					llvm_unreachable("TODO!");
				case METHOD_ALIGNMASK:
				case METHOD_SIZEOF:
				case METHOD_MOVETO:
				case METHOD_ISLIVE:
				case METHOD_SETDEAD:
					llvm_unreachable("Generated elsewhere.");
				case METHOD_IMPLICITCOPY:
					return emitImplicitCopy(type,
					                        functionType,
					                        std::move(args),
					                        hintResultValue);
				case METHOD_COPY:
					return emitExplicitCopy(type,
					                        functionType,
					                        std::move(args),
					                        hintResultValue);
				default:
					llvm_unreachable("Unknown default function.");
			}
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitImplicitCopy(const SEM::Type* const type,
		                                       const SEM::FunctionType functionType,
		                                       PendingResultArray args,
		                                       llvm::Value* const hintResultValue) {
			return emitCopyMethod(METHOD_IMPLICITCOPY,
			                      type,
			                      functionType,
			                      std::move(args),
			                      hintResultValue);
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitExplicitCopy(const SEM::Type* const type,
		                                       const SEM::FunctionType functionType,
		                                       PendingResultArray args,
		                                       llvm::Value* const hintResultValue) {
			return emitCopyMethod(METHOD_COPY,
			                      type,
			                      functionType,
			                      std::move(args),
			                      hintResultValue);
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitCopyMethod(const MethodID methodID,
		                                     const SEM::Type* const type,
		                                     const SEM::FunctionType /*functionType*/,
		                                     PendingResultArray args,
		                                     llvm::Value* const hintResultValue) {
			assert(methodID == METHOD_IMPLICITCOPY ||
			       methodID == METHOD_COPY);
			
			const auto& typeInstance = *(type->getObjectType());
			
			const auto thisPointer = args[0].resolve(functionGenerator_);
			
			IREmitter irEmitter(functionGenerator_);
			
			if (typeInstance.isUnion()) {
				return irEmitter.emitMoveLoad(thisPointer, type);
			}
			
			auto& module = functionGenerator_.module();
			
			const auto resultValue = irEmitter.emitAlloca(type,
			                                              hintResultValue);
			
			if (typeInstance.isUnionDatatype()) {
				const auto loadedTag = irEmitter.emitLoadDatatypeTag(thisPointer);
				irEmitter.emitStoreDatatypeTag(loadedTag, resultValue);
				
				const auto endBB = functionGenerator_.createBasicBlock("end");
				const auto switchInstruction = functionGenerator_.getBuilder().CreateSwitch(loadedTag, endBB, typeInstance.variants().size());
				
				// Start from 1 so that 0 can represent 'empty'.
				uint8_t tag = 1;
				
				for (const auto variantTypeInstance : typeInstance.variants()) {
					const auto matchBB = functionGenerator_.createBasicBlock("tagMatch");
					const auto tagValue = ConstantGenerator(module).getI8(tag++);
					
					switchInstruction->addCase(tagValue, matchBB);
					
					functionGenerator_.selectBasicBlock(matchBB);
					
					const auto variantType = SEM::Type::Object(variantTypeInstance, type->templateArguments().copy());
					
					const auto unionValuePtr = irEmitter.emitGetDatatypeVariantPtr(thisPointer,
					                                                               type,
					                                                               variantType);
					
					const auto copyResult = irEmitter.emitCopyCall(methodID,
					                                               unionValuePtr,
					                                               variantType);
					
					const auto unionValueDestPtr = irEmitter.emitGetDatatypeVariantPtr(resultValue,
					                                                                   type,
					                                                                   variantType);
					
					irEmitter.emitMoveStore(copyResult,
					                        unionValueDestPtr,
					                        variantType);
					
					functionGenerator_.getBuilder().CreateBr(endBB);
				}
				
				functionGenerator_.selectBasicBlock(endBB);
			} else {
				for (const auto& memberVar: typeInstance.variables()) {
					const size_t memberIndex = module.getMemberVarMap().at(memberVar);
					const auto ptrToMember = genMemberPtr(functionGenerator_,
					                                      thisPointer,
					                                      type,
					                                      memberIndex);
					
					const auto memberType = memberVar->constructType()->resolveAliases();
					
					const auto copyResult = irEmitter.emitCopyCall(methodID,
					                                               ptrToMember,
					                                               memberType);
					
					const auto resultPtr = genMemberPtr(functionGenerator_, resultValue, type, memberIndex);
					
					irEmitter.emitMoveStore(copyResult,
					                        resultPtr,
					                        memberType);
				}
			}
			
			return irEmitter.emitMoveLoad(resultValue, type);
		}
		
	}
	
}

