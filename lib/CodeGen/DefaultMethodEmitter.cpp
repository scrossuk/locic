#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/DefaultMethodEmitter.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

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
					return emitCreateConstructor(type,
					                             functionType,
					                             std::move(args),
					                             hintResultValue);
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
				case METHOD_COMPARE:
					return emitCompare(type,
					                   functionType,
					                   std::move(args));
				default:
					llvm_unreachable("Unknown default function.");
			}
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitCreateConstructor(const SEM::Type* const type,
		                                            const SEM::FunctionType /*functionType*/,
		                                            PendingResultArray args,
		                                            llvm::Value* const hintResultValue) {
			const auto& typeInstance = *(type->getObjectType());
			assert(!typeInstance.isUnionDatatype());
			
			auto& module = functionGenerator_.module();
			
			if (typeInstance.isUnion()) {
				assert(hintResultValue == nullptr);
				return ConstantGenerator(module).getNull(genType(module, type));
			}
			
			IREmitter irEmitter(functionGenerator_);
			
			const auto resultValue = irEmitter.emitAlloca(type,
			                                              hintResultValue);
			
			for (size_t i = 0; i < typeInstance.variables().size(); i++) {
				const auto& memberVar = typeInstance.variables()[i];
				const size_t memberIndex = module.getMemberVarMap().at(memberVar);
				
				const auto memberType = memberVar->constructType()->resolveAliases();
				
				const auto resultPtr = genMemberPtr(functionGenerator_, resultValue, type, memberIndex);
				
				irEmitter.emitMoveStore(args[i].resolve(functionGenerator_),
				                        resultPtr,
				                        memberType);
			}
			
			// Set object into live state (e.g. set gap byte to 1).
			setOuterLiveState(functionGenerator_,
			                  typeInstance,
			                  resultValue);
			
			return irEmitter.emitMoveLoad(resultValue, type);
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
				const auto switchInstruction = functionGenerator_.getBuilder().CreateSwitch(loadedTag,
				                                                                            endBB,
				                                                                            typeInstance.variants().size());
				
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
		
		static const SEM::Type*
		createRefType(const SEM::Type* const refTargetType,
		              const SEM::TypeInstance& refTypeInstance,
		              const SEM::Type* const typenameType) {
			auto typeRef = SEM::Value::TypeRef(refTargetType,
			                                   typenameType->createStaticRefType(refTargetType));
			SEM::ValueArray templateArguments;
			templateArguments.push_back(std::move(typeRef));
			return SEM::Type::Object(&refTypeInstance,
			                         std::move(templateArguments))->createRefType(refTargetType);
		}
		
		llvm::Value*
		DefaultMethodEmitter::emitCompare(const SEM::Type* const type,
		                                  const SEM::FunctionType functionType,
		                                  PendingResultArray args) {
			const auto& typeInstance = *(type->getObjectType());
			assert(!typeInstance.isUnion() &&
			       "Unions don't support default compare");
			
			const auto otherPointer = args[1].resolve(functionGenerator_);
			const auto thisPointer = args[0].resolve(functionGenerator_);
			
			IREmitter irEmitter(functionGenerator_);
			
			auto& module = functionGenerator_.module();
			const auto i8Type = TypeGenerator(module).getI8Type();
			
			const auto compareResultType = functionType.returnType();
			const auto& refTypeInstance = *(functionType.parameterTypes()[0]->getObjectType());
			const auto typenameType = refTypeInstance.templateVariables()[0]->type();
			
			if (typeInstance.isUnionDatatype()) {
				const auto thisTag = irEmitter.emitLoadDatatypeTag(thisPointer);
				const auto otherTag = irEmitter.emitLoadDatatypeTag(otherPointer);
				
				const auto isTagNotEqual = functionGenerator_.getBuilder().CreateICmpNE(thisTag,
				                                                                        otherTag);
				
				const auto isTagLessThan = functionGenerator_.getBuilder().CreateICmpSLT(thisTag,
				                                                                         otherTag);
				
				const auto tagCompareResult = functionGenerator_.getBuilder().CreateSelect(isTagLessThan,
				                                                                           ConstantGenerator(module).getI8(-1),
				                                                                           ConstantGenerator(module).getI8(1));
				
				const auto startCompareBB = functionGenerator_.createBasicBlock("startCompare");
				
				const auto endBB = functionGenerator_.createBasicBlock("end");
				
				const auto phiNode = llvm::PHINode::Create(i8Type,
				                                           typeInstance.variants().size(),
				                                           "compare_result",
				                                           endBB);
				
				phiNode->addIncoming(tagCompareResult,
				                     functionGenerator_.getBuilder().GetInsertBlock());
				
				functionGenerator_.getBuilder().CreateCondBr(isTagNotEqual,
				                                             endBB,
				                                             startCompareBB);
				
				functionGenerator_.selectBasicBlock(startCompareBB);
				
				const auto unreachableBB = functionGenerator_.createBasicBlock("");
				
				const auto switchInstruction = functionGenerator_.getBuilder().CreateSwitch(thisTag,
				                                                                            unreachableBB,
				                                                                            typeInstance.variants().size());
				
				functionGenerator_.selectBasicBlock(unreachableBB);
				
				functionGenerator_.getBuilder().CreateUnreachable();
				
				// Start from 1 so that 0 can represent 'empty'.
				uint8_t tag = 1;
				
				for (const auto variantTypeInstance : typeInstance.variants()) {
					const auto matchBB = functionGenerator_.createBasicBlock("tagMatch");
					const auto tagValue = ConstantGenerator(module).getI8(tag++);
					
					switchInstruction->addCase(tagValue, matchBB);
					
					functionGenerator_.selectBasicBlock(matchBB);
					
					const auto variantType = SEM::Type::Object(variantTypeInstance, type->templateArguments().copy());
					const auto variantRefType = createRefType(variantType,
					                                          refTypeInstance,
					                                          typenameType);
					
					const auto thisValuePtr = irEmitter.emitGetDatatypeVariantPtr(thisPointer,
					                                                              type,
					                                                              variantType);
					
					const auto otherValuePtr = irEmitter.emitGetDatatypeVariantPtr(otherPointer,
					                                                               type,
					                                                               variantType);
					
					const auto compareResult = irEmitter.emitCompareCall(thisValuePtr,
					                                                     otherValuePtr,
					                                                     compareResultType,
					                                                     variantType,
					                                                     variantRefType);
					
					phiNode->addIncoming(compareResult,
					                     matchBB);
					
					functionGenerator_.getBuilder().CreateBr(endBB);
				}
				
				functionGenerator_.selectBasicBlock(endBB);
				return phiNode;
			} else {
				if (typeInstance.variables().empty()) {
					// Return equal result for empty objects.
					return ConstantGenerator(module).getI8(0);
				}
				
				const auto endBB = functionGenerator_.createBasicBlock("end");
				
				const auto phiNode = llvm::PHINode::Create(i8Type,
				                                           typeInstance.variables().size(),
				                                           "compare_result",
				                                           endBB);
				
				for (size_t i = 0; i < typeInstance.variables().size(); i++) {
					const auto& memberVar = typeInstance.variables()[i];
					const size_t memberIndex = module.getMemberVarMap().at(memberVar);
					const auto thisMemberPtr = genMemberPtr(functionGenerator_,
					                                        thisPointer,
					                                        type,
					                                        memberIndex);
					const auto otherMemberPtr = genMemberPtr(functionGenerator_,
					                                         otherPointer,
					                                         type,
					                                         memberIndex);
					
					const auto memberType = memberVar->constructType()->resolveAliases();
					const auto memberRefType = createRefType(memberType,
					                                         refTypeInstance,
					                                         typenameType);
					
					const auto compareResult = irEmitter.emitCompareCall(thisMemberPtr,
					                                                     otherMemberPtr,
					                                                     compareResultType,
					                                                     memberType,
					                                                     memberRefType);
					
					phiNode->addIncoming(compareResult,
					                     functionGenerator_.getBuilder().GetInsertBlock());
					
					if (i != (typeInstance.variables().size() - 1)) {
						const auto nextCompareBB = functionGenerator_.createBasicBlock("nextCompare");
						
						const auto zeroValue = ConstantGenerator(module).getI8(0);
						
						const auto isEqualResult = functionGenerator_.getBuilder().CreateICmpEQ(compareResult,
						                                                                        zeroValue);
						
						functionGenerator_.getBuilder().CreateCondBr(isEqualResult,
						                                             nextCompareBB,
						                                             endBB);
						
						functionGenerator_.selectBasicBlock(nextCompareBB);
					} else {
						functionGenerator_.getBuilder().CreateBr(endBB);
					}
				}
				
				functionGenerator_.selectBasicBlock(endBB);
				return phiNode;
			}
		}
		
	}
	
}

