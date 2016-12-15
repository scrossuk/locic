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
#include <locic/CodeGen/ValueEmitter.hpp>
#include <locic/CodeGen/VirtualCallABI.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
		
		namespace {
			
			llvm::Value* getArrayIndex(IREmitter& irEmitter,
			                           const SEM::Type* const elementType,
			                           llvm::Value* const arrayPtr,
			                           llvm::Value* const elementIndex) {
				auto& builder = irEmitter.builder();
				auto& module = irEmitter.module();
				
				TypeInfo typeInfo(module);
				if (typeInfo.isSizeAlwaysKnown(elementType)) {
					return irEmitter.emitInBoundsGEP(genType(module, elementType),
					                                 arrayPtr,
					                                 elementIndex);
				} else {
					const auto elementSize = genSizeOf(irEmitter.function(),
					                                   elementType);
					const auto indexPos = builder.CreateMul(elementSize,
					                                        elementIndex);
					return irEmitter.emitInBoundsGEP(irEmitter.typeGenerator().getI8Type(),
					                                 arrayPtr,
					                                 indexPos);
				}
			}
			
		}
		
		ValueEmitter::ValueEmitter(IREmitter& irEmitter)
		: irEmitter_(irEmitter) { }
		
		llvm::Value*
		ValueEmitter::emitValue(const SEM::Value& value,
		                        llvm::Value* const hintResultValue) {
			const auto& debugInfo = value.debugInfo();
			if (debugInfo) {
				irEmitter_.function().setDebugPosition(debugInfo->location.range().start());
			}
			
			switch (value.kind()) {
				case SEM::Value::SELF:
					return emitSelf();
				case SEM::Value::THIS:
					return emitThis();
				case SEM::Value::CONSTANT:
					return emitConstant(value);
				case SEM::Value::ALIAS:
					return emitAlias(value, hintResultValue);
				case SEM::Value::LOCALVAR:
					return emitLocalVar(value);
				case SEM::Value::REINTERPRET:
					return emitReinterpretCast(value, hintResultValue);
				case SEM::Value::DEREF_REFERENCE:
					return emitDerefReference(value);
				case SEM::Value::UNIONDATAOFFSET:
					return emitUnionDataOffset(value);
				case SEM::Value::MEMBEROFFSET:
					return emitMemberOffset(value);
				case SEM::Value::TERNARY:
					return emitTernary(value, hintResultValue);
				case SEM::Value::CAST:
					return emitCast(value, hintResultValue);
				case SEM::Value::POLYCAST:
					return emitPolyCast(value);
				case SEM::Value::LVAL:
					return emitLval(value, hintResultValue);
				case SEM::Value::NOLVAL:
					return emitNoLval(value, hintResultValue);
				case SEM::Value::REF:
					return emitRef(value, hintResultValue);
				case SEM::Value::NOREF:
					return emitNoRef(value, hintResultValue);
				case SEM::Value::STATICREF:
					return emitStaticRef(value, hintResultValue);
				case SEM::Value::NOSTATICREF:
					return emitNoStaticRef(value, hintResultValue);
				case SEM::Value::INTERNALCONSTRUCT:
					return emitInternalConstruct(value, hintResultValue);
				case SEM::Value::MEMBERACCESS:
					return emitMemberAccess(value);
				case SEM::Value::BIND_REFERENCE:
					return emitBindReference(value);
				case SEM::Value::TYPEREF:
					return emitTypeRef(value);
				case SEM::Value::CALL:
					return emitCall(value, hintResultValue);
				case SEM::Value::FUNCTIONREF:
				case SEM::Value::TEMPLATEFUNCTIONREF:
					return emitFunctionRef(value);
				case SEM::Value::METHODOBJECT:
					return emitMethodObject(value);
				case SEM::Value::INTERFACEMETHODOBJECT:
					return emitInterfaceMethodObject(value);
				case SEM::Value::STATICINTERFACEMETHODOBJECT:
					return emitStaticInterfaceMethodObject(value);
				case SEM::Value::TEMPLATEVARREF:
					return emitTemplateVarRef(value, hintResultValue);
				case SEM::Value::ARRAYLITERAL:
					return emitArrayLiteral(value, hintResultValue);
				case SEM::Value::PREDICATE:
				case SEM::Value::CAPABILITYTEST:
				case SEM::Value::CASTDUMMYOBJECT:
					llvm_unreachable("Invalid value enum for code generation.");
			}
			
			llvm_unreachable("Unknown value enum.");
		}
		
		llvm::Value*
		ValueEmitter::emitSelf() {
			return irEmitter_.function().getContextValue();
		}
		
		llvm::Value*
		ValueEmitter::emitThis() {
			return irEmitter_.function().getContextValue();
		}
		
		llvm::Value*
		ValueEmitter::emitConstant(const SEM::Value& value) {
			switch (value.constant().kind()) {
				case locic::Constant::NULLVAL:
					return irEmitter_.constantGenerator().getNullPointer();
				case locic::Constant::BOOLEAN:
					return irEmitter_.constantGenerator().getBool(value.constant().boolValue());
				case locic::Constant::INTEGER: {
					assert(value.type()->isObject());
					const auto& integerValue = value.constant().integerValue();
					
					// We can currently only have ints up to 64 bits in size.
					const auto int64Value = integerValue.asUint64();
					
					return irEmitter_.constantGenerator().getPrimitiveInt(value.type()->primitiveID(), int64Value);
				}
				
				case locic::Constant::FLOATINGPOINT: {
					assert(value.type()->isObject());
					const auto floatValue = value.constant().floatValue();
					return irEmitter_.constantGenerator().getPrimitiveFloat(value.type()->primitiveID(), floatValue);
				}
				
				case locic::Constant::CHARACTER: {
					const auto characterValue = value.constant().characterValue();
					
					const auto primitiveID = value.type()->resolveAliases()->primitiveID();
					
					if (primitiveID == PrimitiveUInt8) {
						return irEmitter_.constantGenerator().getI8(characterValue);
					} else if (primitiveID == PrimitiveUnichar) {
						return irEmitter_.constantGenerator().getI32(characterValue);
					} else {
						llvm_unreachable("Unknown character literal type.");
					}
				}
				
				case locic::Constant::STRING: {
					const auto& stringValue = value.constant().stringValue();
					
					const auto arrayType =
						irEmitter_.typeGenerator().getArrayType(
							irEmitter_.typeGenerator().getI8Type(),
							stringValue.size() + 1);
					const auto constArray = irEmitter_.constantGenerator().getString(stringValue);
					
					const auto globalName = irEmitter_.module().getCString("cstring_constant");
					const auto globalArray =
						irEmitter_.module().createConstGlobal(globalName, arrayType,
						                                      llvm::GlobalValue::InternalLinkage,
						                                      constArray);
					globalArray->setAlignment(1);
					
					// Convert array to a pointer.
					return irEmitter_.emitConstInBoundsGEP2_32(arrayType,
					                                           globalArray,
					                                           0, 0);
				}
				
				default:
					llvm_unreachable("Unknown constant kind.");
			}
		}
		
		llvm::Value*
		ValueEmitter::emitAlias(const SEM::Value& value,
		                        llvm::Value* const hintResultValue) {
			const auto& alias = value.alias();
			
			SEM::TemplateVarMap assignments(alias.templateVariables().copy(),
			                                value.aliasTemplateArguments().copy());
			auto resolvedValue = alias.value().substitute(assignments);
			return emitValue(resolvedValue, hintResultValue);
		}
		
		llvm::Value*
		ValueEmitter::emitLocalVar(const SEM::Value& value) {
			const auto& var = value.localVar();
			return irEmitter_.function().getLocalVarMap().get(&var);
		}
		
		llvm::Value*
		ValueEmitter::emitReinterpretCast(const SEM::Value& value,
		                                  llvm::Value* const hintResultValue) {
			const auto sourceValue = emitValue(value.reinterpretOperand(), hintResultValue);
			
			// Currently, reinterpret_cast is only implemented for pointers.
			assert(sourceValue->getType()->isPointerTy());
			assert(genType(irEmitter_.module(), value.type())->isPointerTy());
			
			return sourceValue;
		}
		
		llvm::Value*
		ValueEmitter::emitDerefReference(const SEM::Value& value) {
			llvm::Value* const refValue = emitValue(value.derefOperand(),
			                                        /*hintResultValue=*/nullptr);
			return irEmitter_.emitMoveLoad(refValue, value.type());
		}
		
		llvm::Value*
		ValueEmitter::emitUnionDataOffset(const SEM::Value& value) {
			// Offset of union datatype data is equivalent to its
			// alignment size.
			return genAlignOf(irEmitter_.function(), value.unionDataOffsetTypeInstance()->selfType());
		}
		
		llvm::Value*
		ValueEmitter::emitMemberOffset(const SEM::Value& value) {
			// Offset of union datatype data is equivalent to its
			// alignment size.
			return genMemberOffset(irEmitter_.function(), value.memberOffsetTypeInstance()->selfType(), value.memberOffsetMemberIndex());
		}
		
		llvm::Value*
		ValueEmitter::emitTernary(const SEM::Value& value,
		                          llvm::Value* const hintResultValue) {
			const auto boolCondition = emitValue(value.ternaryCondition(),
			                                     /*hintResultValue=*/nullptr);
			const auto condValue = irEmitter_.emitBoolToI1(boolCondition);
			
			const auto ifTrueBB = irEmitter_.createBasicBlock("");
			const auto ifFalseBB = irEmitter_.createBasicBlock("");
			const auto afterCondBB = irEmitter_.createBasicBlock("");
			
			const auto currentBB = irEmitter_.builder().GetInsertBlock();
			
			irEmitter_.selectBasicBlock(ifTrueBB);
			const auto ifTrueValue = emitValue(value.ternaryIfTrue(), hintResultValue);
			const auto ifTrueTermBlock = irEmitter_.builder().GetInsertBlock();
			const auto ifTrueIsEmpty = ifTrueBB->empty();
			irEmitter_.emitBranch(afterCondBB);
			
			irEmitter_.selectBasicBlock(ifFalseBB);
			const auto ifFalseValue = emitValue(value.ternaryIfFalse(), hintResultValue);
			const auto ifFalseTermBlock = irEmitter_.builder().GetInsertBlock();
			const auto ifFalseIsEmpty = ifFalseBB->empty();
			irEmitter_.emitBranch(afterCondBB);
			
			irEmitter_.selectBasicBlock(currentBB);
			
			if (ifTrueIsEmpty && ifFalseIsEmpty) {
				// If possible, create a select instruction.
				ifTrueBB->eraseFromParent();
				ifFalseBB->eraseFromParent();
				afterCondBB->eraseFromParent();
				return irEmitter_.builder().CreateSelect(condValue, ifTrueValue, ifFalseValue);
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
			
			irEmitter_.emitCondBranch(condValue, ifTrueBranchBB, ifFalseBranchBB);
			
			irEmitter_.selectBasicBlock(afterCondBB);
			
			if (ifTrueValue->stripPointerCasts() == ifFalseValue->stripPointerCasts()) {
				assert(ifTrueValue->getType()->isPointerTy() && ifFalseValue->getType()->isPointerTy());
				return ifTrueValue;
			} else {
				const auto phiNode = irEmitter_.builder().CreatePHI(ifTrueValue->getType(), 2);
				phiNode->addIncoming(ifTrueValue, ifTrueReceiveBB);
				phiNode->addIncoming(ifFalseValue, ifFalseReceiveBB);
				return phiNode;
			}
		}
		
		llvm::Value*
		ValueEmitter::emitCast(const SEM::Value& value,
		                       llvm::Value* const hintResultValue) {
			const auto& castValue = value.castOperand();
			const auto codeValue = emitValue(castValue,
			                                 /*hintResultValue=*/nullptr);
			const auto sourceType = castValue.type()->resolveAliases();
			const auto destType = value.type()->resolveAliases();
			assert((sourceType->kind() == destType->kind()
					|| (sourceType->isPrimitive() && sourceType->getObjectType()->name().last() == "null_t")
					|| destType->isBuiltInVoid())
				   && "Types must be in the same group for cast, or "
				   "it should be a cast from null, or a cast to void");
				   
			if (destType->isBuiltInVoid()) {
				// Call destructor for the value.
				irEmitter_.emitDestructorCall(codeValue, sourceType);
				
				// All casts to void have the same outcome.
				return irEmitter_.constantGenerator().getVoidUndef();
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
						
						const auto unionValue = irEmitter_.emitAlloca(destType, hintResultValue);
						
						const auto unionDatatypePointers =
							getUnionDatatypePointers(irEmitter_.function(),
							                         destType, unionValue);
						
						// Set the variant kind value.
						irEmitter_.emitRawStore(irEmitter_.constantGenerator().getI8(variantKind),
						                        unionDatatypePointers.first);
						
						// Store the union value.
						irEmitter_.emitMoveStore(codeValue, unionDatatypePointers.second, sourceType);
						
						return irEmitter_.emitMoveLoad(unionValue, destType);
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
		
		llvm::Value*
		ValueEmitter::emitPolyCast(const SEM::Value& value) {
			const auto& castValue = value.polyCastOperand();
			const auto rawValue = emitValue(castValue,
			                                /*hintResultValue=*/nullptr);
			const auto sourceType = castValue.type();
			
			if (sourceType->isStaticRef()) {
				const auto sourceTarget = sourceType->staticRefTarget();
				
				if (sourceTarget->isInterface()) {
					// Since the vtable is a hash table and it has
					// already been generated, this is a no-op.
					return rawValue;
				}
				
				// Generate the vtable and template generator.
				const auto vtablePointer =
					genVTable(irEmitter_.module(),
					          sourceTarget->resolveAliases()->getObjectType());
				const auto templateGenerator =
					getTemplateGenerator(irEmitter_.function(),
					                     TemplateInst::Type(sourceTarget));
				
				// Build the new type info struct with these values.
				return makeTypeInfoValue(irEmitter_.function(),
				                         vtablePointer, templateGenerator);
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
				
				// Generate the vtable and template generator.
				const auto vtablePointer =
					genVTable(irEmitter_.module(),
					          sourceTarget->resolveAliases()->getObjectType());
				const auto templateGenerator =
					getTemplateGenerator(irEmitter_.function(),
					                     TemplateInst::Type(sourceTarget));
				
				// Build the new interface struct with these values.
				return makeInterfaceStructValue(irEmitter_.function(), rawValue,
				                                makeTypeInfoValue(irEmitter_.function(),
								                  vtablePointer, templateGenerator));
			} else {
				llvm_unreachable("Poly cast type must be ref or staticref.");
			}
		}
		
		llvm::Value*
		ValueEmitter::emitLval(const SEM::Value& value,
		                       llvm::Value* const hintResultValue) {
			return emitValue(value.makeLvalOperand(), hintResultValue);
		}
		
		llvm::Value*
		ValueEmitter::emitNoLval(const SEM::Value& value,
		                         llvm::Value* const hintResultValue) {
			return emitValue(value.makeNoLvalOperand(), hintResultValue);
		}
		
		llvm::Value*
		ValueEmitter::emitRef(const SEM::Value& value,
		                      llvm::Value* const hintResultValue) {
			return emitValue(value.makeRefOperand(), hintResultValue);
		}
		
		llvm::Value*
		ValueEmitter::emitNoRef(const SEM::Value& value,
		                        llvm::Value* const hintResultValue) {
			return emitValue(value.makeNoRefOperand(), hintResultValue);
		}
		
		llvm::Value*
		ValueEmitter::emitStaticRef(const SEM::Value& value,
		                            llvm::Value* const hintResultValue) {
			return emitValue(value.makeStaticRefOperand(), hintResultValue);
		}
		
		llvm::Value*
		ValueEmitter::emitNoStaticRef(const SEM::Value& value,
		                              llvm::Value* const hintResultValue) {
			return emitValue(value.makeNoStaticRefOperand(), hintResultValue);
		}
		
		llvm::Value*
		ValueEmitter::emitInternalConstruct(const SEM::Value& value,
		                                    llvm::Value* const hintResultValue) {
			const auto& parameterValues = value.internalConstructParameters();
			const auto& parameterVars = value.type()->getObjectType()->variables();
			
			const auto type = value.type();
			assert(!type->isUnion());
			
			const auto objectValue = irEmitter_.emitAlloca(type, hintResultValue);
			
			TypeInfo typeInfo(irEmitter_.module());
			
			if (type->isEnum()) {
				// An enum is just an integer; it doesn't have any variables,
				// so we just store the integer directly.
				assert(parameterVars.empty());
				irEmitter_.emitRawStore(emitValue(parameterValues.front(), objectValue),
				                        objectValue);
			} else if (typeInfo.isSizeKnownInThisModule(type)) {
				const auto objectIRType = genType(irEmitter_.module(), type);
				
				for (size_t i = 0; i < parameterValues.size(); i++) {
					const auto var = parameterVars.at(i);
					const auto llvmInsertPointer = irEmitter_.emitConstInBoundsGEP2_32(objectIRType,
					                                                                   objectValue,
					                                                                   0, i);
					const auto llvmParamValue = emitValue(parameterValues.at(i), llvmInsertPointer);
					genStoreVar(irEmitter_.function(), llvmParamValue, llvmInsertPointer, var);
				}
			} else {
				llvm::Value* offsetValue = irEmitter_.constantGenerator().getSizeTValue(0);
				
				for (size_t i = 0; i < parameterValues.size(); i++) {
					const auto var = parameterVars.at(i);
					
					// Align offset for field.
					const auto varAlign = genAlignMask(irEmitter_.function(), var->type());
					offsetValue = makeAligned(irEmitter_.function(), offsetValue,
					                          varAlign);
					
					const auto llvmInsertPointer = irEmitter_.emitInBoundsGEP(irEmitter_.typeGenerator().getI8Type(),
					                                                          objectValue, offsetValue);
					const auto llvmParamValue = emitValue(parameterValues.at(i), llvmInsertPointer);
					
					genStoreVar(irEmitter_.function(), llvmParamValue, llvmInsertPointer, var);
					
					if ((i + 1) != parameterValues.size()) {
						// If this isn't the last field, add its size for calculating
						// the offset of the next field.
						const auto varTypeSize = genSizeOf(irEmitter_.function(), var->type());
						offsetValue = irEmitter_.builder().CreateAdd(offsetValue, varTypeSize);
					}
				}
			}
			
			// Set object into live state (e.g. set gap byte to 1).
			setOuterLiveState(irEmitter_.function(), *(type->getObjectType()), objectValue);
			
			return irEmitter_.emitMoveLoad(objectValue, type);
		}
		
		llvm::Value*
		ValueEmitter::emitMemberAccess(const SEM::Value& value) {
			const auto memberIndex = value.memberAccessVar().index();
			
			const auto& dataRefValue = value.memberAccessObject();
			assert(dataRefValue.type()->isRef() && dataRefValue.type()->isBuiltInReference());
			
			const auto llvmDataRefValue = emitValue(dataRefValue,
			                                        /*hintResultValue=*/nullptr);
			return genMemberPtr(irEmitter_.function(), llvmDataRefValue,
			                    dataRefValue.type()->refTarget(), memberIndex);
		}
		
		llvm::Value*
		ValueEmitter::emitBindReference(const SEM::Value& value) {
			const auto& dataValue = value.bindReferenceOperand();
			const auto llvmDataValue = emitValue(dataValue,
			                                     /*hintResultValue=*/nullptr);
			return genValuePtr(irEmitter_.function(), llvmDataValue,
			                   dataValue.type());
		}
		
		llvm::Value*
		ValueEmitter::emitTypeRef(const SEM::Value& value) {
			const auto targetType = value.typeRefType();
			
			const auto vtablePointer = genVTable(irEmitter_.module(),
			                                     targetType->resolveAliases()->getObjectType());
			const auto templateGenerator = getTemplateGenerator(irEmitter_.function(),
			                                                    TemplateInst::Type(targetType));
			
			// Build the new type info struct with these values.
			return makeTypeInfoValue(irEmitter_.function(),
			                         vtablePointer, templateGenerator);
		}
		
		llvm::Value*
		ValueEmitter::emitCall(const SEM::Value& value,
		                       llvm::Value* const hintResultValue) {
			auto& module = irEmitter_.module();
			auto& function = irEmitter_.function();
			const auto& semCallValue = value.callValue();
			const auto& semArgumentValues = value.callParameters();
			
			if (semCallValue.type()->isBuiltInInterfaceMethod() || semCallValue.type()->isBuiltInStaticInterfaceMethod()) {
				const auto methodComponents = genVirtualMethodComponents(function, semCallValue);
				
				llvm::SmallVector<llvm::Value*, 10> llvmArgs;
				for (const auto& arg: semArgumentValues) {
					llvmArgs.push_back(emitValue(arg, /*hintResultValue=*/nullptr));
				}
				
				return module.virtualCallABI().emitCall(irEmitter_,
				                                        semCallValue.type()->asFunctionType(),
				                                        methodComponents,
				                                        llvmArgs,
				                                        hintResultValue);
			}
			
			// TODO: merge this with the call below.
			if (isTrivialFunction(module, semCallValue)) {
				return genTrivialFunctionCall(function, semCallValue, arrayRef(semArgumentValues), hintResultValue);
			}
			
			return genSEMFunctionCall(function, semCallValue, arrayRef(semArgumentValues), hintResultValue);
		}
		
		llvm::Value*
		ValueEmitter::emitFunctionRef(const SEM::Value& value) {
			const auto callInfo = genFunctionCallInfo(irEmitter_.function(),
			                                          value);
			
			if (callInfo.templateGenerator != nullptr) {
				const auto type = genType(irEmitter_.module(), value.type());
				llvm::Value* functionValue = irEmitter_.constantGenerator().getUndef(type);
				functionValue = irEmitter_.emitInsertValue(functionValue, callInfo.functionPtr, { 0 });
				functionValue = irEmitter_.emitInsertValue(functionValue, callInfo.templateGenerator, { 1 });
				return functionValue;
			} else {
				return callInfo.functionPtr;
			}
		}
		
		llvm::Value*
		ValueEmitter::emitMethodObject(const SEM::Value& value) {
			const auto callInfo = genFunctionCallInfo(irEmitter_.function(),
			                                          value);
			
			llvm::Value* functionValue = nullptr;
			
			if (callInfo.templateGenerator != nullptr) {
				const auto functionPtrType = getBasicPrimitiveType(irEmitter_.module(),
				                                                   PrimitiveTemplatedMethodFunctionPtr0);
				functionValue = irEmitter_.constantGenerator().getUndef(functionPtrType);
				functionValue = irEmitter_.emitInsertValue(functionValue, callInfo.functionPtr, { 0 });
				functionValue = irEmitter_.emitInsertValue(functionValue, callInfo.templateGenerator, { 1 });
			} else {
				functionValue = callInfo.functionPtr;
			}
			
			const auto type = genType(irEmitter_.module(), value.type());
			llvm::Value* methodValue = irEmitter_.constantGenerator().getUndef(type);
			methodValue = irEmitter_.emitInsertValue(methodValue, callInfo.contextPointer, { 0 });
			methodValue = irEmitter_.emitInsertValue(methodValue, functionValue, { 1 });
			return methodValue;
		}
		
		llvm::Value*
		ValueEmitter::emitInterfaceMethodObject(const SEM::Value& value) {
			const auto& method = value.interfaceMethodObject();
			const auto methodOwner = emitValue(value.interfaceMethodOwner(),
			                                   /*hintResultValue=*/nullptr);
			
			assert(method.kind() == SEM::Value::FUNCTIONREF);
			
			const auto interfaceFunction = method.functionRefFunction();
			const auto methodHash = CreateMethodNameHash(interfaceFunction->name().last());
			const auto methodHashValue = irEmitter_.constantGenerator().getI64(methodHash);
			return makeInterfaceMethodValue(irEmitter_.function(),
			                                methodOwner, methodHashValue);
		}
		
		llvm::Value*
		ValueEmitter::emitStaticInterfaceMethodObject(const SEM::Value& value) {
			const auto& method = value.staticInterfaceMethodObject();
			const auto typeRefPtr = emitValue(value.staticInterfaceMethodOwner(),
			                                  /*hintResultValue=*/nullptr);
			
			const auto typeRef = irEmitter_.emitRawLoad(typeRefPtr,
			                                            typeInfoType(irEmitter_.module()).second);
			
			assert(method.kind() == SEM::Value::FUNCTIONREF);
			
			const auto interfaceFunction = method.functionRefFunction();
			const auto methodHash = CreateMethodNameHash(interfaceFunction->name().last());
			const auto methodHashValue = irEmitter_.constantGenerator().getI64(methodHash);
			return makeStaticInterfaceMethodValue(irEmitter_.function(),
			                                      typeRef, methodHashValue);
		}
		
		llvm::Value*
		ValueEmitter::emitTemplateVarRef(const SEM::Value& value,
		                                 llvm::Value* const hintResultValue) {
			const auto templateArgs = irEmitter_.function().getTemplateArgs();
			const auto templateVar = value.templateVar();
			const auto index = templateVar->index();
			const unsigned extractIndexArray[] = { (unsigned) index, 0 };
			const auto valueEntry = irEmitter_.builder().CreateExtractValue(templateArgs,
			                                                                extractIndexArray);
			
			SEM::FunctionAttributes attributes(/*isVarArg=*/false,
			                                   /*isMethod=*/false,
			                                   /*isTemplated=*/false,
			                                   /*noExceptPredicate=*/SEM::Predicate::True());
			SEM::FunctionType functionType(std::move(attributes),
			                               /*returnType=*/value.type(),
			                               /*parameterTypes=*/{});
			
			FunctionCallInfo callInfo;
			callInfo.functionPtr = valueEntry;
			
			return genFunctionCall(irEmitter_.function(),
			                       functionType,
			                       callInfo,
			                       /*args=*/{},
			                       hintResultValue);
		}
		
		llvm::Value*
		ValueEmitter::emitArrayLiteral(const SEM::Value& value,
		                               llvm::Value* const hintResultValue) {
			const auto arrayPtr = irEmitter_.emitAlloca(value.type(), hintResultValue);
			
			for (size_t i = 0; i < value.arrayLiteralValues().size(); i++) {
				const auto& elementValue = value.arrayLiteralValues()[i];
				
				const auto indexValue = irEmitter_.constantGenerator().getSizeTValue(i);
				const auto elementPtr = getArrayIndex(irEmitter_,
				                                      elementValue.type(),
				                                      arrayPtr, indexValue);
				
				const auto elementIRValue = emitValue(elementValue, elementPtr);
				irEmitter_.emitMoveStore(elementIRValue,
				                         elementPtr,
				                         elementValue.type());
			}
			
			return irEmitter_.emitMoveLoad(arrayPtr,
			                               value.type());
		}
		
	}
	
}

