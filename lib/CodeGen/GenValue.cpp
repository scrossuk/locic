#include <assert.h>

#include <boost/optional.hpp>

#include <locic/Constant.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Exception.hpp>
#include <locic/CodeGen/Function.hpp>
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
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VirtualCall.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
		
		static llvm::Value* makePtr(Function& function, llvm::Value* value, SEM::Type* type) {
			assert(isTypeSizeAlwaysKnown(function.module(), type) && !type->isReference());
			
			const auto ptrValue = genAlloca(function, type);
			genStore(function, value, ptrValue, type);
			return ptrValue;
		}
		
		boost::optional<llvm::DebugLoc> getDebugLocation(Function& function, SEM::Value* value) {
			auto& module = function.module();
			auto& valueMap = module.debugModule().valueMap;
			const auto iterator = valueMap.find(value);
			if (iterator != valueMap.end()) {
				const auto debugSourceLocation = iterator->second.location;
				const auto debugStartPosition = debugSourceLocation.range().start();
				return boost::make_optional(llvm::DebugLoc::get(debugStartPosition.lineNumber(), debugStartPosition.column(), function.debugInfo()));
			} else {
				return boost::none;
			}
		}
		
		llvm::Value* genValue(Function& function, SEM::Value* value) {
			auto& module = function.module();
			const auto debugLoc = getDebugLocation(function, value);
			
			switch (value->kind()) {
				case SEM::Value::SELF:
				case SEM::Value::THIS: {
					return function.getContextValue();
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
							|| destType->isVoid())
						   && "Types must be in the same group for cast, or "
						   "it should be a cast from null, or a cast to void");
						   
					if (destType->isVoid()) {
						// Call destructor for the value.
						genDestructorCall(function, sourceType, codeValue);
						
						// All casts to void have the same outcome.
						return ConstantGenerator(module).getVoidUndef();
					}
					
					switch (sourceType->kind()) {
						case SEM::Type::VOID: {
							return codeValue;
						}
						
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
								
								return unionValue;
							}
							
							assert(false && "Casts between named types not implemented.");
							return nullptr;
						}
						
						case SEM::Type::REFERENCE: {
							return function.getBuilder().CreatePointerCast(codeValue, genType(module, destType));
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
					
					assert(sourceType->isReference()  && "Polycast source type must be reference.");
					assert(destType->isReference() && "Polycast dest type must be reference.");
					assert(destType->getReferenceTarget()->isInterface() && "Polycast dest target type must be interface");
					
					const auto sourceTarget = sourceType->getReferenceTarget();
					
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
					const auto vtablePointer = genVTable(module, sourceTarget->getObjectType());
					const auto templateGenerator = computeTemplateGenerator(function, sourceTarget);
					
					// Build the new interface struct with these values.
					return makeInterfaceStructValue(function, objectPointer, makeTypeInfoValue(function, vtablePointer, templateGenerator));
				}
				
				case SEM::Value::LVAL: {
					return genValue(function, value->makeLval.value);
				}
				
				case SEM::Value::REF: {
					return genValue(function, value->makeRef.value);
				}
				
				case SEM::Value::INTERNALCONSTRUCT: {
					const auto& parameterValues = value->internalConstruct.parameters;
					const auto& parameterVars = value->type()->getObjectType()->variables();
					
					const auto objectValue = genAlloca(function, value->type());
					
					for (size_t i = 0; i < parameterValues.size(); i++) {
						const auto llvmParamValue = genValue(function, parameterValues.at(i));
						const auto llvmInsertPointer = function.getBuilder().CreateConstInBoundsGEP2_32(objectValue, 0, i);
						genStoreVar(function, llvmParamValue, llvmInsertPointer, parameterVars.at(i));
					}
					
					return genLoad(function, objectValue, value->type());
				}
				
				case SEM::Value::MEMBERACCESS: {
					const auto offset = module.getMemberVarMap().get(value->memberAccess.memberVar);
					
					const auto dataValue = value->memberAccess.object;
					const auto llvmDataValue = genValue(function, dataValue);
					
					// Members must have a pointer to the object, which
					// may require generating a fresh 'alloca'.
					const bool isValuePtr = dataValue->type()->isReference() ||
						!isTypeSizeAlwaysKnown(function.module(), dataValue->type());
					const auto dataPointer = isValuePtr ? llvmDataValue : makePtr(function, llvmDataValue, dataValue->type());
					
					return function.getBuilder().CreateConstInBoundsGEP2_32(dataPointer, 0, offset);
				}
				
				case SEM::Value::FUNCTIONCALL: {
					const auto semFunctionValue = value->functionCall.functionValue;
					const auto& semArgumentValues = value->functionCall.parameters;
					
					if (semFunctionValue->type()->isInterfaceMethod()) {
						const auto methodValue = genValue(function, semFunctionValue);
						
						std::vector<llvm::Value*> llvmArgs;
						for (const auto& arg: semArgumentValues) {
							llvmArgs.push_back(genValue(function, arg));
						}
						
						const auto functionType = semFunctionValue->type()->getInterfaceMethodFunctionType();
						return VirtualCall::generateCall(function, functionType, methodValue, llvmArgs);
					}
					
					assert(semFunctionValue->type()->isFunction() || semFunctionValue->type()->isMethod());
					
					const auto callValue = genValue(function, semFunctionValue);
					
					const auto contextPointer = semFunctionValue->type()->isMethod() ?
						function.getBuilder().CreateExtractValue(callValue, { 0 }) :
						nullptr;
					const auto functionValue = semFunctionValue->type()->isMethod() ?
						function.getBuilder().CreateExtractValue(callValue, { 1 }) :
						callValue;
					const auto functionType = semFunctionValue->type()->isMethod() ?
						semFunctionValue->type()->getMethodFunctionType() : semFunctionValue->type();
					
					return genFunctionCall(function, functionValue, contextPointer, functionType, semArgumentValues, debugLoc);
				}
				
				case SEM::Value::FUNCTIONREF: {
					const auto parentType = value->functionRef.parentType;
					const auto objectType = parentType != nullptr ? parentType->getObjectType() : nullptr;
					
					const auto functionPtr = genFunction(module, objectType, value->functionRef.function);
					
					if (value->type()->isFunctionTemplatedMethod()) {
						assert(parentType != nullptr);
						llvm::Value* functionValue = ConstantGenerator(module).getUndef(genType(module, value->type()));
						functionValue = function.getBuilder().CreateInsertValue(functionValue, functionPtr, { 0 });
						functionValue = function.getBuilder().CreateInsertValue(functionValue, computeTemplateGenerator(function, parentType), { 1 });
						return functionValue;
					} else {
						return functionPtr;
					}
				}
				
				case SEM::Value::METHODOBJECT: {
					const auto functionValue = genValue(function, value->methodObject.method);
					assert(functionValue != nullptr && "MethodObject requires a valid function");
					
					const auto dataValue = value->methodObject.methodOwner;
					const auto llvmDataValue = genValue(function, dataValue);
					
					// Methods must have a pointer to the object, which
					// may require generating a fresh 'alloca'.
					const bool isValuePtr = dataValue->type()->isReference() ||
						!isTypeSizeAlwaysKnown(function.module(), dataValue->type());
					const auto dataPointer = isValuePtr ? llvmDataValue : makePtr(function, llvmDataValue, dataValue->type());
					
					assert(dataPointer != nullptr && "MethodObject requires a valid data pointer");
					
					assert(value->type()->isMethod());
					
					const auto functionPtr =
						function.getBuilder().CreatePointerCast(functionValue,
								genFunctionType(module, value->type()->getMethodFunctionType(), i8PtrType())->getPointerTo(),
								"dynamic_method_function_ptr");
					
					const auto contextPtr = function.getBuilder().CreatePointerCast(dataPointer, i8PtrType(), "this_ptr_cast_to_void_ptr");
					
					const auto methodValueUndef = ConstantGenerator(module).getUndef(genType(module, value->type()));
					const auto methodValueWithFunction = function.getBuilder().CreateInsertValue(methodValueUndef, functionPtr, std::vector<unsigned>(1, 0));
					const auto methodValue = function.getBuilder().CreateInsertValue(methodValueWithFunction, contextPtr, std::vector<unsigned>(1, 1));
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

