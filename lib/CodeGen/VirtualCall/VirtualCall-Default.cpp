#include <vector>

#include <llvm-abi/Type.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenFunction.hpp>
#include <locic/CodeGen/GenValue.hpp>
#include <locic/CodeGen/Memory.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/VirtualCall.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
	
		namespace VirtualCall {
		
			llvm::FunctionType* getStubFunctionType(Module& module) {
				TypeGenerator typeGen(module);
				
				std::vector<llvm::Type*> argTypes;
				
				// Return value pointer type (to handle
				// any possible return type).
				argTypes.push_back(typeGen.getI8PtrType());
				
				// Template generator type.
				argTypes.push_back(templateGeneratorType(module));
				
				// Class pointer type.
				argTypes.push_back(typeGen.getI8PtrType());
				
				// Hash value type.
				argTypes.push_back(typeGen.getI64Type());
				
				// Arguments struct pointer type.
				argTypes.push_back(typeGen.getI8PtrType());
				
				return typeGen.getVoidFunctionType(argTypes);
			}
			
			ArgInfo getStubArgInfo() {
				const bool hasReturnVarArgument = true;
				const bool hasTemplateGenerator = true;
				const bool hasContextArgument = true;
				
				std::vector<llvm_abi::Type> standardArguments;
				standardArguments.push_back(llvm_abi::Type::Integer(llvm_abi::Int64));
				standardArguments.push_back(llvm_abi::Type::Pointer());
				
				return ArgInfo(hasReturnVarArgument, hasTemplateGenerator, hasContextArgument, std::move(standardArguments), {nullptr, nullptr});
			}
			
			void setStubAttributes(llvm::Function* llvmFunction) {
				{
					// Return value pointer attributes.
					llvmFunction->addAttribute(1, llvm::Attribute::StructRet);
					llvmFunction->addAttribute(1, llvm::Attribute::NoAlias);
					llvmFunction->addAttribute(1, llvm::Attribute::NoCapture);
				}
				
				{
					// Arguments struct pointer attributes.
					llvmFunction->addAttribute(4, llvm::Attribute::NoAlias);
					llvmFunction->addAttribute(4, llvm::Attribute::NoCapture);
				}
			}
			
			llvm::Value* makeArgsStruct(Function& function, const std::vector<llvm::Value*>& args) {
				if (args.empty()) {
					// Don't allocate struct when it's not needed.
					return ConstantGenerator(function.module()).getNullPointer(TypeGenerator(function.module()).getI8PtrType());
				}
				
				std::vector<llvm::Type*> argTypes;
				for (auto arg: args) {
					argTypes.push_back(arg->getType());
				}
				
				const auto argsStructType = TypeGenerator(function.module()).getStructType(argTypes);
				
				const auto argsStructPtr = function.getEntryBuilder().CreateAlloca(argsStructType);
				for (size_t offset = 0; offset < args.size(); offset++) {
					const auto argPtr = function.getBuilder().CreateConstInBoundsGEP2_32(
						argsStructPtr, 0, offset);
					function.getBuilder().CreateStore(args.at(offset), argPtr);
				}
				
				return argsStructPtr;
			}
			
			llvm::Value* generateCall(Function& function, SEM::Type* functionType, llvm::Value* interfaceMethodValue, const std::vector<llvm::Value*>& args) {
				auto& builder = function.getBuilder();
				
				// Extract the components of the interface method struct.
				const auto interfaceValue = builder.CreateExtractValue(interfaceMethodValue, { 0 }, "interface");
				const auto objectPointer = builder.CreateExtractValue(interfaceValue, { 0 }, "object");
				const auto typeInfoValue = builder.CreateExtractValue(interfaceValue, { 1 }, "typeInfo");
				const auto vtablePointer = builder.CreateExtractValue(typeInfoValue, { 0 }, "vtable");
				const auto templateGeneratorValue = builder.CreateExtractValue(typeInfoValue, { 1 }, "templateGenerator");
				const auto hashValue = builder.CreateExtractValue(interfaceMethodValue, { 1 }, "methodHash");
				
				// Calculate the slot for the virtual call.
				ConstantGenerator constantGen(function.module());
				const auto vtableSizeValue = constantGen.getI64(VTABLE_SIZE);
				
				const auto vtableOffsetValue = builder.CreateURem(hashValue, vtableSizeValue, "vtableOffset");
				
				// Get a pointer to the slot.
				std::vector<llvm::Value*> vtableEntryGEP;
				vtableEntryGEP.push_back(constantGen.getI32(0));
				vtableEntryGEP.push_back(constantGen.getI32(2));
				vtableEntryGEP.push_back(vtableOffsetValue);
				
				const auto vtableEntryPointer = builder.CreateInBoundsGEP(vtablePointer, vtableEntryGEP, "vtableEntryPointer");
				
				// Load the slot.
				const auto methodFunctionPointer = builder.CreateLoad(vtableEntryPointer, "methodFunctionPointer");
				
				// Cast the loaded pointer to the stub function type.
				const auto stubFunctionPtrType = getStubFunctionType(function.module())->getPointerTo();
						 
				const auto castedMethodFunctionPointer = builder.CreatePointerCast(methodFunctionPointer, stubFunctionPtrType, "castedMethodFunctionPointer");
				
				// i8
				const auto i8PtrType = TypeGenerator(function.module()).getI8PtrType();
				
				// Put together the arguments.
				std::vector<llvm::Value*> parameters;
				
				const auto returnType = functionType->getFunctionReturnType();
				
				// If the return type isn't void, allocate space on the stack for the return value.
				llvm::Value* returnVarValue = nullptr;
				if (!returnType->isVoid()) {
					returnVarValue = genAlloca(function, returnType);
					parameters.push_back(builder.CreatePointerCast(returnVarValue, i8PtrType, "castedReturnVarPtr"));
				} else {
					parameters.push_back(constantGen.getNullPointer(i8PtrType));
				}
				
				// Pass in the template generator.
				parameters.push_back(templateGeneratorValue);
				
				// Pass in the object pointer.
				parameters.push_back(builder.CreatePointerCast(objectPointer, i8PtrType, "castedObjectPtr"));
				
				// Pass in the method hash value.
				parameters.push_back(hashValue);
				
				// Store all the arguments into a struct on the stack,
				// and pass the pointer to the stub.
				const auto argsStructPtr = makeArgsStruct(function, args);
				parameters.push_back(builder.CreatePointerCast(argsStructPtr, i8PtrType, "castedArgsStructPtr"));
				
				// Call the stub function.
				// TODO: exception handling!
				const auto callReturnValue = builder.CreateCall(castedMethodFunctionPointer, parameters);
				
				// If the return type isn't void, load the return value from the stack.
				if (returnVarValue != NULL) {
					return genLoad(function, returnVarValue, returnType);
				} else {
					return callReturnValue;
				}
			}
			
			llvm::Constant* generateVTableSlot(Module& module, SEM::TypeInstance* typeInstance, const std::vector<SEM::Function*>& methods) {
				ConstantGenerator constGen(module);
				
				if (methods.empty()) {
					return constGen.getNullPointer(TypeGenerator(module).getI8PtrType());
				}
				
				TypeGenerator typeGen(module);
				
				const auto linkage = llvm::Function::PrivateLinkage;
				
				llvm::Function* llvmFunction = createLLVMFunction(module, getStubFunctionType(module), linkage, "__slot_conflict_resolution_stub");
				
				setStubAttributes(llvmFunction);
				
				Function function(module, *llvmFunction, getStubArgInfo());
				
				const auto llvmHashValue = function.getArg(0);
				const auto llvmOpaqueArgsStructPtr = function.getArg(1);
				
				for (const auto semMethod : methods) {
					const auto callMethodBasicBlock = function.createBasicBlock("callMethod");
					const auto tryNextMethodBasicBlock = function.createBasicBlock("tryNextMethod");
					
					const auto methodHash = CreateMethodNameHash(semMethod->name().last());
					
					const auto cmpValue = function.getBuilder().CreateICmpEQ(llvmHashValue, constGen.getI64(methodHash));
					function.getBuilder().CreateCondBr(cmpValue, callMethodBasicBlock, tryNextMethodBasicBlock);
					
					function.selectBasicBlock(callMethodBasicBlock);
					
					const auto llvmMethod = genFunction(module, typeInstance, semMethod);
					
					const auto functionType = semMethod->type();
					const auto returnType = functionType->getFunctionReturnType();
					
					std::vector<llvm::Value*> parameters;
					
					// If the function uses a return value pointer, just pass
					// the pointer we received from our caller.
					if (!isTypeSizeAlwaysKnown(module, returnType)) {
						const auto returnVarPointerType = llvmMethod->getFunctionType()->getParamType(0);
						const auto llvmCastReturnVar = function.getBuilder().CreatePointerCast(function.getReturnVar(), returnVarPointerType, "castedReturnVar");
						parameters.push_back(llvmCastReturnVar);
					}
					
					// If type is templated, pass the template generator.
					if (!typeInstance->templateVariables().empty()) {
						parameters.push_back(function.getTemplateGenerator());
					}
					
					// If this is not a static method, pass the object pointer.
					if (!semMethod->isStaticMethod()) {
						const auto objectPointerOffset = parameters.size();
						const auto objectPointerType = llvmMethod->getFunctionType()->getParamType(objectPointerOffset);
						parameters.push_back(function.getBuilder().CreatePointerCast(function.getContextValue(), objectPointerType));
					}
					
					const auto numArgs = functionType->getFunctionParameterTypes().size();
					const auto rawArgsOffset = parameters.size();
					
					// Build the args struct type.
					std::vector<llvm::Type*> llvmArgsTypes;
					for (size_t offset = 0; offset < numArgs; offset++) {
						assert(offset < llvmMethod->getFunctionType()->getNumParams());
						const auto rawOffset = rawArgsOffset + offset;
						llvmArgsTypes.push_back(llvmMethod->getFunctionType()->getParamType(rawOffset));
					}
					
					const auto llvmArgsStructPtrType = typeGen.getStructType(llvmArgsTypes)->getPointerTo();
					
					// Cast the args struct pointer.
					const auto llvmArgsStructPtr = function.getBuilder().CreatePointerCast(llvmOpaqueArgsStructPtr, llvmArgsStructPtrType, "castedArgsStructPtr");
					
					// Extract the arguments.
					for (size_t offset = 0; offset < numArgs; offset++) {
						const auto argPtr = function.getBuilder().CreateConstInBoundsGEP2_32(llvmArgsStructPtr, 0, offset);
						parameters.push_back(function.getBuilder().CreateLoad(argPtr, "extractedArg"));
					}
					
					// Call the method.
					const auto llvmCallReturnValue = function.getBuilder().CreateCall(llvmMethod, parameters);
					
					// Store return value.
					if (isTypeSizeAlwaysKnown(module, returnType) && !returnType->isVoid()) {
						const auto returnValuePointerType = llvmMethod->getFunctionType()->getReturnType()->getPointerTo();
						const auto llvmCastReturnVar = function.getBuilder().CreatePointerCast(function.getReturnVar(), returnValuePointerType, "castedReturnVar");
						function.getBuilder().CreateStore(llvmCallReturnValue, llvmCastReturnVar);
					}
					
					function.getBuilder().CreateRetVoid();
					
					function.selectBasicBlock(tryNextMethodBasicBlock);
				}
				
				// Terminate function with unreachable
				// (notifies optimiser that this should
				// never be reached...).
				function.getBuilder().CreateUnreachable();
				
				return llvmFunction;
			}
			
		}
		
	}
}
	
