#include <vector>

#include <llvm-abi/Type.hpp>

#include <locic/AST/Function.hpp>
#include <locic/AST/Type.hpp>

#include <locic/SEM.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/GenFunctionCall.hpp>
#include <locic/CodeGen/GenType.hpp>
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
#include <locic/CodeGen/VirtualCall/NestVirtualCallABI.hpp>
#include <locic/CodeGen/VirtualCallABI.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {
	
	namespace CodeGen {
		
		NestVirtualCallABI::NestVirtualCallABI(Module& module)
		: module_(module) { }
		
		NestVirtualCallABI::~NestVirtualCallABI() { }
		
		ArgInfo
		NestVirtualCallABI::getStubArgInfo() {
			TypeGenerator typeGen(module_);
			
			// Return i64 as a generic register sized value.
			const auto i64Type = std::make_pair(llvm_abi::Int64Ty, typeGen.getI64Type());
			
			return ArgInfo::VarArgs(module_, i64Type, {}).withNestArgument();
		}
		
		llvm::AttributeSet
		NestVirtualCallABI::conflictResolutionStubAttributes(const llvm::AttributeSet& existingAttributes) {
			auto& context = module_.getLLVMContext();
			
			auto attributes = existingAttributes;
			
			// Always inline stubs.
			attributes = attributes.addAttribute(context, llvm::AttributeSet::FunctionIndex, llvm::Attribute::AlwaysInline);
			
			return attributes;
		}
		
		llvm::Constant*
		NestVirtualCallABI::emitVTableSlot(const AST::TypeInstance& typeInstance,
		                                      llvm::ArrayRef<AST::Function*> methods) {
			ConstantGenerator constGen(module_);
			TypeGenerator typeGen(module_);
			
			if (methods.empty()) {
				return constGen.getNullPointer();
			}
			
			if (methods.size() == 1) {
				// Only one method, so place it directly in the slot.
				auto& semFunctionGenerator = module_.semFunctionGenerator();
				const auto llvmMethod = semFunctionGenerator.getDecl(&typeInstance,
				                                                     *methods[0]);
				return llvmMethod;
			}
			
			const auto stubArgInfo = getStubArgInfo();
			const auto linkage = llvm::Function::InternalLinkage;
			
			const auto llvmFunction = createLLVMFunction(module_, stubArgInfo, linkage,
			                                             module_.getCString("__slot_conflict_resolution_stub"));
			llvmFunction->setAttributes(conflictResolutionStubAttributes(llvmFunction->getAttributes()));
			
			Function function(module_, *llvmFunction, stubArgInfo);
			
			IREmitter irEmitter(function);
			auto& builder = function.getBuilder();
			
			const auto llvmHashValuePtr = function.getNestArgument();
			const auto llvmHashValue = builder.CreatePtrToInt(llvmHashValuePtr,
			                                                  typeGen.getI64Type());
			
			for (const auto method : methods) {
				const auto callMethodBasicBlock = irEmitter.createBasicBlock("callMethod");
				const auto tryNextMethodBasicBlock = irEmitter.createBasicBlock("tryNextMethod");
				
				const auto methodHash = CreateMethodNameHash(method->fullName().last());
				
				const auto cmpValue = builder.CreateICmpEQ(llvmHashValue, constGen.getI64(methodHash));
				irEmitter.emitCondBranch(cmpValue, callMethodBasicBlock,
				                         tryNextMethodBasicBlock);
				
				irEmitter.selectBasicBlock(callMethodBasicBlock);
				
				auto& semFunctionGenerator = module_.semFunctionGenerator();
				const auto llvmMethod = semFunctionGenerator.getDecl(&typeInstance,
				                                                     *method);
				llvm::Value* const parameters[] = { llvmHashValuePtr };
				
				// Use 'musttail' to ensure perfect forwarding.
				const auto result = genRawFunctionCall(function, stubArgInfo, llvmMethod, parameters,
				                                       /*musttail=*/true);
				
				irEmitter.emitReturn(result->getType(), result);
				
				irEmitter.selectBasicBlock(tryNextMethodBasicBlock);
			}
			
			// Terminate function with unreachable
			// (notifies optimiser that this should
			// never be reached...).
			irEmitter.emitUnreachable();
			
			return llvmFunction;
		}
		
		llvm::Value*
		NestVirtualCallABI::emitRawCall(IREmitter& irEmitter,
		                                const ArgInfo& argInfo,
		                                const VirtualMethodComponents methodComponents,
		                                llvm::ArrayRef<llvm::Value*> args,
		                                llvm::Value* const returnVarPointer) {
			ConstantGenerator constantGen(module_);
			
			// Calculate the slot for the virtual call.
			const auto vtableSizeValue = constantGen.getI64(VTABLE_SIZE);
			const auto vtableOffsetValue = irEmitter.builder().CreateURem(methodComponents.hashValue,
			                                                              vtableSizeValue, "vtableOffset");
			const auto castVTableOffsetValue = irEmitter.builder().CreateTrunc(vtableOffsetValue,
			                                                                   irEmitter.typeGenerator().getI32Type());
			
			// Get a pointer to the slot.
			llvm::SmallVector<llvm::Value*, 3> vtableEntryGEP;
			vtableEntryGEP.push_back(constantGen.getI32(0));
			vtableEntryGEP.push_back(constantGen.getI32(4));
			vtableEntryGEP.push_back(castVTableOffsetValue);
			
			const auto vtableEntryPointer = irEmitter.emitInBoundsGEP(vtableType(module_),
			                                                          methodComponents.object.typeInfo.vtablePointer,
			                                                          vtableEntryGEP);
			
			// Load the slot.
			const auto methodFunctionPointer = irEmitter.emitRawLoad(vtableEntryPointer,
			                                                         irEmitter.typeGenerator().getPtrType());
			
			// Cast hash value to pointer so we can pass it through
			// as the 'nest' parameter.
			const auto hashValuePtr = irEmitter.builder().CreateIntToPtr(methodComponents.hashValue,
			                                                             irEmitter.typeGenerator().getPtrType());
			
			llvm::SmallVector<llvm::Value*, 8> newArgs;
			newArgs.reserve(args.size() + 4);
			
			// Add the 'nest' parameter.
			newArgs.push_back(hashValuePtr);
			
			// Add return pointer if necessary.
			if (argInfo.hasReturnVarArgument()) {
				assert(returnVarPointer != nullptr);
				newArgs.push_back(returnVarPointer);
			} else {
				assert(returnVarPointer == nullptr);
			}
			
			// Add object pointer.
			if (argInfo.hasContextArgument()) {
				newArgs.push_back(methodComponents.object.objectPointer);
			}
			
			// Add the method arguments.
			for (const auto& arg: args) {
				newArgs.push_back(arg);
			}
			
			// Add template generator.
			if (argInfo.hasTemplateGeneratorArgument()) {
				newArgs.push_back(methodComponents.object.typeInfo.templateGenerator);
			}
			
			// Call the stub function.
			return genRawFunctionCall(irEmitter.function(), argInfo.withNestArgument(),
			                          methodFunctionPointer, newArgs);
		}
		
		llvm::Value*
		NestVirtualCallABI::emitCall(IREmitter& irEmitter,
		                             const AST::FunctionType functionType,
		                             const VirtualMethodComponents methodComponents,
		                             llvm::ArrayRef<llvm::Value*> args,
		                             llvm::Value* const hintResultValue) {
			const auto argInfo = getFunctionArgInfo(irEmitter.module(),
			                                        functionType);
			
			// If necessary allocate space on the stack for the return value.
			const auto returnType = functionType.returnType();
			llvm::Value* returnVarPointer = nullptr;
			if (argInfo.hasReturnVarArgument()) {
				returnVarPointer = irEmitter.emitAlloca(returnType, hintResultValue);
			}
			
			const auto result = emitRawCall(irEmitter,
			                                argInfo,
			                                methodComponents,
			                                args,
			                                returnVarPointer);
			
			return returnVarPointer != nullptr ?
			       irEmitter.emitMoveLoad(returnVarPointer, returnType) : result;
		}
		
		llvm::Value*
		NestVirtualCallABI::emitCountFnCall(IREmitter& irEmitter,
		                                       llvm::Value* const typeInfoValue,
		                                       const CountFnKind kind) {
			// Extract vtable and template generator.
			const auto vtablePointer = irEmitter.builder().CreateExtractValue(typeInfoValue, { 0 }, "vtable");
			const auto templateGeneratorValue = irEmitter.builder().CreateExtractValue(typeInfoValue, { 1 }, "templateGenerator");
			
			// Get a pointer to the slot.
			ConstantGenerator constGen(module_);
			llvm::SmallVector<llvm::Value*, 2> vtableEntryGEP;
			vtableEntryGEP.push_back(constGen.getI32(0));
			vtableEntryGEP.push_back(constGen.getI32(kind == ALIGNOF ? 2 : 3));
			
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
		
		ArgInfo
		NestVirtualCallABI::virtualMoveArgInfo() {
			const TypePair types[] = { pointerTypePair(module_), sizeTypePair(module_) };
			return ArgInfo::VoidTemplateAndContextWithArgs(module_, types).withNoExcept();
		}
		
		void
		NestVirtualCallABI::emitMoveCall(IREmitter& irEmitter,
		                                    llvm::Value* typeInfoValue,
		                                    llvm::Value* sourceValue,
		                                    llvm::Value* destValue,
		                                    llvm::Value* positionValue) {
			// Extract vtable and template generator.
			const auto vtablePointer = irEmitter.builder().CreateExtractValue(typeInfoValue, { 0 }, "vtable");
			const auto templateGeneratorValue = irEmitter.builder().CreateExtractValue(typeInfoValue, { 1 }, "templateGenerator");
			
			// Get a pointer to the slot.
			ConstantGenerator constGen(module_);
			llvm::SmallVector<llvm::Value*, 2> vtableEntryGEP;
			vtableEntryGEP.push_back(constGen.getI32(0));
			vtableEntryGEP.push_back(constGen.getI32(0));
			
			const auto vtableEntryPointer = irEmitter.emitInBoundsGEP(vtableType(module_),
			                                                          vtablePointer,
			                                                          vtableEntryGEP);
			
			const auto argInfo = virtualMoveArgInfo();
			
			// Load the slot.
			const auto methodFunctionPointer = irEmitter.emitRawLoad(vtableEntryPointer,
			                                                         irEmitter.typeGenerator().getPtrType());
			
			llvm::Value* const args[] = { sourceValue, destValue, positionValue, templateGeneratorValue };
			(void) genRawFunctionCall(irEmitter.function(), argInfo, methodFunctionPointer, args);
		}
		
		ArgInfo
		NestVirtualCallABI::virtualDestructorArgInfo() {
			return ArgInfo::VoidTemplateAndContext(module_).withNoExcept();
		}
		
		void
		NestVirtualCallABI::emitDestructorCall(IREmitter& irEmitter,
		                                          llvm::Value* typeInfoValue,
		                                          llvm::Value* objectValue) {
			// Extract vtable and template generator.
			const auto vtablePointer = irEmitter.builder().CreateExtractValue(typeInfoValue, { 0 }, "vtable");
			const auto templateGeneratorValue = irEmitter.builder().CreateExtractValue(typeInfoValue, { 1 }, "templateGenerator");
			
			// Get a pointer to the slot.
			ConstantGenerator constGen(module_);
			llvm::SmallVector<llvm::Value*, 2> vtableEntryGEP;
			vtableEntryGEP.push_back(constGen.getI32(0));
			vtableEntryGEP.push_back(constGen.getI32(1));
			
			const auto vtableEntryPointer = irEmitter.emitInBoundsGEP(vtableType(module_),
			                                                          vtablePointer,
			                                                          vtableEntryGEP);
			
			const auto argInfo = virtualDestructorArgInfo();
			
			// Load the slot.
			const auto methodFunctionPointer = irEmitter.emitRawLoad(vtableEntryPointer,
			                                                         irEmitter.typeGenerator().getPtrType());
			
			llvm::Value* const args[] = { objectValue, templateGeneratorValue };
			(void) genRawFunctionCall(irEmitter.function(), argInfo, methodFunctionPointer, args);
		}
		
	}
	
}
