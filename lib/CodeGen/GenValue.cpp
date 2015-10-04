#include <assert.h>

#include <boost/optional.hpp>

#include <locic/Constant.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Debug.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Exception.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/LivenessIndicator.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/MethodInfo.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/CodeGen/VirtualCall.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Value* genValue(Function& function, const SEM::Value& value, llvm::Value* const hintResultValue) {
			auto& module = function.module();
			const auto& debugInfo = value.debugInfo();
			if (debugInfo) {
				function.setDebugPosition(debugInfo->location.range().start());
			}
			
			IREmitter irEmitter(function, hintResultValue);
			
			switch (value.kind()) {
				case SEM::Value::SELF: {
					return function.getContextValue(value.type()->refTarget()->getObjectType());
				}
				case SEM::Value::THIS: {
					return function.getContextValue(value.type()->templateArguments().at(0).typeRefType()->getObjectType());
				}
				case SEM::Value::CONSTANT: {
					switch (value.constant().kind()) {
						case locic::Constant::NULLVAL:
							return ConstantGenerator(module).getNullPointer(TypeGenerator(module).getPtrType());
									   
						case locic::Constant::BOOLEAN:
							return ConstantGenerator(module).getI1(value.constant().boolValue());
						
						case locic::Constant::INTEGER: {
							assert(value.type()->isObject());
							const auto integerValue = value.constant().integerValue();
							return ConstantGenerator(module).getPrimitiveInt(value.type()->primitiveID(), integerValue);
						}
						
						case locic::Constant::FLOATINGPOINT: {
							assert(value.type()->isObject());
							const auto floatValue = value.constant().floatValue();
							return ConstantGenerator(module).getPrimitiveFloat(value.type()->primitiveID(), floatValue);
						}
						
						case locic::Constant::CHARACTER: {
							const auto characterValue = value.constant().characterValue();
							
							const auto primitiveID = value.type()->resolveAliases()->primitiveID();
							
							if (primitiveID == PrimitiveUInt8) {
								return ConstantGenerator(module).getI8(characterValue);
							} else {
								llvm_unreachable("Unknown character literal type.");
							}
						}
						
						case locic::Constant::STRING: {
							const auto& stringValue = value.constant().stringValue();
							
							const auto arrayType =
								TypeGenerator(module).getArrayType(
									TypeGenerator(module).getI8Type(),
										stringValue.size() + 1);
							const auto constArray = ConstantGenerator(module).getString(stringValue);
							const auto globalArray =
								module.createConstGlobal(module.getCString("cstring_constant"),
										arrayType, llvm::GlobalValue::InternalLinkage, constArray);
							globalArray->setAlignment(1);
							
							// Convert array to a pointer.
							return irEmitter.emitConstInBoundsGEP2_32(arrayType,
							                                          globalArray,
							                                          0, 0);
						}
						
						default:
							llvm_unreachable("Unknown constant kind.");
					}
				}
				
				case SEM::Value::ALIAS: {
					const auto& alias = value.alias();
					
					SEM::TemplateVarMap assignments(alias.templateVariables().copy(),
					                                value.aliasTemplateArguments().copy());
					auto resolvedValue = alias.value().substitute(assignments);
					return genValue(function, std::move(resolvedValue), hintResultValue);
				}
				
				case SEM::Value::LOCALVAR: {
					const auto& var = value.localVar();
					return function.getLocalVarMap().get(&var);
				}
				
				case SEM::Value::REINTERPRET: {
					const auto sourceValue = genValue(function, value.reinterpretOperand());
					const auto targetType = genType(module, value.type());
					
					// Currently, reinterpret_cast is only implemented for pointers.
					return function.getBuilder().CreatePointerCast(sourceValue, targetType);
				}
				
				case SEM::Value::DEREF_REFERENCE: {
					llvm::Value* const refValue = genValue(function, value.derefOperand());
					return genMoveLoad(function, refValue, value.type());
				}
				
				case SEM::Value::UNIONDATAOFFSET: {
					// Offset of union datatype data is equivalent to its
					// alignment size.
					return genAlignOf(function, value.unionDataOffsetTypeInstance()->selfType());
				}
				
				case SEM::Value::MEMBEROFFSET: {
					// Offset of union datatype data is equivalent to its
					// alignment size.
					return genMemberOffset(function, value.memberOffsetTypeInstance()->selfType(), value.memberOffsetMemberIndex());
				}
				
				case SEM::Value::TERNARY: {
					const auto condValue = genValue(function, value.ternaryCondition());
					
					const auto ifTrueBB = function.createBasicBlock("");
					const auto ifFalseBB = function.createBasicBlock("");
					const auto afterCondBB = function.createBasicBlock("");
					
					const auto currentBB = function.getBuilder().GetInsertBlock();
					
					function.selectBasicBlock(ifTrueBB);
					const auto ifTrueValue = genValue(function, value.ternaryIfTrue(), hintResultValue);
					const auto ifTrueTermBlock = function.getBuilder().GetInsertBlock();
					const auto ifTrueIsEmpty = ifTrueBB->empty();
					function.getBuilder().CreateBr(afterCondBB);
					
					function.selectBasicBlock(ifFalseBB);
					const auto ifFalseValue = genValue(function, value.ternaryIfFalse(), hintResultValue);
					const auto ifFalseTermBlock = function.getBuilder().GetInsertBlock();
					const auto ifFalseIsEmpty = ifFalseBB->empty();
					function.getBuilder().CreateBr(afterCondBB);
					
					function.selectBasicBlock(currentBB);
					
					if (ifTrueIsEmpty && ifFalseIsEmpty) {
						// If possible, create a select instruction.
						ifTrueBB->eraseFromParent();
						ifFalseBB->eraseFromParent();
						afterCondBB->eraseFromParent();
						return function.getBuilder().CreateSelect(condValue, ifTrueValue, ifFalseValue);
					}
					
					const auto ifTrueBranchBB = !ifTrueIsEmpty ? ifTrueBB : afterCondBB;
					const auto ifFalseBranchBB = !ifFalseIsEmpty ? ifFalseBB : afterCondBB;
					const auto ifTrueReceiveBB = !ifTrueIsEmpty ? ifTrueTermBlock : currentBB;
					const auto ifFalseReceiveBB = !ifFalseIsEmpty ? ifFalseTermBlock : currentBB;
					
					if (ifTrueIsEmpty) {
						ifTrueBB->eraseFromParent();
					}
					
					if (ifFalseIsEmpty) {
						ifFalseBB->eraseFromParent();
					}
					
					function.getBuilder().CreateCondBr(condValue, ifTrueBranchBB, ifFalseBranchBB);
					
					function.selectBasicBlock(afterCondBB);
					
					if (ifTrueValue->stripPointerCasts() == ifFalseValue->stripPointerCasts()) {
						assert(ifTrueValue->getType()->isPointerTy() && ifFalseValue->getType()->isPointerTy());
						return function.getBuilder().CreatePointerCast(ifTrueValue, genPointerType(module, value.type()));
					} else {
						const auto phiNode = function.getBuilder().CreatePHI(ifTrueValue->getType(), 2);
						phiNode->addIncoming(ifTrueValue, ifTrueReceiveBB);
						phiNode->addIncoming(ifFalseValue, ifFalseReceiveBB);
						return phiNode;
					}
				}
				
				case SEM::Value::CAST: {
					const auto& castValue = value.castOperand();
					const auto codeValue = genValue(function, castValue);
					const auto sourceType = castValue.type()->resolveAliases();
					const auto destType = value.type()->resolveAliases();
					assert((sourceType->kind() == destType->kind()
							|| (sourceType->isPrimitive() && sourceType->getObjectType()->name().last() == "null_t")
							|| destType->isBuiltInVoid())
						   && "Types must be in the same group for cast, or "
						   "it should be a cast from null, or a cast to void");
						   
					if (destType->isBuiltInVoid()) {
						// Call destructor for the value.
						genDestructorCall(function, sourceType, codeValue);
						
						// All casts to void have the same outcome.
						return ConstantGenerator(module).getVoidUndef();
					}
					
					switch (sourceType->kind()) {
						case SEM::Type::OBJECT: {
							if (sourceType->getObjectType() == destType->getObjectType()) {
								return codeValue;
							}
							
							if (sourceType->isDatatype() && destType->isUnionDatatype()) {
								// Start from 1 so 0 can be used to represent 'empty'.
								uint8_t variantKind = 1;
								for (auto variantTypeInstance: destType->getObjectType()->variants()) {
									if (variantTypeInstance == sourceType->getObjectType()) break;
									variantKind++;
								}
								
								const auto unionValue = hintResultValue != nullptr ? hintResultValue : genAlloca(function, destType);
								
								const auto unionDatatypePointers = getUnionDatatypePointers(function, destType, unionValue);
								
								// Set the variant kind value.
								irEmitter.emitRawStore(ConstantGenerator(module).getI8(variantKind),
								                       unionDatatypePointers.first);
								
								// Store the union value.
								const auto unionValueType = genType(function.module(), sourceType);
								const auto castedUnionValuePtr = function.getBuilder().CreatePointerCast(unionDatatypePointers.second, unionValueType->getPointerTo());
								genMoveStore(function, codeValue, castedUnionValuePtr, sourceType);
								
								return genMoveLoad(function, unionValue, destType);
							}
							
							assert(false && "Casts between named types not implemented.");
							return nullptr;
						}
						
						case SEM::Type::TEMPLATEVAR: {
							return codeValue;
						}
						
						default:
							assert(false && "Unknown type in cast.");
							return nullptr;
					}
				}
				
				case SEM::Value::POLYCAST: {
					const auto& castValue = value.polyCastOperand();
					const auto rawValue = genValue(function, castValue);
					const auto sourceType = castValue.type();
					
					if (sourceType->isStaticRef()) {
						const auto sourceTarget = sourceType->staticRefTarget();
						
						if (sourceTarget->isInterface()) {
							// Since the vtable is a hash table and it has
							// already been generated, this is a no-op.
							return rawValue;
						}
						
						// Generate the vtable and template generator.
						const auto vtablePointer = genVTable(module, sourceTarget->resolveAliases()->getObjectType());
						const auto templateGenerator = getTemplateGenerator(function, TemplateInst::Type(sourceTarget));
						
						// Build the new type info struct with these values.
						return makeTypeInfoValue(function, vtablePointer, templateGenerator);
					} else if (sourceType->isRef()) {
						assert(sourceType->isBuiltInReference()  && "Polycast source type must be reference.");
						assert(value.type()->isBuiltInReference() && "Polycast dest type must be reference.");
						assert(value.type()->refTarget()->isInterface() && "Polycast dest target type must be interface");
						
						const auto sourceTarget = sourceType->refTarget();
						
						if (sourceTarget->isInterface()) {
							// Since the vtable is a hash table and it has
							// already been generated, this is a no-op.
							return rawValue;
						}
						
						// Cast class pointer to pointer to the opaque struct
						// representing destination interface type.
						const auto objectPointer = function.getBuilder().CreatePointerCast(rawValue,
									TypeGenerator(module).getPtrType());
						
						// Generate the vtable and template generator.
						const auto vtablePointer = genVTable(module, sourceTarget->resolveAliases()->getObjectType());
						const auto templateGenerator = getTemplateGenerator(function, TemplateInst::Type(sourceTarget));
						
						// Build the new interface struct with these values.
						return makeInterfaceStructValue(function, objectPointer, makeTypeInfoValue(function, vtablePointer, templateGenerator));
					} else {
						llvm_unreachable("Poly cast type must be ref or staticref.");
					}
				}
				
				case SEM::Value::LVAL: {
					return genValue(function, value.makeLvalOperand(), hintResultValue);
				}
				
				case SEM::Value::NOLVAL: {
					return genValue(function, value.makeNoLvalOperand(), hintResultValue);
				}
				
				case SEM::Value::REF: {
					return genValue(function, value.makeRefOperand(), hintResultValue);
				}
				
				case SEM::Value::NOREF: {
					return genValue(function, value.makeNoRefOperand(), hintResultValue);
				}
				
				case SEM::Value::STATICREF: {
					return genValue(function, value.makeStaticRefOperand(), hintResultValue);
				}
				
				case SEM::Value::NOSTATICREF: {
					return genValue(function, value.makeNoStaticRefOperand(), hintResultValue);
				}
				
				case SEM::Value::INTERNALCONSTRUCT: {
					const auto& parameterValues = value.internalConstructParameters();
					const auto& parameterVars = value.type()->getObjectType()->variables();
					
					const auto type = value.type();
					assert(!type->isUnion());
					
					const auto objectValue = genAlloca(function, type, hintResultValue);
					
					TypeInfo typeInfo(module);
					if (typeInfo.isSizeKnownInThisModule(type)) {
						const auto objectIRType = genType(module, type);
						for (size_t i = 0; i < parameterValues.size(); i++) {
							const auto var = parameterVars.at(i);
							const auto llvmInsertPointer = irEmitter.emitConstInBoundsGEP2_32(objectIRType,
							                                                                  objectValue,
							                                                                  0, i);
							const auto llvmParamValue = genValue(function, parameterValues.at(i), llvmInsertPointer);
							genStoreVar(function, llvmParamValue, llvmInsertPointer, var);
						}
					} else {
						const auto castObjectValue = function.getBuilder().CreatePointerCast(objectValue, TypeGenerator(module).getPtrType());
						llvm::Value* offsetValue = ConstantGenerator(module).getSizeTValue(0);
						
						for (size_t i = 0; i < parameterValues.size(); i++) {
							const auto var = parameterVars.at(i);
							
							// Align offset for field.
							offsetValue = makeAligned(function, offsetValue, genAlignMask(function, var->type()));
							
							const auto llvmInsertPointer = irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
							                                                         castObjectValue, offsetValue);
							const auto castInsertPointer = function.getBuilder().CreatePointerCast(llvmInsertPointer, genPointerType(module, var->type()));
							const auto insertHintPointer = function.getBuilder().CreatePointerCast(castInsertPointer, genPointerType(module, parameterValues.at(i).type()));
							
							const auto llvmParamValue = genValue(function, parameterValues.at(i), insertHintPointer);
							
							genStoreVar(function, llvmParamValue, castInsertPointer, var);
							
							if ((i + 1) != parameterValues.size()) {
								// If this isn't the last field, add its size for calculating
								// the offset of the next field.
								offsetValue = function.getBuilder().CreateAdd(offsetValue, genSizeOf(function, var->type()));
							}
						}
					}
					
					// Set object into live state (e.g. set gap byte to 1).
					setOuterLiveState(function, *(type->getObjectType()), objectValue);
					
					return genMoveLoad(function, objectValue, type);
				}
				
				case SEM::Value::MEMBERACCESS: {
					const auto memberIndex = module.getMemberVarMap().at(&(value.memberAccessVar()));
					
					const auto& dataRefValue = value.memberAccessObject();
					assert(dataRefValue.type()->isRef() && dataRefValue.type()->isBuiltInReference());
					
					const auto llvmDataRefValue = genValue(function, dataRefValue);
					return genMemberPtr(function, llvmDataRefValue, dataRefValue.type()->refTarget(), memberIndex);
				}
				
				case SEM::Value::BIND_REFERENCE: {
					const auto& dataValue = value.bindReferenceOperand();
					const auto llvmDataValue = genValue(function, dataValue);
					return genValuePtr(function, llvmDataValue, dataValue.type());
				}
				
				case SEM::Value::TYPEREF: {
					const auto targetType = value.typeRefType();
					
					const auto vtablePointer = genVTable(module, targetType->resolveAliases()->getObjectType());
					const auto templateGenerator = getTemplateGenerator(function, TemplateInst::Type(targetType));
					
					// Build the new type info struct with these values.
					return makeTypeInfoValue(function, vtablePointer, templateGenerator);
				}
				
				case SEM::Value::CALL: {
					const auto& semCallValue = value.callValue();
					const auto& semArgumentValues = value.callParameters();
					
					if (semCallValue.type()->isBuiltInInterfaceMethod() || semCallValue.type()->isBuiltInStaticInterfaceMethod()) {
						const auto methodComponents = genVirtualMethodComponents(function, semCallValue);
						
						llvm::SmallVector<llvm::Value*, 10> llvmArgs;
						for (const auto& arg: semArgumentValues) {
							llvmArgs.push_back(genValue(function, arg));
						}
						
						return VirtualCall::generateCall(function, semCallValue.type()->asFunctionType(), methodComponents, llvmArgs, hintResultValue);
					}
					
					// TODO: merge this with the call below.
					if (isTrivialFunction(module, semCallValue)) {
						return genTrivialFunctionCall(function, semCallValue, arrayRef(semArgumentValues), hintResultValue);
					}
					
					return genSEMFunctionCall(function, semCallValue, arrayRef(semArgumentValues), hintResultValue);
				}
				
				case SEM::Value::FUNCTIONREF:
				case SEM::Value::TEMPLATEFUNCTIONREF: {
					const auto callInfo = genFunctionCallInfo(function, value);
					
					if (callInfo.templateGenerator != nullptr) {
						llvm::Value* functionValue = ConstantGenerator(module).getUndef(genType(module, value.type()));
						functionValue = irEmitter.emitInsertValue(functionValue, callInfo.functionPtr, { 0 });
						functionValue = irEmitter.emitInsertValue(functionValue, callInfo.templateGenerator, { 1 });
						return functionValue;
					} else {
						return callInfo.functionPtr;
					}
				}
				
				case SEM::Value::METHODOBJECT: {
					const auto callInfo = genFunctionCallInfo(function, value);
					
					llvm::Value* functionValue = nullptr;
					
					if (callInfo.templateGenerator != nullptr) {
						functionValue = ConstantGenerator(module).getUndef(genType(module, value.type()));
						functionValue = irEmitter.emitInsertValue(functionValue, callInfo.functionPtr, { 0 });
						functionValue = irEmitter.emitInsertValue(functionValue, callInfo.templateGenerator, { 1 });
					} else {
						functionValue = callInfo.functionPtr;
					}
					
					llvm::Value* methodValue = ConstantGenerator(module).getUndef(genType(module, value.type()));
					methodValue = irEmitter.emitInsertValue(methodValue, callInfo.contextPointer, { 0 });
					methodValue = irEmitter.emitInsertValue(methodValue, functionValue, { 1 });
					return methodValue;
				}
				
				case SEM::Value::INTERFACEMETHODOBJECT: {
					const auto& method = value.interfaceMethodObject();
					const auto methodOwner = genValue(function, value.interfaceMethodOwner());
					
					assert(method.kind() == SEM::Value::FUNCTIONREF);
					
					const auto interfaceFunction = method.functionRefFunction();
					const auto methodHash = CreateMethodNameHash(interfaceFunction->name().last());
					const auto methodHashValue = ConstantGenerator(module).getI64(methodHash);
					return makeInterfaceMethodValue(function, methodOwner, methodHashValue);
				}
				
				case SEM::Value::STATICINTERFACEMETHODOBJECT: {
					const auto& method = value.staticInterfaceMethodObject();
					const auto typeRefPtr = genValue(function, value.staticInterfaceMethodOwner());
					
					const auto typeRef = irEmitter.emitRawLoad(typeRefPtr,
					                                           typeInfoType(module).second);
					
					assert(method.kind() == SEM::Value::FUNCTIONREF);
					
					const auto interfaceFunction = method.functionRefFunction();
					const auto methodHash = CreateMethodNameHash(interfaceFunction->name().last());
					const auto methodHashValue = ConstantGenerator(module).getI64(methodHash);
					return makeStaticInterfaceMethodValue(function, typeRef, methodHashValue);
				}
				
				case SEM::Value::TEMPLATEVARREF: {
					const auto templateArgs = function.getTemplateArgs();
					const auto templateVar = value.templateVar();
					const auto index = templateVar->index();
					const unsigned extractIndexArray[] = { (unsigned) index, 0 };
					const auto valueEntry = function.getBuilder().CreateExtractValue(templateArgs,
					                                                                 extractIndexArray);
					
					SEM::FunctionAttributes attributes(/*isVarArg=*/false,
					                                   /*isMethod=*/false,
					                                   /*isTemplated=*/false,
					                                   /*noExceptPredicate=*/SEM::Predicate::True());
					SEM::FunctionType functionType(std::move(attributes),
					                               /*returnType=*/value.type(),
					                               /*parameterTypes=*/{});
					
					const auto argInfo = getFunctionArgInfo(module, functionType);
					const auto functionPtrType = argInfo.makeFunctionType()->getPointerTo();
					
					FunctionCallInfo callInfo;
					callInfo.functionPtr = function.getBuilder().CreatePointerCast(valueEntry,
					                                                               functionPtrType);
					
					return genFunctionCall(function,
					                       functionType,
					                       callInfo,
					                       /*args=*/{},
					                       hintResultValue);
				}
				
				case SEM::Value::PREDICATE:
				case SEM::Value::CAPABILITYTEST:
				case SEM::Value::CASTDUMMYOBJECT:
					llvm_unreachable("Invalid value enum for code generation.");
			}
			
			llvm_unreachable("Unknown value enum.");
		}
		
	}
	
}

