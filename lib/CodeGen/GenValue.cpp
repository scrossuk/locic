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
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Support.hpp>
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
			LOG(LOG_INFO, "Generating value %s.",
				value->toString().c_str());
			
			auto& module = function.module();
			const auto debugLoc = getDebugLocation(function, value);
			
			switch (value->kind()) {
				case SEM::Value::CONSTANT: {
					switch (value->constant->getType()) {
						case locic::Constant::NULLVAL:
							return ConstantGenerator(module).getNullPointer(
									   TypeGenerator(module).getI8PtrType());
									   
						case locic::Constant::BOOLEAN:
							return ConstantGenerator(module).getI1(value->constant->getBool());
						
						case locic::Constant::INTEGER: {
							return ConstantGenerator(module).getI64(static_cast<int64_t>(value->constant->getInteger()));
						}
						
						case locic::Constant::FLOATINGPOINT: {
							return ConstantGenerator(module).getLongDouble(value->constant->getFloat());
						}
						
						case locic::Constant::STRING: {
							const auto stringValue = value->constant->getString();
							
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
							llvm_unreachable("Unknown constant type.");
					}
				}
				
				case SEM::Value::LOCALVAR: {
					const auto var = value->localVar.var;
					return function.getLocalVarMap().get(var);
				}
				
				case SEM::Value::MEMBERVAR: {
					const auto var = value->memberVar.var;
					return function.getBuilder().CreateConstInBoundsGEP2_32(
							function.getContextValue(), 0,
							module.getMemberVarMap().get(var));
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
						   
					LOG(LOG_INFO, "Generating cast from type %s to type %s.",
						sourceType->toString().c_str(), destType->toString().c_str());
					
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
					
					// Generate the vtable.
					const auto vtablePointer = genVTable(module, sourceTarget);
						
					// Build the new interface pointer struct with these values.
					const auto interfaceValueEmpty = llvm::UndefValue::get(genType(module, destType));
					const auto interfaceValuePartial = function.getBuilder().CreateInsertValue(interfaceValueEmpty, objectPointer,
										 std::vector<unsigned>(1, 0));
					const auto interfaceValue = function.getBuilder().CreateInsertValue(interfaceValuePartial, vtablePointer,
										 std::vector<unsigned>(1, 1));
					return interfaceValue;
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
					
					return objectValue;
				}
				
				case SEM::Value::MEMBERACCESS: {
					const auto offset = module.getMemberVarMap().get(value->memberAccess.memberVar);
					
					return function.getBuilder().CreateConstInBoundsGEP2_32(
						genValue(function, value->memberAccess.object), 0, offset);
				}
				
				case SEM::Value::FUNCTIONCALL: {
					LOG(LOG_EXCESSIVE, "Generating function call value %s.",
						value->functionCall.functionValue->toString().c_str());
						
					const auto functionValue = genValue(function, value->functionCall.functionValue);
					const auto contextPointer = nullptr;
					const auto returnType = value->type();
					
					return genFunctionCall(function, functionValue, contextPointer, returnType, value->functionCall.parameters, debugLoc);
				}
				
				case SEM::Value::FUNCTIONREF: {
					return genFunction(module, value->functionRef.parentType, value->functionRef.function);
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
				
				case SEM::Value::METHODCALL: {
					LOG(LOG_EXCESSIVE, "Generating method call value %s.",
						value->methodCall.methodValue->toString().c_str());
						
					const auto method = genValue(function, value->methodCall.methodValue);
					const auto functionValue = function.getBuilder().CreateExtractValue(method, std::vector<unsigned>(1, 0));
					const auto contextPointer = function.getBuilder().CreateExtractValue(method, std::vector<unsigned>(1, 1));
					
					const auto returnType = value->type();
					
					return genFunctionCall(function, functionValue, contextPointer, returnType, value->methodCall.parameters, debugLoc);
				}
				
				case SEM::Value::INTERFACEMETHODOBJECT: {
					const auto method = value->interfaceMethodObject.method;
					const auto methodOwner = genValue(function, value->interfaceMethodObject.methodOwner);
					
					assert(method->kind() == SEM::Value::FUNCTIONREF);
					
					const auto interfaceFunction = method->functionRef.function;
					const auto methodHash = CreateMethodNameHash(interfaceFunction->name().last());
					
					const auto methodHashValue = ConstantGenerator(module).getI64(methodHash);
					
					const auto methodValueUndef = ConstantGenerator(module).getUndef(genType(module, value->type()));
					
					const auto methodValueWithOwner = function.getBuilder().CreateInsertValue(methodValueUndef, methodOwner, std::vector<unsigned>(1, 0));
					const auto methodValue = function.getBuilder().CreateInsertValue(methodValueWithOwner, methodHashValue, std::vector<unsigned>(1, 1));
					
					return methodValue;
				}
				
				case SEM::Value::INTERFACEMETHODCALL: {
					const auto method = value->interfaceMethodCall.methodValue;
					const auto& paramList = value->interfaceMethodCall.parameters;
					
					return VirtualCall::generateCall(function, method, paramList);
				}
				
				default:
					assert(false && "Unknown value enum.");
					return nullptr;
			}
		}
		
	}
	
}

