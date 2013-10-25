#include <assert.h>

#include <Locic/SEM.hpp>

#include <Locic/CodeGen/ConstantGenerator.hpp>
#include <Locic/CodeGen/Function.hpp>
#include <Locic/CodeGen/GenFunction.hpp>
#include <Locic/CodeGen/GenType.hpp>
#include <Locic/CodeGen/GenValue.hpp>
#include <Locic/CodeGen/GenVTable.hpp>
#include <Locic/CodeGen/Mangling.hpp>
#include <Locic/CodeGen/Memory.hpp>
#include <Locic/CodeGen/Module.hpp>
#include <Locic/CodeGen/Primitives.hpp>
#include <Locic/CodeGen/Support.hpp>
#include <Locic/CodeGen/TypeGenerator.hpp>
#include <Locic/CodeGen/VirtualCall.hpp>
#include <Locic/CodeGen/VTable.hpp>

namespace Locic {

	namespace CodeGen {
		
		static llvm::Value* makePtr(Function& function, llvm::Value* value, SEM::Type* type) {
			// TODO: check if object, since objects are always referred to be pointer?
			
			llvm::Value* lValue = genAlloca(function, type);
			llvm::Value* rValue = value;
			genStore(function, rValue, lValue, type);
			return lValue;
		}
		
