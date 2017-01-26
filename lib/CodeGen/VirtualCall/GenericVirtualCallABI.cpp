#include <locic/CodeGen/VirtualCall/GenericVirtualCallABI.hpp>

#include <vector>

#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/AST/Function.hpp>
#include <locic/AST/Type.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ASTFunctionGenerator.hpp>
#include <locic/CodeGen/CallEmitter.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/FunctionCallInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
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
			
			llvm::SmallVector<llvm_abi::Type, 2> arguments;
			
			// Hash value type.
			arguments.push_back(llvm_abi::Int64Ty);
			
			// Arguments struct pointer type.
			arguments.push_back(llvm_abi::PointerTy);
			
			return ArgInfo(module_, hasReturnVarArgument,
			               hasTemplateGenerator, hasContextArgument,
			               isVarArg, llvm_abi::VoidTy, arguments);
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
			
			if (args.empty()) {
				// Don't allocate struct when it's not needed.
				return ConstantGenerator(module_).getNullPointer();
			}
			
			llvm::SmallVector<llvm_abi::Type, 10> argABITypes(args.size(), llvm_abi::PointerTy);
			const auto argsStructType = module_.abiTypeBuilder().getStructTy(argABITypes);
			
			const auto argsStructPtr = irEmitter.emitRawAlloca(argsStructType);
			
			for (size_t offset = 0; offset < args.size(); offset++) {
				const auto argPtr = irEmitter.emitConstInBoundsGEP2_32(argsStructType,
				                                                       argsStructPtr,
				                                                       0, offset);
				
				if (TypeInfo(module_).isPassedByValue(argTypes[offset])) {
					const auto argAlloca = irEmitter.emitAlloca(argTypes[offset]);
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
			
			const auto llvmFunction = stubArgInfo.createFunction("__slot_conflict_resolution_stub",
			                                                     linkage);
			llvmFunction->setAttributes(conflictResolutionStubAttributes(llvmFunction->getAttributes()));
			
			Function function(module_, *llvmFunction, stubArgInfo);
			
			IREmitter irEmitter(function);
			
			const auto llvmHashValue = function.getArg(0);
			
			auto& builder = function.getBuilder();
			
			for (const auto method : methods) {
				const auto callMethodBasicBlock = irEmitter.createBasicBlock("callMethod");
				const auto tryNextMethodBasicBlock = irEmitter.createBasicBlock("tryNextMethod");
				
				const auto methodHash = CreateMethodNameHash(method->fullName().last());
				
				const auto cmpValue = builder.CreateICmpEQ(llvmHashValue, constGen.getI64(methodHash));
				irEmitter.emitCondBranch(cmpValue, callMethodBasicBlock,
				                         tryNextMethodBasicBlock);
				
				irEmitter.selectBasicBlock(callMethodBasicBlock);
				
				emitVTableSlotCall(function, typeInstance, *method);
				
				irEmitter.selectBasicBlock(tryNextMethodBasicBlock);
			}
			
			// Terminate function with unreachable
			// (notifies optimiser that this should
			// never be reached...).
			irEmitter.emitUnreachable();
			
			return llvmFunction;
		}
		
		void
		GenericVirtualCallABI::emitVTableSlotCall(Function& function,
		                                          const AST::TypeInstance& typeInstance,
		                                          const AST::Function& method) {
			const auto llvmArgsStructPtr = function.getArg(1);
			
			auto& astFunctionGenerator = module_.astFunctionGenerator();
			const auto llvmMethod = astFunctionGenerator.getDecl(&typeInstance,
			                                                     method);
			
			const auto functionType = method.type();
			
			FunctionCallInfo callInfo;
			callInfo.functionPtr = llvmMethod;
			callInfo.templateGenerator = function.getTemplateGenerator();
			callInfo.contextPointer = function.getContextValue();
			
			// Build the args struct type, which is just a struct
			// containing i8* for each parameter.
			const auto numArgs = functionType.parameterTypes().size();
			llvm::SmallVector<llvm_abi::Type, 10> argTypes(numArgs, llvm_abi::PointerTy);
			const auto argsStructType = module_.abiTypeBuilder().getStructTy(argTypes);
			
			llvm::SmallVector<llvm_abi::TypedValue, 10> parameters;
			
			IREmitter irEmitter(function);
			
			// Extract the arguments.
			for (size_t offset = 0; offset < numArgs; offset++) {
				const auto& paramType = functionType.parameterTypes().at(offset);
				
				const auto argPtrPtr = irEmitter.emitConstInBoundsGEP2_32(argsStructType,
				                                                          llvmArgsStructPtr,
				                                                          0, offset);
				const auto argPtr = irEmitter.emitRawLoad(argPtrPtr, llvm_abi::PointerTy);
				
				const auto arg = irEmitter.emitLoad(argPtr, paramType);
				const auto argABIType = genABIArgType(module_, paramType);
				parameters.push_back(llvm_abi::TypedValue(arg, argABIType));
			}
			
			CallEmitter callEmitter(irEmitter);
			auto callReturnValue = callEmitter.emitCall(functionType,
			                                            callInfo, parameters,
			                                            function.getReturnVar());
			
			if (!ArgInfo::FromAST(module_, functionType).hasReturnVarArgument()) {
				// The callee returns by-value, but we're forced to return by
				// pointer, so we store the value into our return pointer.
				irEmitter.emitStore(callReturnValue, function.getReturnVar(),
				                    functionType.returnType());
				callReturnValue = function.getReturnVar();
			}
			
			irEmitter.emitReturn(callReturnValue);
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
			                                                         llvm_abi::PointerTy);
			
			const auto callArgInfo = ArgInfo::FromAST(module_, functionType);
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
			CallEmitter callEmitter(irEmitter);
			(void) callEmitter.emitRawCall(argInfo, methodFunctionPointer,
			                               parameters);
		}
		
		llvm::Value*
		GenericVirtualCallABI::emitCall(IREmitter& irEmitter,
		                                AST::FunctionType functionType,
		                                VirtualMethodComponents methodComponents,
		                                llvm::ArrayRef<llvm::Value*> args,
		                                llvm::Value* const resultPtr) {
			const auto returnType = functionType.returnType();
			const bool hasReturnVar = !returnType->isBuiltInVoid();
			
			ConstantGenerator constGen(module_);
			
			// If the return type isn't void, allocate space on the stack for the return value.
			const auto returnVarValue = hasReturnVar ?
				irEmitter.emitAlloca(returnType, resultPtr) :
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
			
			const auto argInfo = ArgInfo::TemplateOnly(module_, llvm_abi::SizeTy).withNoMemoryAccess().withNoExcept();
			
			// Load the slot.
			const auto methodFunctionPointer = irEmitter.emitRawLoad(vtableEntryPointer,
			                                                         llvm_abi::PointerTy);
			
			CallEmitter callEmitter(irEmitter);
			return callEmitter.emitRawCall(argInfo, methodFunctionPointer,
			                               { templateGeneratorValue });
		}
		
	}
	
}
