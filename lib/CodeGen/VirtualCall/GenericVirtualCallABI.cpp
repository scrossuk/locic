#include <locic/CodeGen/VirtualCall/GenericVirtualCallABI.hpp>

#include <vector>

#include <llvm-abi/Type.hpp>

#include <locic/AST/Function.hpp>
#include <locic/AST/Type.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ASTFunctionGenerator.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/IREmitter.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/CodeGen/UnwindAction.hpp>
#include <locic/CodeGen/VirtualCallABI.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {
	
	namespace CodeGen {
		
		GenericVirtualCallABI::GenericVirtualCallABI(Module& module)
		: module_(module) { }
		
		GenericVirtualCallABI::~GenericVirtualCallABI() { }
		
		ArgInfo
		GenericVirtualCallABI::getStubArgInfo() {
			const bool hasReturnVarArgument = true;
			const bool hasTemplateGenerator = true;
			const bool hasContextArgument = true;
			const bool isVarArg = false;
			
			TypeGenerator typeGen(module_);
			
			llvm::SmallVector<TypePair, 2> arguments;
			
			// Hash value type.
			arguments.push_back(std::make_pair(llvm_abi::Int64Ty, typeGen.getI64Type()));
			
			// Arguments struct pointer type.
			arguments.push_back(std::make_pair(llvm_abi::PointerTy, typeGen.getPtrType()));
			
			return ArgInfo(module_, hasReturnVarArgument,
			               hasTemplateGenerator, hasContextArgument,
			               isVarArg, voidTypePair(module_),
			               arguments);
		}
		
		llvm::AttributeSet
		GenericVirtualCallABI::conflictResolutionStubAttributes(const llvm::AttributeSet& existingAttributes) {
			const auto iterator = module_.attributeMap().find(AttributeVirtualCallStub);
			
			if (iterator != module_.attributeMap().end()) {
				return iterator->second;
			}
			
			auto& context = module_.getLLVMContext();
			
			auto attributes = existingAttributes;
			
			// Always inline stubs.
			attributes = attributes.addAttribute(context, llvm::AttributeSet::FunctionIndex, llvm::Attribute::AlwaysInline);
			
			// Return value pointer attributes.
			attributes = attributes.addAttribute(context, 1, llvm::Attribute::StructRet);
			attributes = attributes.addAttribute(context, 1, llvm::Attribute::NoAlias);
			attributes = attributes.addAttribute(context, 1, llvm::Attribute::NoCapture);
			
			// Arguments struct pointer attributes.
			attributes = attributes.addAttribute(context, 4, llvm::Attribute::NoAlias);
			attributes = attributes.addAttribute(context, 4, llvm::Attribute::NoCapture);
			
			module_.attributeMap().insert(std::make_pair(AttributeVirtualCallStub, attributes));
			
			return attributes;
		}
		
		llvm::Value*
		GenericVirtualCallABI::makeArgsStruct(IREmitter& irEmitter,
		                                      llvm::ArrayRef<const AST::Type*> argTypes,
		                                      llvm::ArrayRef<llvm::Value*> args) {
			assert(argTypes.size() == args.size());
			
			const auto ptrType = TypeGenerator(module_).getPtrType();
			
			if (args.empty()) {
				// Don't allocate struct when it's not needed.
				return ConstantGenerator(module_).getNullPointer();
			}
			
			llvm::SmallVector<llvm::Type*, 10> llvmArgTypes(args.size(), ptrType);
			const auto argsStructType = TypeGenerator(module_).getStructType(llvmArgTypes);
			
			const auto argsStructPtr = irEmitter.emitRawAlloca(argsStructType);
			
			for (size_t offset = 0; offset < args.size(); offset++) {
				const auto argPtr = irEmitter.emitConstInBoundsGEP2_32(argsStructType,
				                                                       argsStructPtr,
				                                                       0, offset);
				
				if (TypeInfo(module_).isPassedByValue(argTypes[offset])) {
					const auto argAlloca = irEmitter.emitRawAlloca(args[offset]->getType());
					irEmitter.emitRawStore(args[offset], argAlloca);
					irEmitter.emitRawStore(argAlloca, argPtr);
				} else {
					irEmitter.emitRawStore(args[offset], argPtr);
				}
			}
			
			return argsStructPtr;
		}
		
		llvm::Constant*
		GenericVirtualCallABI::emitVTableSlot(const AST::TypeInstance& typeInstance,
		                                      llvm::ArrayRef<AST::Function*> methods) {
			ConstantGenerator constGen(module_);
			TypeGenerator typeGen(module_);
			
			if (methods.empty()) {
				return constGen.getNullPointer();
			}
			
			const auto stubArgInfo = getStubArgInfo();
			const auto linkage = llvm::Function::InternalLinkage;
			
			const auto llvmFunction = createLLVMFunction(module_, stubArgInfo, linkage,
			                                             module_.getCString("__slot_conflict_resolution_stub"));
			llvmFunction->setAttributes(conflictResolutionStubAttributes(llvmFunction->getAttributes()));
			
			Function function(module_, *llvmFunction, stubArgInfo);
			
			IREmitter irEmitter(function);
			
			const auto llvmHashValue = function.getArg(0);
			const auto llvmArgsStructPtr = function.getArg(1);
			
			auto& builder = function.getBuilder();
			
			for (const auto method : methods) {
				const auto callMethodBasicBlock = irEmitter.createBasicBlock("callMethod");
				const auto tryNextMethodBasicBlock = irEmitter.createBasicBlock("tryNextMethod");
				
				const auto methodHash = CreateMethodNameHash(method->fullName().last());
				
				const auto cmpValue = builder.CreateICmpEQ(llvmHashValue, constGen.getI64(methodHash));
				irEmitter.emitCondBranch(cmpValue, callMethodBasicBlock,
				                         tryNextMethodBasicBlock);
				
				irEmitter.selectBasicBlock(callMethodBasicBlock);
				
				const auto argInfo = getFunctionArgInfo(module_, method->type());
				
				auto& astFunctionGenerator = module_.astFunctionGenerator();
				const auto llvmMethod = astFunctionGenerator.getDecl(&typeInstance,
				                                                     *method);
				
				const auto functionType = method->type();
				const auto returnType = functionType.returnType();
				const auto& paramTypes = functionType.parameterTypes();
				
				llvm::SmallVector<llvm::Value*, 10> parameters;
				
				// If the function uses a return value pointer, just pass
				// the pointer we received from our caller.
				if (argInfo.hasReturnVarArgument()) {
					parameters.push_back(function.getReturnVar());
				}
				
				// If type is templated, pass the template generator.
				if (argInfo.isVarArg() && argInfo.hasTemplateGeneratorArgument()) {
					parameters.push_back(function.getTemplateGenerator());
				}
				
				// If this is not a static method, pass the object pointer.
				if (argInfo.hasContextArgument()) {
					parameters.push_back(function.getContextValue());
				}
				
				const auto numArgs = functionType.parameterTypes().size();
				
				// Build the args struct type, which is just a struct
				// containing i8* for each parameter.
				llvm::SmallVector<llvm::Type*, 10> llvmArgsTypes(numArgs, TypeGenerator(module_).getPtrType());
				
				const auto llvmArgsStructType = typeGen.getStructType(llvmArgsTypes);
				
				// Extract the arguments.
				for (size_t offset = 0; offset < numArgs; offset++) {
					const auto& paramType = paramTypes.at(offset);
					
					const auto argPtrPtr = irEmitter.emitConstInBoundsGEP2_32(llvmArgsStructType,
					                                                          llvmArgsStructPtr,
					                                                          0, offset);
					const auto argPtr = irEmitter.emitRawLoad(argPtrPtr,
					                                          irEmitter.typeGenerator().getPtrType());
					
					if (TypeInfo(module_).isPassedByValue(paramType)) {
						parameters.push_back(irEmitter.emitRawLoad(argPtr,
						                                           genType(module_, paramType)));
					} else {
						parameters.push_back(argPtr);
					}
				}
				
				// If type is templated, pass the template generator.
				if (!argInfo.isVarArg() && argInfo.hasTemplateGeneratorArgument()) {
					parameters.push_back(function.getTemplateGenerator());
				}
				
				// Call the method.
				const auto llvmCallReturnValue = genRawFunctionCall(function, argInfo, llvmMethod, parameters);
				
				// Store return value.
				if (!argInfo.hasReturnVarArgument() && !returnType->isBuiltInVoid()) {
					irEmitter.emitRawStore(llvmCallReturnValue, function.getReturnVar());
				}
				
				irEmitter.emitReturnVoid();
				
				irEmitter.selectBasicBlock(tryNextMethodBasicBlock);
			}
			
			// Terminate function with unreachable
			// (notifies optimiser that this should
			// never be reached...).
			irEmitter.emitUnreachable();
			
			return llvmFunction;
		}
		
		void
		GenericVirtualCallABI::emitCallWithReturnVar(IREmitter& irEmitter,
		                                             const AST::FunctionType functionType,
		                                             llvm::Value* returnVarPointer,
		                                             const VirtualMethodComponents methodComponents,
		                                             llvm::ArrayRef<llvm::Value*> args) {
			// Calculate the slot for the virtual call.
			ConstantGenerator constantGen(module_);
			const auto vtableSizeValue = constantGen.getI64(VTABLE_SIZE);
			
			const auto vtableOffsetValue = irEmitter.builder().CreateURem(methodComponents.hashValue,
			                                                              vtableSizeValue, "vtableOffset");
			const auto castVTableOffsetValue = irEmitter.builder().CreateTrunc(vtableOffsetValue,
			                                                                   irEmitter.typeGenerator().getI32Type());
			
			// Get a pointer to the slot.
			llvm::SmallVector<llvm::Value*, 3> vtableEntryGEP;
			vtableEntryGEP.push_back(constantGen.getI32(0));
			vtableEntryGEP.push_back(constantGen.getI32(2));
			vtableEntryGEP.push_back(castVTableOffsetValue);
			
			const auto vtableEntryPointer = irEmitter.emitInBoundsGEP(vtableType(module_),
			                                                          methodComponents.object.typeInfo.vtablePointer,
			                                                          vtableEntryGEP);
			
			// Load the slot.
			const auto methodFunctionPointer = irEmitter.emitRawLoad(vtableEntryPointer,
			                                                         irEmitter.typeGenerator().getPtrType());
			
			const auto callArgInfo = getFunctionArgInfo(module_, functionType);
			auto argInfo = getStubArgInfo();
			if (callArgInfo.noMemoryAccess()) {
				argInfo = argInfo.withNoMemoryAccess();
			}
			if (callArgInfo.noExcept()) {
				argInfo = argInfo.withNoExcept();
			}
			if (callArgInfo.noReturn()) {
				argInfo = argInfo.withNoReturn();
			}
			
			// Put together the arguments.
			llvm::SmallVector<llvm::Value*, 5> parameters;
			
			// Pass in the return var pointer.
			parameters.push_back(returnVarPointer);
			
			// Pass in the object pointer.
			parameters.push_back(methodComponents.object.objectPointer);
			
			// Pass in the method hash value.
			parameters.push_back(methodComponents.hashValue);
			
			// Store all the arguments into a struct on the stack,
			// and pass the pointer to the stub.
			const auto argsStructPtr = makeArgsStruct(irEmitter, arrayRef(functionType.parameterTypes()), args);
			parameters.push_back(argsStructPtr);
			
			// Pass in the template generator.
			parameters.push_back(methodComponents.object.typeInfo.templateGenerator);
			
			// Call the stub function.
			(void) genRawFunctionCall(irEmitter.function(), argInfo,
			                          methodFunctionPointer, parameters);
		}
		
		llvm::Value*
		GenericVirtualCallABI::emitCall(IREmitter& irEmitter,
		                                AST::FunctionType functionType,
		                                VirtualMethodComponents methodComponents,
		                                llvm::ArrayRef<llvm::Value*> args,
		                                llvm::Value* const hintResultValue) {
			const auto returnType = functionType.returnType();
			const bool hasReturnVar = !returnType->isBuiltInVoid();
			
			ConstantGenerator constGen(module_);
			
			// If the return type isn't void, allocate space on the stack for the return value.
			const auto returnVarValue = hasReturnVar ?
				irEmitter.emitAlloca(returnType, hintResultValue) :
				constGen.getNullPointer();
			
			emitCallWithReturnVar(irEmitter, functionType,
			                      returnVarValue, methodComponents,
			                      args);
			
			// If the return type isn't void, load the return value from the stack.
			return hasReturnVar ? irEmitter.emitLoad(returnVarValue, returnType) : constGen.getVoidUndef();
		}
		
		llvm::Value*
		GenericVirtualCallABI::emitCountFnCall(IREmitter& irEmitter,
		                                       llvm::Value* const typeInfoValue,
		                                       const CountFnKind kind) {
			// Extract vtable and template generator.
			const auto vtablePointer = irEmitter.builder().CreateExtractValue(typeInfoValue, { 0 }, "vtable");
			const auto templateGeneratorValue = irEmitter.builder().CreateExtractValue(typeInfoValue, { 1 }, "templateGenerator");
			
			// Get a pointer to the slot.
			ConstantGenerator constGen(module_);
			llvm::SmallVector<llvm::Value*, 2> vtableEntryGEP;
			vtableEntryGEP.push_back(constGen.getI32(0));
			vtableEntryGEP.push_back(constGen.getI32(kind == ALIGNOF ? 0 : 1));
			
			const auto vtableEntryPointer = irEmitter.emitInBoundsGEP(vtableType(module_),
			                                                          vtablePointer,
			                                                          vtableEntryGEP);
			
			const auto argInfo = ArgInfo::TemplateOnly(module_, sizeTypePair(module_)).withNoMemoryAccess().withNoExcept();
			
			// Load the slot.
			const auto methodFunctionPointer = irEmitter.emitRawLoad(vtableEntryPointer,
			                                                         irEmitter.typeGenerator().getPtrType());
			
			return genRawFunctionCall(irEmitter.function(), argInfo,
			                          methodFunctionPointer,
			                          { templateGeneratorValue });
		}
		
	}
	
}
