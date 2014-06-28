#include <assert.h>

#include <boost/optional.hpp>

#include <locic/Constant.hpp>

#include <locic/SEM.hpp>

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
			const auto debugLoc = getDebugLocation(function, value);
			
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
					return genLoad(function, refValue, value->type());
				}
				
				case SEM::Value::SIZEOF: {
					return genSizeOf(function, value->sizeOf.targetType);
				}
				
				case SEM::Value::TERNARY: {
					return function.getBuilder().CreateSelect(genValue(function, value->ternary.condition),
							genValue(function, value->ternary.ifTrue),
							genValue(function, value->ternary.ifFalse));
				}
				
				case SEM::Value::CAST: {
					const auto codeValue = genValue(function, value->cast.value);
					const auto sourceType = value->cast.value->type();
					const auto destType = value->type();
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
								
								// Set the variant kind value.
								const auto variantKindPtr = function.getBuilder().CreateConstInBoundsGEP2_32(unionValue, 0, 0);
								function.getBuilder().CreateStore(ConstantGenerator(module).getI8(variantKind), variantKindPtr);
								
								// Store the union value.
								const auto unionValuePtr = function.getBuilder().CreateConstInBoundsGEP2_32(unionValue, 0, 1);
								const auto unionValueType = genType(function.module(), sourceType);
								const auto castedUnionValuePtr = function.getBuilder().CreatePointerCast(unionValuePtr, unionValueType->getPointerTo());
								genStore(function, codeValue, castedUnionValuePtr, sourceType);
								
								return genLoad(function, unionValue, destType);
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
					const auto destType = value->type();
					(void) destType;
					
					assert(sourceType->isBuiltInReference()  && "Polycast source type must be reference.");
					assert(destType->isBuiltInReference() && "Polycast dest type must be reference.");
					assert(destType->refTarget()->isInterface() && "Polycast dest target type must be interface");
					
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
					const auto templateGenerator = getTemplateGenerator(function, sourceTarget);
					
					// Build the new interface struct with these values.
					return makeInterfaceStructValue(function, objectPointer, makeTypeInfoValue(function, vtablePointer, templateGenerator));
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
				
				case SEM::Value::INTERNALCONSTRUCT: {
					const auto& parameterValues = value->internalConstruct.parameters;
					const auto& parameterVars = value->type()->getObjectType()->variables();
					
					const auto type = value->type();
					const auto objectValue = genAlloca(function, type);
					const auto castObjectValue = function.getBuilder().CreatePointerCast(objectValue, TypeGenerator(module).getI8PtrType());
					
					for (size_t i = 0; i < parameterValues.size(); i++) {
						const auto llvmParamValue = genValue(function, parameterValues.at(i));
						const auto memberOffset = genMemberOffset(function, type, i);
						const auto llvmInsertPointer = function.getBuilder().CreateInBoundsGEP(castObjectValue, memberOffset);
						genStoreVar(function, llvmParamValue, llvmInsertPointer, parameterVars.at(i));
					}
					
					return genLoad(function, objectValue, type);
				}
				
				case SEM::Value::MEMBERACCESS: {
					const auto memberIndex = module.getMemberVarMap().get(value->memberAccess.memberVar);
					
					const auto dataValue = value->memberAccess.object;
					const auto llvmDataValue = genValue(function, dataValue);
					
					// Members must have a pointer to the object, which
					// may require generating a fresh 'alloca'.
					const bool isValuePtr = dataValue->type()->isBuiltInReference() ||
						!isTypeSizeAlwaysKnown(function.module(), dataValue->type());
					const auto dataPointer = isValuePtr ? llvmDataValue : genValuePtr(function, llvmDataValue, dataValue->type());
					const auto castDataPointer = function.getBuilder().CreatePointerCast(dataPointer, TypeGenerator(module).getI8PtrType());
					
					const auto type = dataValue->type()->isBuiltInReference() ? dataValue->type()->refTarget() : dataValue->type();
					const auto memberOffset = genMemberOffset(function, type, memberIndex);
					
					return function.getBuilder().CreateInBoundsGEP(castDataPointer, memberOffset);
				}
				
				case SEM::Value::REFVALUE: {
					const auto dataValue = value->refValue.value;
					const auto llvmDataValue = genValue(function, dataValue);
					
					const auto llvmPtrValue = genValuePtr(function, llvmDataValue, dataValue->type());
					
					// Call destructor for the object at the end of the current scope.
					scheduleDestructorCall(function, dataValue->type(), llvmPtrValue);
					
					return llvmPtrValue;
				}
				
				case SEM::Value::FUNCTIONCALL: {
					const auto semFunctionValue = value->functionCall.functionValue;
					const auto& semArgumentValues = value->functionCall.parameters;
					
					if (semFunctionValue->type()->isInterfaceMethod()) {
						const auto methodValue = genValue(function, semFunctionValue);
						
						llvm::SmallVector<llvm::Value*, 10> llvmArgs;
						for (const auto& arg: semArgumentValues) {
							llvmArgs.push_back(genValue(function, arg));
						}
						
						const auto functionType = semFunctionValue->type()->getInterfaceMethodFunctionType();
						return VirtualCall::generateCall(function, functionType, methodValue, llvmArgs);
					}
					
					assert(semFunctionValue->type()->isFunction() || semFunctionValue->type()->isMethod());
					
					if (isTrivialFunction(module, semFunctionValue)) {
						llvm::SmallVector<llvm::Value*, 10> llvmArgs;
						for (const auto& arg: semArgumentValues) {
							llvmArgs.push_back(genValue(function, arg));
						}
						return genTrivialFunctionCall(function, semFunctionValue, false, nullptr, llvmArgs);
					}
					
					const auto callInfo = genFunctionCallInfo(function, semFunctionValue);
					const auto functionType = semFunctionValue->type()->isMethod() ?
						semFunctionValue->type()->getMethodFunctionType() : semFunctionValue->type();
					
					return genFunctionCall(function, callInfo, functionType, semArgumentValues, debugLoc);
				}
				
				case SEM::Value::FUNCTIONREF: {
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
					// TODO: use genFunctionCallInfo here.
					
					const auto functionValue = genValue(function, value->methodObject.method);
					assert(functionValue != nullptr && "MethodObject requires a valid function");
					
					const auto dataValue = value->methodObject.methodOwner;
					const auto llvmDataValue = genValue(function, dataValue);
					
					// Methods must have a pointer to the object, which
					// may require generating a fresh 'alloca'.
					const bool isValuePtr = dataValue->type()->isBuiltInReference() ||
						!isTypeSizeAlwaysKnown(function.module(), dataValue->type());
					const auto dataPointer = isValuePtr ? llvmDataValue : genValuePtr(function, llvmDataValue, dataValue->type());
					
					assert(dataPointer != nullptr && "MethodObject requires a valid data pointer");
					
					assert(value->type()->isMethod());
					
					const auto contextPtr = function.getBuilder().CreatePointerCast(dataPointer, TypeGenerator(module).getI8PtrType(), "this_ptr_cast_to_void_ptr");
					
					llvm::Value* methodValue = ConstantGenerator(module).getUndef(genType(module, value->type()));
					methodValue = function.getBuilder().CreateInsertValue(methodValue, contextPtr, { 0 });
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
				
				default:
					llvm_unreachable("Unknown value enum.");
			}
		}
		
	}
	
}

