#include <assert.h>

#include <boost/optional.hpp>

#include <locic/Constant.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Exception.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/GenVTable.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SizeOf.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>
#include <locic/CodeGen/VirtualCall.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Value* genValue(Function& function, SEM::Value* value) {
			auto& module = function.module();
			
			switch (value->kind()) {
				case SEM::Value::SELF: {
					return function.getContextValue(value->type()->refTarget()->getObjectType());
				}
				case SEM::Value::THIS: {
					return function.getContextValue(value->type()->templateArguments().at(0)->getObjectType());
				}
				case SEM::Value::CONSTANT: {
					switch (value->constant->kind()) {
						case locic::Constant::NULLVAL:
							return ConstantGenerator(module).getNullPointer(TypeGenerator(module).getI8PtrType());
									   
						case locic::Constant::BOOLEAN:
							return ConstantGenerator(module).getI1(value->constant->boolValue());
						
						case locic::Constant::INTEGER: {
							assert(value->type()->isObject());
							const auto integerValue = value->constant->integerValue();
							return ConstantGenerator(module).getPrimitiveInt(value->type()->getObjectType()->name().last(), integerValue);
						}
						
						case locic::Constant::FLOATINGPOINT: {
							assert(value->type()->isObject());
							const auto floatValue = value->constant->floatValue();
							return ConstantGenerator(module).getPrimitiveFloat(value->type()->getObjectType()->name().last(), floatValue);
						}
						
						case locic::Constant::CHARACTER: {
							const auto characterValue = value->constant->characterValue();
							
							const auto typeName = value->type()->resolveAliases()->getObjectType()->name().last();
							
							if (typeName == "uint8_t") {
								return ConstantGenerator(module).getI8(characterValue);
							} else {
								llvm_unreachable("Unknown character literal type.");
							}
						}
						
						case locic::Constant::STRING: {
							const auto stringValue = value->constant->stringValue();
							
							const auto arrayType =
								TypeGenerator(module).getArrayType(
									TypeGenerator(module).getI8Type(),
										stringValue.size() + 1);
							const auto constArray = ConstantGenerator(module).getString(stringValue.c_str());
							const auto globalArray =
								module.createConstGlobal("cstring_constant",
										arrayType, llvm::GlobalValue::PrivateLinkage, constArray);
							globalArray->setAlignment(1);
							
							// Convert array to a pointer.
							return function.getBuilder().CreateConstGEP2_32(globalArray, 0, 0);
						}
						
						default:
							llvm_unreachable("Unknown constant kind.");
					}
				}
				
				case SEM::Value::LOCALVAR: {
					const auto var = value->localVar.var;
					return function.getLocalVarMap().get(var);
				}
				
				case SEM::Value::REINTERPRET: {
					const auto sourceValue = genValue(function, value->reinterpretValue.value);
					const auto targetType = genType(module, value->type());
					
					// Currently, reinterpret_cast is only implemented for pointers.
					return function.getBuilder().CreatePointerCast(sourceValue, targetType);
				}
				
				case SEM::Value::DEREF_REFERENCE: {
					llvm::Value* refValue = genValue(function, value->derefReference.value);
					return genMoveLoad(function, refValue, value->type());
				}
				
				case SEM::Value::UNIONTAG: {
					const auto unionType = value->unionTag.operand->type();
					const auto unionValue = genValue(function, value->unionTag.operand);
					
					llvm::Value* unionValuePtr = nullptr;
					
					if (!unionValue->getType()->isPointerTy()) {
						// TODO: remove this unnecessary alloc.
						unionValuePtr = genAlloca(function, unionType);
						genMoveStore(function, unionValue, unionValuePtr, unionType);
					} else {
						unionValuePtr = unionValue;
					}
					
					const auto unionDatatypePointers = getUnionDatatypePointers(function, unionType, unionValuePtr);
					
					return function.getBuilder().CreateLoad(unionDatatypePointers.first);
				}
				
				case SEM::Value::SIZEOF: {
					return genSizeOf(function, value->sizeOf.targetType);
				}
				
				case SEM::Value::UNIONDATAOFFSET: {
					// Offset of union datatype data is equivalent to its
					// alignment size.
					return genAlignOf(function, value->unionDataOffset.typeInstance->selfType());
				}
				
				case SEM::Value::MEMBEROFFSET: {
					// Offset of union datatype data is equivalent to its
					// alignment size.
					return genMemberOffset(function, value->memberOffset.typeInstance->selfType(), value->memberOffset.memberIndex);
				}
				
				case SEM::Value::TERNARY: {
					const auto condValue = genValue(function, value->ternary.condition);
					
					const auto ifTrueBB = function.createBasicBlock("");
					const auto ifFalseBB = function.createBasicBlock("");
					const auto afterCondBB = function.createBasicBlock("");
					
					const auto currentBB = function.getBuilder().GetInsertBlock();
					
					function.selectBasicBlock(ifTrueBB);
					const auto ifTrueValue = genValue(function, value->ternary.ifTrue);
					const auto ifTrueTermBlock = function.getBuilder().GetInsertBlock();
					const auto ifTrueIsEmpty = ifTrueBB->empty();
					function.getBuilder().CreateBr(afterCondBB);
					
					function.selectBasicBlock(ifFalseBB);
					const auto ifFalseValue = genValue(function, value->ternary.ifFalse);
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
					
					const auto phiNode = function.getBuilder().CreatePHI(ifTrueValue->getType(), 2);
					phiNode->addIncoming(ifTrueValue, ifTrueReceiveBB);
					phiNode->addIncoming(ifFalseValue, ifFalseReceiveBB);
					return phiNode;
				}
				
				case SEM::Value::CAST: {
					const auto codeValue = genValue(function, value->cast.value);
					const auto sourceType = value->cast.value->type()->resolveAliases();
					const auto destType = value->type()->resolveAliases();
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
							
							if (sourceType->isPrimitive() && sourceType->getObjectType()->name().last() == "null_t" && destType->isFunction()) {
								return function.getBuilder().CreatePointerCast(codeValue,
									genType(module, destType));
							}
							
							if (sourceType->isDatatype() && destType->isUnionDatatype()) {
								uint8_t variantKind = 0;
								for (auto variantTypeInstance: destType->getObjectType()->variants()) {
									if (variantTypeInstance == sourceType->getObjectType()) break;
									variantKind++;
								}
								
								const auto unionValue = genAlloca(function, destType);
								
								const auto unionDatatypePointers = getUnionDatatypePointers(function, destType, unionValue);
								
								// Set the variant kind value.
								function.getBuilder().CreateStore(ConstantGenerator(module).getI8(variantKind), unionDatatypePointers.first);
								
								// Store the union value.
								const auto unionValueType = genType(function.module(), sourceType);
								const auto castedUnionValuePtr = function.getBuilder().CreatePointerCast(unionDatatypePointers.second, unionValueType->getPointerTo());
								genMoveStore(function, codeValue, castedUnionValuePtr, sourceType);
								
								return genMoveLoad(function, unionValue, destType);
							}
							
							assert(false && "Casts between named types not implemented.");
							return nullptr;
						}
						
						case SEM::Type::FUNCTION: {
							return codeValue;
						}
						
						case SEM::Type::METHOD: {
							return codeValue;
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
					const auto rawValue = genValue(function, value->polyCast.value);
					const auto sourceType = value->polyCast.value->type();
					
					if (sourceType->isStaticRef()) {
						const auto sourceTarget = sourceType->staticRefTarget();
						
						if (sourceTarget->isInterface()) {
							// Since the vtable is a hash table and it has
							// already been generated, this is a no-op.
							return rawValue;
						}
						
						// Generate the vtable and template generator.
						const auto vtablePointer = genVTable(module, sourceTarget);
						const auto templateGenerator = getTemplateGenerator(function, TemplateInst::Type(sourceTarget));
						
						// Build the new type info struct with these values.
						return makeTypeInfoValue(function, vtablePointer, templateGenerator);
					} else if (sourceType->isRef()) {
						assert(sourceType->isBuiltInReference()  && "Polycast source type must be reference.");
						assert(value->type()->isBuiltInReference() && "Polycast dest type must be reference.");
						assert(value->type()->refTarget()->isInterface() && "Polycast dest target type must be interface");
						
						const auto sourceTarget = sourceType->refTarget();
						
						if (sourceTarget->isInterface()) {
							// Since the vtable is a hash table and it has
							// already been generated, this is a no-op.
							return rawValue;
						}
						
						// Cast class pointer to pointer to the opaque struct
						// representing destination interface type.
						const auto objectPointer = function.getBuilder().CreatePointerCast(rawValue,
									TypeGenerator(module).getI8PtrType());
						
						// Generate the vtable and template generator.
						const auto vtablePointer = genVTable(module, sourceTarget);
						const auto templateGenerator = getTemplateGenerator(function, TemplateInst::Type(sourceTarget));
						
						// Build the new interface struct with these values.
						return makeInterfaceStructValue(function, objectPointer, makeTypeInfoValue(function, vtablePointer, templateGenerator));
					} else {
						llvm_unreachable("Poly cast type must be ref or staticref.");
					}
				}
				
				case SEM::Value::LVAL: {
					return genValue(function, value->makeLval.value);
				}
				
				case SEM::Value::NOLVAL: {
					return genValue(function, value->makeNoLval.value);
				}
				
				case SEM::Value::REF: {
					return genValue(function, value->makeRef.value);
				}
				
				case SEM::Value::NOREF: {
					return genValue(function, value->makeNoRef.value);
				}
				
				case SEM::Value::STATICREF: {
					return genValue(function, value->makeStaticRef.value);
				}
				
				case SEM::Value::NOSTATICREF: {
					return genValue(function, value->makeNoStaticRef.value);
				}
				
				case SEM::Value::INTERNALCONSTRUCT: {
					const auto& parameterValues = value->internalConstruct.parameters;
					const auto& parameterVars = value->type()->getObjectType()->variables();
					
					const auto type = value->type();
					const auto objectValue = genAlloca(function, type);
					
					if (isTypeSizeKnownInThisModule(module, type)) {
						for (size_t i = 0; i < parameterValues.size(); i++) {
							const auto var = parameterVars.at(i);
							const auto llvmParamValue = genValue(function, parameterValues.at(i));
							const auto llvmInsertPointer = function.getBuilder().CreateConstInBoundsGEP2_32(objectValue, 0, i);
							genStoreVar(function, llvmParamValue, llvmInsertPointer, var);
						}
					} else {
						const auto castObjectValue = function.getBuilder().CreatePointerCast(objectValue, TypeGenerator(module).getI8PtrType());
						llvm::Value* offsetValue = ConstantGenerator(module).getSizeTValue(0);
						
						for (size_t i = 0; i < parameterValues.size(); i++) {
							const auto var = parameterVars.at(i);
							const auto llvmParamValue = genValue(function, parameterValues.at(i));
							
							// Align offset for field.
							offsetValue = makeAligned(function, offsetValue, genAlignMask(function, var->type()));
							
							const auto llvmInsertPointer = function.getBuilder().CreateInBoundsGEP(castObjectValue, offsetValue);
							const auto castInsertPointer = function.getBuilder().CreatePointerCast(llvmInsertPointer, genPointerType(module, var->type()));
							genStoreVar(function, llvmParamValue, castInsertPointer, var);
							
							if ((i + 1) != parameterValues.size()) {
								// If this isn't the last field, add its size for calculating
								// the offset of the next field.
								offsetValue = function.getBuilder().CreateAdd(offsetValue, genSizeOf(function, var->type()));
							}
						}
					}
					
					return genMoveLoad(function, objectValue, type);
				}
				
				case SEM::Value::MEMBERACCESS: {
					const auto memberIndex = module.getMemberVarMap().get(value->memberAccess.memberVar);
					
					const auto dataValue = value->memberAccess.object;
					const auto llvmDataValue = genValue(function, dataValue);
					
					// Members must have a pointer to the object, which
					// may require generating a fresh 'alloca'.
					const auto dataPointer = genValuePtr(function, llvmDataValue, dataValue->type());
					
					const auto type = dataValue->type()->isBuiltInReference() ? dataValue->type()->refTarget() : dataValue->type();
					return genMemberPtr(function, dataPointer, type, memberIndex);
				}
				
				case SEM::Value::REFVALUE: {
					const auto dataValue = value->refValue.value;
					const auto llvmDataValue = genValue(function, dataValue);
					return genValuePtr(function, llvmDataValue, dataValue->type());
				}
				
				case SEM::Value::TYPEREF: {
					const auto targetType = value->typeRef.targetType;
					
					const auto vtablePointer = genVTable(module, targetType);
					const auto templateGenerator = getTemplateGenerator(function, TemplateInst::Type(targetType));
					
					// Build the new type info struct with these values.
					return makeTypeInfoValue(function, vtablePointer, templateGenerator);
				}
				
				case SEM::Value::FUNCTIONCALL: {
					const auto semFunctionValue = value->functionCall.functionValue;
					const auto& semArgumentValues = value->functionCall.parameters;
					
					if (semFunctionValue->type()->isInterfaceMethod() || semFunctionValue->type()->isStaticInterfaceMethod()) {
						const auto methodValue = genValue(function, semFunctionValue);
						
						llvm::SmallVector<llvm::Value*, 10> llvmArgs;
						for (const auto& arg: semArgumentValues) {
							llvmArgs.push_back(genValue(function, arg));
						}
						
						return VirtualCall::generateCall(function, semFunctionValue->type(), methodValue, llvmArgs);
					}
					
					assert(semFunctionValue->type()->isFunction() || semFunctionValue->type()->isMethod());
					
					if (isTrivialFunction(module, semFunctionValue)) {
						return genTrivialFunctionCall(function, semFunctionValue, semArgumentValues);
					}
					
					const auto callInfo = genFunctionCallInfo(function, semFunctionValue);
					const auto functionType = semFunctionValue->type()->isMethod() ?
						semFunctionValue->type()->getMethodFunctionType() : semFunctionValue->type();
					
					const auto debugLoc = getDebugLocation(function, value);
					return genFunctionCall(function, callInfo, functionType, semArgumentValues, debugLoc);
				}
				
				case SEM::Value::FUNCTIONREF:
				case SEM::Value::TEMPLATEFUNCTIONREF: {
					const auto callInfo = genFunctionCallInfo(function, value);
					
					if (callInfo.templateGenerator != nullptr) {
						llvm::Value* functionValue = ConstantGenerator(module).getUndef(genType(module, value->type()));
						functionValue = function.getBuilder().CreateInsertValue(functionValue, callInfo.functionPtr, { 0 });
						functionValue = function.getBuilder().CreateInsertValue(functionValue, callInfo.templateGenerator, { 1 });
						return functionValue;
					} else {
						return callInfo.functionPtr;
					}
				}
				
				case SEM::Value::METHODOBJECT: {
					const auto callInfo = genFunctionCallInfo(function, value);
					
					llvm::Value* functionValue = nullptr;
					
					if (callInfo.templateGenerator != nullptr) {
						functionValue = ConstantGenerator(module).getUndef(genType(module, value->type()));
						functionValue = function.getBuilder().CreateInsertValue(functionValue, callInfo.functionPtr, { 0 });
						functionValue = function.getBuilder().CreateInsertValue(functionValue, callInfo.templateGenerator, { 1 });
					} else {
						functionValue = callInfo.functionPtr;
					}
					
					llvm::Value* methodValue = ConstantGenerator(module).getUndef(genType(module, value->type()));
					methodValue = function.getBuilder().CreateInsertValue(methodValue, callInfo.contextPointer, { 0 });
					methodValue = function.getBuilder().CreateInsertValue(methodValue, functionValue, { 1 });
					return methodValue;
				}
				
				case SEM::Value::INTERFACEMETHODOBJECT: {
					const auto method = value->interfaceMethodObject.method;
					const auto methodOwner = genValue(function, value->interfaceMethodObject.methodOwner);
					
					assert(method->kind() == SEM::Value::FUNCTIONREF);
					
					const auto interfaceFunction = method->functionRef.function;
					const auto methodHash = CreateMethodNameHash(interfaceFunction->name().last());
					const auto methodHashValue = ConstantGenerator(module).getI64(methodHash);
					return makeInterfaceMethodValue(function, methodOwner, methodHashValue);
				}
				
				case SEM::Value::STATICINTERFACEMETHODOBJECT: {
					const auto method = value->staticInterfaceMethodObject.method;
					const auto typeRef = genValue(function, value->staticInterfaceMethodObject.typeRef);
					
					assert(method->kind() == SEM::Value::FUNCTIONREF);
					
					const auto interfaceFunction = method->functionRef.function;
					const auto methodHash = CreateMethodNameHash(interfaceFunction->name().last());
					const auto methodHashValue = ConstantGenerator(module).getI64(methodHash);
					return makeStaticInterfaceMethodValue(function, typeRef, methodHashValue);
				}
				
				default:
					llvm_unreachable("Unknown value enum.");
			}
		}
		
	}
	
}

