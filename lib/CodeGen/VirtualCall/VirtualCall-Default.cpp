#include <vector>

#include <llvm-abi/Type.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Move.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/SEMFunctionGenerator.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/VirtualCall.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
	
		namespace VirtualCall {
		
			ArgInfo getStubArgInfo(Module& module) {
				const bool hasReturnVarArgument = true;
				const bool hasTemplateGenerator = true;
				const bool hasContextArgument = true;
				const bool isVarArg = false;
				
				TypeGenerator typeGen(module);
				
				llvm::SmallVector<TypePair, 2> arguments;
				
				// Hash value type.
				arguments.push_back(std::make_pair(llvm_abi::Type::Integer(module.abiContext(), llvm_abi::Int64), typeGen.getI64Type()));
				
				// Arguments struct pointer type.
				arguments.push_back(std::make_pair(llvm_abi::Type::Pointer(module.abiContext()), typeGen.getPtrType()));
				
				return ArgInfo(module, hasReturnVarArgument, hasTemplateGenerator, hasContextArgument, isVarArg,
							   voidTypePair(module), arguments);
			}
			
			llvm::AttributeSet conflictResolutionStubAttributes(Module& module) {
				const auto iterator = module.attributeMap().find(AttributeVirtualCallStub);
				
				if (iterator != module.attributeMap().end()) {
					return iterator->second;
				}
				
				auto& context = module.getLLVMContext();
				
				auto attributes = llvm::AttributeSet();
				
				// Always inline stubs.
				attributes = attributes.addAttribute(context, llvm::AttributeSet::FunctionIndex, llvm::Attribute::AlwaysInline);
				
				// Return value pointer attributes.
				attributes = attributes.addAttribute(context, 1, llvm::Attribute::StructRet);
				attributes = attributes.addAttribute(context, 1, llvm::Attribute::NoAlias);
				attributes = attributes.addAttribute(context, 1, llvm::Attribute::NoCapture);
				
				// Arguments struct pointer attributes.
				attributes = attributes.addAttribute(context, 5, llvm::Attribute::NoAlias);
				attributes = attributes.addAttribute(context, 5, llvm::Attribute::NoCapture);
				
				module.attributeMap().insert(std::make_pair(AttributeVirtualCallStub, attributes));
				
				return attributes;
			}
			
			llvm::Value* makeArgsStruct(Function& function, llvm::ArrayRef<const SEM::Type*> argTypes, llvm::ArrayRef<llvm::Value*> args) {
				assert(argTypes.size() == args.size());
				
				auto& module = function.module();
				const auto i8PtrType = TypeGenerator(module).getPtrType();
				
				if (args.empty()) {
					// Don't allocate struct when it's not needed.
					return ConstantGenerator(module).getNullPointer(i8PtrType);
				}
				
				llvm::SmallVector<llvm::Type*, 10> llvmArgTypes(args.size(), i8PtrType);
				const auto argsStructType = TypeGenerator(function.module()).getStructType(llvmArgTypes);
				
				const auto argsStructPtr = function.getEntryBuilder().CreateAlloca(argsStructType);
				
				IREmitter irEmitter(function);
				
				for (size_t offset = 0; offset < args.size(); offset++) {
					const auto argPtr = irEmitter.emitConstInBoundsGEP2_32(argsStructType,
					                                                       argsStructPtr,
					                                                       0, offset);
											
					if (canPassByValue(module, argTypes[offset])) {
						const auto argAlloca = function.getEntryBuilder().CreateAlloca(args[offset]->getType());
						irEmitter.emitRawStore(args[offset], argAlloca);
						irEmitter.emitRawStore(argAlloca, argPtr);
					} else {
						irEmitter.emitRawStore(args[offset], argPtr);
					}
				}
				
				return argsStructPtr;
			}
			
			void generateCallWithReturnVar(Function& function, const SEM::FunctionType functionType, llvm::Value* returnVarPointer,
					const VirtualMethodComponents methodComponents, llvm::ArrayRef<llvm::Value*> args) {
				auto& builder = function.getBuilder();
				auto& module = function.module();
				
				IREmitter irEmitter(function);
				
				// Calculate the slot for the virtual call.
				ConstantGenerator constantGen(function.module());
				const auto vtableSizeValue = constantGen.getI64(VTABLE_SIZE);
				
				const auto vtableOffsetValue = builder.CreateURem(methodComponents.hashValue, vtableSizeValue, "vtableOffset");
				const auto castVTableOffsetValue = builder.CreateTrunc(vtableOffsetValue, TypeGenerator(module).getI32Type());
				
				// Get a pointer to the slot.
				llvm::SmallVector<llvm::Value*, 3> vtableEntryGEP;
				vtableEntryGEP.push_back(constantGen.getI32(0));
				vtableEntryGEP.push_back(constantGen.getI32(4));
				vtableEntryGEP.push_back(castVTableOffsetValue);
				
				const auto vtableEntryPointer = irEmitter.emitInBoundsGEP(vtableType(module),
				                                                          methodComponents.object.typeInfo.vtablePointer,
				                                                          vtableEntryGEP);
				
				// Load the slot.
				const auto methodFunctionPointer = irEmitter.emitRawLoad(vtableEntryPointer,
				                                                         irEmitter.typeGenerator().getPtrType());
				
				const auto argInfo = getStubArgInfo(function.module());
				
				// Put together the arguments.
				llvm::SmallVector<llvm::Value*, 5> parameters;
				
				// Pass in the return var pointer.
				parameters.push_back(returnVarPointer);
				
				// Pass in the template generator.
				parameters.push_back(methodComponents.object.typeInfo.templateGenerator);
				
				// Pass in the object pointer.
				parameters.push_back(methodComponents.object.objectPointer);
				
				// Pass in the method hash value.
				parameters.push_back(methodComponents.hashValue);
				
				// Store all the arguments into a struct on the stack,
				// and pass the pointer to the stub.
				const auto argsStructPtr = makeArgsStruct(function, arrayRef(functionType.parameterTypes()), args);
				parameters.push_back(argsStructPtr);
				
				// Call the stub function.
				(void) genRawFunctionCall(function, argInfo, methodFunctionPointer, parameters);
			}
			
			llvm::Value* generateCall(Function& function, const SEM::FunctionType functionType, const VirtualMethodComponents methodComponents,
					llvm::ArrayRef<llvm::Value*> args, llvm::Value* const hintResultValue) {
				const auto returnType = functionType.returnType();
				const bool hasReturnVar = !returnType->isBuiltInVoid();
				
				ConstantGenerator constGen(function.module());
				const auto i8PtrType = TypeGenerator(function.module()).getPtrType();
				
				// If the return type isn't void, allocate space on the stack for the return value.
				const auto returnVarValue = hasReturnVar ?
					genAlloca(function, returnType, hintResultValue) :
					constGen.getNullPointer(i8PtrType);
				
				generateCallWithReturnVar(function, functionType, returnVarValue, methodComponents, args);
				
				// If the return type isn't void, load the return value from the stack.
				return hasReturnVar ? genMoveLoad(function, returnVarValue, returnType) : constGen.getVoidUndef();
			}
			
			llvm::Value* generateCountFnCall(Function& function, llvm::Value* typeInfoValue, CountFnKind kind) {
				auto& module = function.module();
				auto& builder = function.getBuilder();
				
				IREmitter irEmitter(function);
				
				// Extract vtable and template generator.
				const auto vtablePointer = builder.CreateExtractValue(typeInfoValue, { 0 }, "vtable");
				const auto templateGeneratorValue = builder.CreateExtractValue(typeInfoValue, { 1 }, "templateGenerator");
				
				// Get a pointer to the slot.
				ConstantGenerator constGen(module);
				llvm::SmallVector<llvm::Value*, 2> vtableEntryGEP;
				vtableEntryGEP.push_back(constGen.getI32(0));
				vtableEntryGEP.push_back(constGen.getI32(kind == ALIGNOF ? 2 : 3));
				
				const auto vtableEntryPointer = irEmitter.emitInBoundsGEP(vtableType(module),
				                                                          vtablePointer,
				                                                          vtableEntryGEP);
				
				const auto argInfo = ArgInfo::TemplateOnly(module, sizeTypePair(module)).withNoMemoryAccess().withNoExcept();
				
				// Load the slot.
				const auto methodFunctionPointer = irEmitter.emitRawLoad(vtableEntryPointer,
				                                                         irEmitter.typeGenerator().getPtrType());
				
				return genRawFunctionCall(function, argInfo, methodFunctionPointer, { templateGeneratorValue });
			}
			
			ArgInfo virtualMoveArgInfo(Module& module) {
				const TypePair types[] = { pointerTypePair(module), sizeTypePair(module) };
				return ArgInfo::VoidTemplateAndContextWithArgs(module, types).withNoExcept();
			}
			
			void generateMoveCall(Function& function, llvm::Value* typeInfoValue, llvm::Value* sourceValue,
					llvm::Value* destValue, llvm::Value* positionValue) {
				auto& module = function.module();
				auto& builder = function.getBuilder();
				
				IREmitter irEmitter(function);
				
				// Extract vtable and template generator.
				const auto vtablePointer = builder.CreateExtractValue(typeInfoValue, { 0 }, "vtable");
				const auto templateGeneratorValue = builder.CreateExtractValue(typeInfoValue, { 1 }, "templateGenerator");
				
				// Get a pointer to the slot.
				ConstantGenerator constGen(module);
				llvm::SmallVector<llvm::Value*, 2> vtableEntryGEP;
				vtableEntryGEP.push_back(constGen.getI32(0));
				vtableEntryGEP.push_back(constGen.getI32(0));
				
				const auto vtableEntryPointer = irEmitter.emitInBoundsGEP(vtableType(module),
				                                                          vtablePointer,
				                                                          vtableEntryGEP);
				
				const auto argInfo = virtualMoveArgInfo(module);
				
				// Load the slot.
				const auto methodFunctionPointer = irEmitter.emitRawLoad(vtableEntryPointer,
				                                                         irEmitter.typeGenerator().getPtrType());
				
				llvm::Value* const args[] = { templateGeneratorValue, sourceValue, destValue, positionValue };
				(void) genRawFunctionCall(function, argInfo, methodFunctionPointer, args);
			}
			
			ArgInfo virtualDestructorArgInfo(Module& module) {
				return ArgInfo::VoidTemplateAndContext(module).withNoExcept();
			}
			
			void generateDestructorCall(Function& function, llvm::Value* typeInfoValue, llvm::Value* objectValue) {
				auto& module = function.module();
				auto& builder = function.getBuilder();
				
				IREmitter irEmitter(function);
				
				// Extract vtable and template generator.
				const auto vtablePointer = builder.CreateExtractValue(typeInfoValue, { 0 }, "vtable");
				const auto templateGeneratorValue = builder.CreateExtractValue(typeInfoValue, { 1 }, "templateGenerator");
				
				// Get a pointer to the slot.
				ConstantGenerator constGen(module);
				llvm::SmallVector<llvm::Value*, 2> vtableEntryGEP;
				vtableEntryGEP.push_back(constGen.getI32(0));
				vtableEntryGEP.push_back(constGen.getI32(1));
				
				const auto vtableEntryPointer = irEmitter.emitInBoundsGEP(vtableType(module),
				                                                          vtablePointer,
				                                                          vtableEntryGEP);
				
				const auto argInfo = virtualDestructorArgInfo(module);
				
				// Load the slot.
				const auto methodFunctionPointer = irEmitter.emitRawLoad(vtableEntryPointer,
				                                                         irEmitter.typeGenerator().getPtrType());
				
				llvm::Value* const args[] = { templateGeneratorValue, objectValue };
				(void) genRawFunctionCall(function, argInfo, methodFunctionPointer, args);
			}
			
			llvm::Constant* generateVTableSlot(Module& module, const SEM::TypeInstance* typeInstance, llvm::ArrayRef<SEM::Function*> methods) {
				ConstantGenerator constGen(module);
				TypeGenerator typeGen(module);
				
				if (methods.empty()) {
					return constGen.getNullPointer(typeGen.getPtrType());
				}
				
				const auto stubArgInfo = getStubArgInfo(module);
				const auto linkage = llvm::Function::InternalLinkage;
				
				const auto llvmFunction = createLLVMFunction(module, stubArgInfo, linkage, module.getCString("__slot_conflict_resolution_stub"));
				llvmFunction->setAttributes(conflictResolutionStubAttributes(module));
				
				Function function(module, *llvmFunction, stubArgInfo);
				
				IREmitter irEmitter(function);
				
				const auto llvmHashValue = function.getArg(0);
				const auto llvmArgsStructPtr = function.getArg(1);
				
				auto& builder = function.getBuilder();
				
				for (const auto semMethod : methods) {
					const auto callMethodBasicBlock = function.createBasicBlock("callMethod");
					const auto tryNextMethodBasicBlock = function.createBasicBlock("tryNextMethod");
					
					const auto methodHash = CreateMethodNameHash(semMethod->name().last());
					
					const auto cmpValue = builder.CreateICmpEQ(llvmHashValue, constGen.getI64(methodHash));
					builder.CreateCondBr(cmpValue, callMethodBasicBlock, tryNextMethodBasicBlock);
					
					function.selectBasicBlock(callMethodBasicBlock);
					
					const auto argInfo = getFunctionArgInfo(module, semMethod->type());
					
					auto& semFunctionGenerator = module.semFunctionGenerator();
					const auto llvmMethod = semFunctionGenerator.getDecl(typeInstance,
					                                                     *semMethod);
					
					const auto functionType = semMethod->type();
					const auto returnType = functionType.returnType();
					const auto& paramTypes = functionType.parameterTypes();
					
					llvm::SmallVector<llvm::Value*, 10> parameters;
					
					// If the function uses a return value pointer, just pass
					// the pointer we received from our caller.
					if (argInfo.hasReturnVarArgument()) {
						parameters.push_back(function.getReturnVar());
					}
					
					// If type is templated, pass the template generator.
					if (argInfo.hasTemplateGeneratorArgument()) {
						parameters.push_back(function.getTemplateGenerator());
					}
					
					// If this is not a static method, pass the object pointer.
					if (argInfo.hasContextArgument()) {
						parameters.push_back(function.getRawContextValue());
					}
					
					const auto numArgs = functionType.parameterTypes().size();
					
					// Build the args struct type, which is just a struct
					// containing i8* for each parameter.
					llvm::SmallVector<llvm::Type*, 10> llvmArgsTypes(numArgs, TypeGenerator(module).getPtrType());
					
					const auto llvmArgsStructType = typeGen.getStructType(llvmArgsTypes);
					
					// Extract the arguments.
					for (size_t offset = 0; offset < numArgs; offset++) {
						const auto& paramType = paramTypes.at(offset);
						
						const auto argPtrPtr = irEmitter.emitConstInBoundsGEP2_32(llvmArgsStructType,
						                                                          llvmArgsStructPtr,
						                                                          0, offset);
						const auto argPtr = irEmitter.emitRawLoad(argPtrPtr,
						                                          irEmitter.typeGenerator().getPtrType());
						
						if (canPassByValue(module, paramType)) {
							parameters.push_back(irEmitter.emitRawLoad(argPtr,
							                                           genType(module, paramType)));
						} else {
							parameters.push_back(argPtr);
						}
					}
					
					// Call the method.
					const auto llvmCallReturnValue = genRawFunctionCall(function, argInfo, llvmMethod, parameters);
					
					// Store return value.
					if (!argInfo.hasReturnVarArgument() && !returnType->isBuiltInVoid()) {
						irEmitter.emitRawStore(llvmCallReturnValue, function.getReturnVar());
					}
					
					builder.CreateRetVoid();
					
					function.selectBasicBlock(tryNextMethodBasicBlock);
				}
				
				// Terminate function with unreachable
				// (notifies optimiser that this should
				// never be reached...).
				builder.CreateUnreachable();
				
				return llvmFunction;
			}
			
		}
		
	}
}