		llvm::Value* genValue(Function& function, SEM::Value* value) {
			assert(value != NULL && "Cannot generate NULL value");
			
			LOG(LOG_INFO, "Generating value %s.",
				value->toString().c_str());
			
			Module& module = function.getModule();
				
			switch (value->kind()) {
				case SEM::Value::CONSTANT: {
					switch (value->constant->getType()) {
						case Locic::Constant::NULLVAL:
							return ConstantGenerator(module).getNullPointer(
									   TypeGenerator(module).getI8PtrType());
									   
						case Locic::Constant::BOOLEAN:
							return ConstantGenerator(module).getI1(value->constant->getBool());
							
						case Locic::Constant::SIGNEDINT: {
							return ConstantGenerator(module).getPrimitiveInt(
									   value->constant->getTypeName(), value->constant->getInt());
						}
						
						case Locic::Constant::UNSIGNEDINT: {
							return ConstantGenerator(module).getPrimitiveInt(
									   value->constant->getTypeName(), value->constant->getUint());
						}
						
						case Locic::Constant::FLOATINGPOINT: {
							switch (value->constant->getFloatType()) {
								case Locic::Constant::FLOAT:
									return ConstantGenerator(module).getFloat(
										value->constant->getFloat());
											   
								case Locic::Constant::DOUBLE:
									return ConstantGenerator(module).getDouble(
										value->constant->getFloat());
											   
								case Locic::Constant::LONGDOUBLE:
									assert(false && "Long double not implemented yet");
									return NULL;
									
								default:
									assert(false && "Unknown float constant type");
									return NULL;
							}
						}
						
						case Locic::Constant::STRING: {
							const std::string stringValue = value->constant->getString();
							
							switch (value->constant->getStringType()) {
								case Locic::Constant::CSTRING: {
									llvm::ArrayType* arrayType =
										TypeGenerator(module).getArrayType(
											TypeGenerator(module).getI8Type(),
											stringValue.size() + 1);
									llvm::Constant* constArray =
										ConstantGenerator(module).getString(
											stringValue.c_str());
									llvm::GlobalVariable* globalArray =
										module.createConstGlobal("cstring_constant",
												arrayType, llvm::GlobalValue::PrivateLinkage, constArray);
									globalArray->setAlignment(1);
									// Convert array to a pointer.
									return function.getBuilder().CreateConstGEP2_32(globalArray, 0, 0);
								}
								
								case Locic::Constant::LOCISTRING: {
									assert(false && "Loci string constants not yet implemented.");
									return NULL;
								}
								
								default:
									assert(false && "Unknown string constant type.");
									return NULL;
							}
						}
						
						default:
							assert(false && "Unknown constant type.");
							return NULL;
					}
				}
				
				case SEM::Value::COPY: {
					// This copies built-in types only; object
					// copies use the 'implicitCopy' method.
					assert(!value->type()->isObject());
					return genValue(function, value->copyValue.value);
				}
				
				case SEM::Value::VAR: {
					SEM::Var* var = value->varValue.var;
					
					switch (var->kind()) {
						case SEM::Var::PARAM:
						case SEM::Var::LOCAL: {
							return function.getLocalVarMap().get(var);
						}
						
						case SEM::Var::MEMBER: {
							return function.getBuilder().CreateConstInBoundsGEP2_32(
									function.getContextValue(), 0,
									module.getMemberVarMap().get(var));
						}
						
						default: {
							assert(false && "Unknown variable type in variable access.");
							return NULL;
						}
					}
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
					llvm::Value* codeValue = genValue(function, value->cast.value);
					SEM::Type* sourceType = value->cast.value->type();
					SEM::Type* destType = value->type();
					assert((sourceType->kind() == destType->kind()
							|| sourceType->isNull()
							|| destType->isVoid())
						   && "Types must be in the same group for cast, or "
						   "it should be a cast from null, or a cast to void");
						   
					LOG(LOG_INFO, "Generating cast from type %s to type %s.",
						sourceType->toString().c_str(), destType->toString().c_str());
						
					if (destType->isVoid()) {
						// All casts to void have the same outcome.
						return ConstantGenerator(module).getVoidUndef();
					}
					
					switch (sourceType->kind()) {
						case SEM::Type::VOID: {
							return codeValue;
						}
						
						case SEM::Type::NULLT: {
							switch (destType->kind()) {
								case SEM::Type::NULLT:
									return codeValue;
									
								case SEM::Type::FUNCTION:
									return function.getBuilder().CreatePointerCast(codeValue,
											genType(module, destType));
											
								case SEM::Type::OBJECT: {
									assert(false && "TODO");
									return NULL;
								}
								
								default: {
									assert(false && "Invalid cast from null.");
									return NULL;
								}
							}
						}
						
						case SEM::Type::OBJECT: {
							if (sourceType->getObjectType() == destType->getObjectType()) {
								return codeValue;
							}
							
							assert(false && "Casts between named types not implemented.");
							return NULL;
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
							return NULL;
					}
				}
				
				case SEM::Value::POLYCAST: {
					llvm::Value* rawValue = genValue(function, value->polyCast.value);
					SEM::Type* sourceType = value->polyCast.value->type();
					SEM::Type* destType = value->type();
					assert(sourceType->isReference()  && "Polycast source type must be reference.");
					assert(destType->isReference() && "Polycast dest type must be reference.");
					SEM::Type* sourceTarget = sourceType->getReferenceTarget();
					SEM::Type* destTarget = destType->getReferenceTarget();
					assert(destTarget->isInterface() && "Polycast dest target type must be interface");
					
					if (sourceTarget->isInterface()) {
						/*// Get the object pointer (of type i8*).
						llvm::Value* objectPointer = function.getBuilder().CreateExtractValue(rawValue,
														  std::vector<unsigned>(1, 0));
													 
						// Get the vtable pointer.
						llvm::Value* vtablePointer = function.getBuilder().CreateExtractValue(rawValue,
													 std::vector<unsigned>(1, 1));
													 
						// Build the new interface pointer struct with these values.
						llvm::Value* interfaceValue = llvm::UndefValue::get(genType(module, destType));
						interfaceValue = function.getBuilder().CreateInsertValue(interfaceValue, objectPointer,
										 std::vector<unsigned>(1, 0));
						interfaceValue = function.getBuilder().CreateInsertValue(interfaceValue, vtablePointer,
										 std::vector<unsigned>(1, 1));*/
						return rawValue;
					} else {
						// Cast class pointer to pointer to the opaque struct
						// representing destination interface type.
						llvm::Value* objectPointer = function.getBuilder().CreatePointerCast(rawValue,
								TypeGenerator(module).getI8PtrType());
													 
						// Create the vtable.
						llvm::Value* vtablePointer = genVTable(module, sourceTarget);
						
						// Build the new interface pointer struct with these values.
						llvm::Value* interfaceValue = llvm::UndefValue::get(genType(module, destType));
						interfaceValue = function.getBuilder().CreateInsertValue(interfaceValue, objectPointer,
										 std::vector<unsigned>(1, 0));
						interfaceValue = function.getBuilder().CreateInsertValue(interfaceValue, vtablePointer,
										 std::vector<unsigned>(1, 1));
						return interfaceValue;
					}
				}
				
				case SEM::Value::INTERNALCONSTRUCT: {
					const std::vector<SEM::Value*>& parameters = value->internalConstruct.parameters;
					llvm::Value* objectValue = genAlloca(function, value->type());
					
					// Set 'liveness indicator' to true (indicating destructor should be run).
					function.getBuilder().CreateStore(ConstantGenerator(function.getModule()).getI1(true),
						function.getBuilder().CreateConstInBoundsGEP2_32(objectValue, 0, 0));
													  
					for (size_t i = 0; i < parameters.size(); i++) {
						SEM::Value* paramValue = parameters.at(i);
						llvm::Value* llvmParamValue = genValue(function, paramValue);
						llvm::Value* llvmInsertPointer = function.getBuilder().CreateConstInBoundsGEP2_32(objectValue, 0, i + 1);
						genStore(function, llvmParamValue, llvmInsertPointer, paramValue->type());
					}
					
					return objectValue;
				}
				
				case SEM::Value::MEMBERACCESS: {
					const size_t offset = module.getMemberVarMap().get(value->memberAccess.memberVar);
					
					return function.getBuilder().CreateConstInBoundsGEP2_32(
						genValue(function, value->memberAccess.object), 0, offset);
				}
				
				case SEM::Value::FUNCTIONCALL: {
					LOG(LOG_EXCESSIVE, "Generating function call value %s.",
						value->functionCall.functionValue->toString().c_str());
						
					llvm::Value* functionValue = genValue(function, value->functionCall.functionValue);
					assert(functionValue->getType()->isPointerTy());
					llvm::Type* functionType = functionValue->getType()->getPointerElementType();
					assert(functionType->isFunctionTy());
					std::vector<llvm::Value*> parameters;
					const std::vector<SEM::Value*>& paramList = value->functionCall.parameters;
					SEM::Type* returnType = value->type();
					
					// Some values (e.g. classes) will be returned
					// by assigning to a pointer passed as the first
					// argument (this deals with the class sizes
					// potentially being unknown).
					llvm::Value* returnVarValue = NULL;
					
					if (!isTypeSizeAlwaysKnown(module, returnType)) {
						returnVarValue = genAlloca(function, returnType);
						parameters.push_back(returnVarValue);
					}
					
					for (std::size_t i = 0; i < paramList.size(); i++) {
						llvm::Value* argValue = genValue(function, paramList.at(i));
						
						// When calling var-args functions, all 'char' and
						// 'short' values must be extended to 'int' values,
						// and all 'float' values must be converted to 'double'
						// values.
						if (functionType->isFunctionVarArg()) {
							llvm::Type* argType = argValue->getType();
							const unsigned sizeInBits = argType->getPrimitiveSizeInBits();
							
							if (argType->isIntegerTy() && sizeInBits < module.getTargetInfo().getPrimitiveSize("int")) {
								// Need to extend to int.
								// TODO: this doesn't handle unsigned types; perhaps
								// this code should be moved to semantic analysis.
								argValue = function.getBuilder().CreateSExt(argValue,
										   getPrimitiveType(module, "int", std::vector<llvm::Type*>()));
							} else if (argType->isFloatingPointTy() && sizeInBits < 64) {
								// Need to extend to double.
								argValue = function.getBuilder().CreateFPExt(argValue,
										   TypeGenerator(module).getDoubleType());
							}
						}
						
						parameters.push_back(argValue);
					}
					
					llvm::Value* callReturnValue = function.getBuilder().CreateCall(functionValue, parameters);
					
					if (returnVarValue != NULL) {
						// As above, if the return value pointer is used,
						// this should be loaded (and used instead).
						return genLoad(function, returnVarValue, returnType);
					} else {
						return callReturnValue;
					}
				}
				
				case SEM::Value::FUNCTIONREF: {
					return genFunction(module, value->functionRef.parentType, value->functionRef.function);
				}
				
				case SEM::Value::METHODOBJECT: {
					llvm::Value* functionValue = genValue(function, value->methodObject.method);
					assert(functionValue != NULL && "MethodObject requires a valid function");
					
					SEM::Value* dataValue = value->methodObject.methodOwner;
					llvm::Value* llvmDataValue = genValue(function, dataValue);
					
					// Methods must have a pointer to the object, which
					// may require generating a fresh 'alloca'.
					const bool isValuePtr = dataValue->type()->isReference() ||
						!isTypeSizeAlwaysKnown(function.getModule(), dataValue->type());
					llvm::Value* dataPointer = isValuePtr ? llvmDataValue : makePtr(function, llvmDataValue, dataValue->type());
							
					assert(dataPointer != NULL && "MethodObject requires a valid data pointer");
					
					assert(value->type()->isMethod());
					
					llvm::Value* functionPtr =
						function.getBuilder().CreatePointerCast(functionValue,
								genFunctionType(module, value->type()->getMethodFunctionType(), i8PtrType())->getPointerTo(),
								"dynamic_method_function_ptr");
					
					llvm::Value* contextPtr = function.getBuilder().CreatePointerCast(dataPointer, i8PtrType(), "this_ptr_cast_to_void_ptr");
					
					llvm::Value* llvmMethodValue = ConstantGenerator(module).getUndef(genType(module, value->type()));
					llvmMethodValue = function.getBuilder().CreateInsertValue(llvmMethodValue, functionPtr, std::vector<unsigned>(1, 0));
					llvmMethodValue = function.getBuilder().CreateInsertValue(llvmMethodValue, contextPtr, std::vector<unsigned>(1, 1));
					return llvmMethodValue;
				}
				
				case SEM::Value::METHODCALL: {
					LOG(LOG_EXCESSIVE, "Generating method call value %s.",
						value->methodCall.methodValue->toString().c_str());
						
					llvm::Value* method = genValue(function, value->methodCall.methodValue);
					llvm::Value* functionValue = function.getBuilder().CreateExtractValue(method, std::vector<unsigned>(1, 0));
					llvm::Value* contextPointer = function.getBuilder().CreateExtractValue(method, std::vector<unsigned>(1, 1));
					
					std::vector<llvm::Value*> parameters;
					
					SEM::Type* returnType = value->type();
					
					// Some values (e.g. classes) will be returned
					// by assigning to a pointer passed as the first
					// argument (this deals with the class sizes
					// potentially being unknown).
					llvm::Value* returnVarValue = NULL;
					
					if (!isTypeSizeAlwaysKnown(module, returnType)) {
						returnVarValue = genAlloca(function, returnType);
						parameters.push_back(returnVarValue);
					}
					
					parameters.push_back(contextPointer);
					
					const std::vector<SEM::Value*>& paramList = value->methodCall.parameters;
					
					for (std::size_t i = 0; i < paramList.size(); i++) {
						LOG(LOG_EXCESSIVE, "Generating method call argument %s.",
							paramList.at(i)->toString().c_str());
						parameters.push_back(genValue(function, paramList.at(i)));
					}
					
					llvm::Value* callReturnValue = function.getBuilder().CreateCall(functionValue, parameters);
					
					if (returnVarValue != NULL) {
						// As above, if the return value pointer is used,
						// this should be loaded (and used instead).
						return genLoad(function, returnVarValue, returnType);
					} else {
						return callReturnValue;
					}
				}
				
				case SEM::Value::INTERFACEMETHODOBJECT: {
					SEM::Value* method = value->interfaceMethodObject.method;
					llvm::Value* methodOwner = genValue(function, value->interfaceMethodObject.methodOwner);
					
					assert(method->kind() == SEM::Value::FUNCTIONREF);
					
					SEM::Function* interfaceFunction = method->functionRef.function;
					const MethodHash methodHash = CreateMethodNameHash(interfaceFunction->name().last());
					
					llvm::Value* methodHashValue =
						ConstantGenerator(module).getI64(methodHash);
					
					llvm::Value* methodValue = ConstantGenerator(module).getUndef(
						genType(module, value->type()));
					
					methodValue = function.getBuilder().CreateInsertValue(methodValue, methodOwner, std::vector<unsigned>(1, 0));
					methodValue = function.getBuilder().CreateInsertValue(methodValue, methodHashValue, std::vector<unsigned>(1, 1));
					
					return methodValue;
				}
				
				case SEM::Value::INTERFACEMETHODCALL: {
					SEM::Value* method = value->interfaceMethodCall.methodValue;
					const std::vector<SEM::Value*>& paramList = value->interfaceMethodCall.parameters;
					
					return VirtualCall::generateCall(function, method, paramList);
				}
				
				default:
					assert(false && "Unknown value enum.");
					return NULL;
			}
		}
		
	}
	
}

