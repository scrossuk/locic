#include <vector>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Exception.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/ScopeExitActions.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm::Function* getExceptionAllocateFunction(Module& module) {
			const std::string functionName = "__loci_allocate_exception";
			const auto result = module.getFunctionMap().tryGet(functionName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			const auto functionType = typeGen.getFunctionType(typeGen.getI8PtrType(), std::vector<llvm::Type*> { getSizeType(module.getTargetInfo()) });
			
			const auto function = createLLVMFunction(module, functionType, llvm::Function::ExternalLinkage, functionName);
			function->addFnAttr(llvm::Attribute::NoUnwind);
			
			module.getFunctionMap().insert(functionName, function);
			
			return function;
		}
		
		llvm::Function* getExceptionFreeFunction(Module& module) {
			const std::string functionName = "__loci_free_exception";
			const auto result = module.getFunctionMap().tryGet(functionName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			const auto functionType = typeGen.getFunctionType(typeGen.getVoidType(), std::vector<llvm::Type*> { typeGen.getI8PtrType() });
			
			const auto function = createLLVMFunction(module, functionType, llvm::Function::ExternalLinkage, functionName);
			function->addFnAttr(llvm::Attribute::NoUnwind);
			
			module.getFunctionMap().insert(functionName, function);
			
			return function;
		}
		
		llvm::Function* getExceptionThrowFunction(Module& module) {
			const std::string functionName = "__loci_throw";
			const auto result = module.getFunctionMap().tryGet(functionName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			const auto functionType = typeGen.getVoidFunctionType(std::vector<llvm::Type*> { typeGen.getI8PtrType(), typeGen.getI8PtrType(), typeGen.getI8PtrType() });
			
			const auto function = createLLVMFunction(module, functionType, llvm::Function::ExternalLinkage, functionName);
			
			module.getFunctionMap().insert(functionName, function);
			
			return function;
		}
		
		llvm::Function* getExceptionRethrowFunction(Module& module) {
			const std::string functionName = "__loci_rethrow";
			const auto result = module.getFunctionMap().tryGet(functionName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			const auto functionType = typeGen.getVoidFunctionType(std::vector<llvm::Type*> { typeGen.getI8PtrType() });
			
			const auto function = createLLVMFunction(module, functionType, llvm::Function::ExternalLinkage, functionName);
			
			module.getFunctionMap().insert(functionName, function);
			
			return function;
		}
		
		llvm::Function* getExceptionPersonalityFunction(Module& module) {
			const std::string functionName = "__loci_personality_v0";
			const auto result = module.getFunctionMap().tryGet(functionName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			const auto functionType = typeGen.getVarArgsFunctionType(typeGen.getI32Type(), std::vector<llvm::Type*> {});
			
			const auto function = createLLVMFunction(module, functionType, llvm::Function::ExternalLinkage, functionName);
			function->addFnAttr(llvm::Attribute::NoUnwind);
			
			module.getFunctionMap().insert(functionName, function);
			
			return function;
		}
		
		llvm::Function* getExceptionPtrFunction(Module& module) {
			const std::string functionName = "__loci_get_exception";
			const auto result = module.getFunctionMap().tryGet(functionName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			const auto functionType = typeGen.getFunctionType(typeGen.getI8PtrType(), std::vector<llvm::Type*>{typeGen.getI8PtrType()});
			
			const auto function = createLLVMFunction(module, functionType, llvm::Function::ExternalLinkage, functionName);
			function->addFnAttr(llvm::Attribute::NoUnwind);
			
			module.getFunctionMap().insert(functionName, function);
			
			return function;
		}
		
		void genLandingPad(Function& function, bool isRethrow) {
			auto& module = function.module();
			const auto& unwindStack = function.unwindStack();
			
			TypeGenerator typeGen(module);
			const auto landingPadType = typeGen.getStructType(std::vector<llvm::Type*> {typeGen.getI8PtrType(), typeGen.getI32Type()});
			const auto personalityFunction = getExceptionPersonalityFunction(module);
			
			// Find all catch types on the stack.
			std::vector<llvm::Constant*> catchTypes;
			
			for (size_t i = 0; i < unwindStack.size(); i++) {
				const size_t pos = unwindStack.size() - i - 1;
				const auto& action = unwindStack.at(pos);
				
				if (action.isCatch()) {
					catchTypes.push_back(action.catchTypeInfo());
				}
			}
			
			const auto landingPad = function.getBuilder().CreateLandingPad(landingPadType, personalityFunction, catchTypes.size());
			landingPad->setCleanup(true);
			
			for (const auto& catchType: catchTypes) {
				landingPad->addClause(ConstantGenerator(module).getPointerCast(catchType, typeGen.getI8PtrType()));
			}
			
			// Set the exception information.
			function.getBuilder().CreateStore(landingPad, function.exceptionInfo());
			
			// Set the unwind state.
			setCurrentUnwindState(function, isRethrow ? UnwindStateRethrow : UnwindStateThrow);
			
			// Jump to first unwind block.
			function.getBuilder().CreateBr(getNextUnwindBlock(function));
		}
		
		void scheduleExceptionDestroy(Function& function, llvm::Value* exceptionPtrValue) {
			const auto currentBB = function.getSelectedBasicBlock();
			
			const auto unwindBB = function.createBasicBlock("exceptionDestroy");
			function.selectBasicBlock(unwindBB);
			
			const auto isRethrowBB = function.createBasicBlock("");
			const auto isNotRethrowBB = function.createBasicBlock("");
			
			const auto nextUnwindBB = getNextUnwindBlock(function);
			
			const auto isRethrowState = getIsCurrentUnwindState(function, UnwindStateRethrow);
			function.getBuilder().CreateCondBr(isRethrowState, isRethrowBB, isNotRethrowBB);
			
			function.selectBasicBlock(isRethrowBB);
			// Set state to 'throw' so that outer catch blocks don't
			// think their exception has been rethrown.
			setCurrentUnwindState(function, UnwindStateThrow);
			function.getBuilder().CreateBr(nextUnwindBB);
			
			// If this isn't a rethrow, destroy the exception.
			function.selectBasicBlock(isNotRethrowBB);
			function.getBuilder().CreateCall(getExceptionFreeFunction(function.module()), std::vector<llvm::Value*>{ exceptionPtrValue });
			function.getBuilder().CreateBr(nextUnwindBB);
			
			function.unwindStack().push_back(UnwindAction::CatchBlock(unwindBB, exceptionPtrValue));
			
			function.selectBasicBlock(currentBB);
		}
		
		llvm::Constant* getTypeNameGlobal(Module& module, const std::string& typeName) {
			ConstantGenerator constGen(module);
			TypeGenerator typeGen(module);
			
			// Generate the name of the exception type as a constant C string.
			const auto typeNameConstant = constGen.getString(typeName.c_str());
			const auto typeNameType = typeGen.getArrayType(typeGen.getI8Type(), typeName.size() + 1);
			const auto typeNameGlobal = module.createConstGlobal("type_name", typeNameType, llvm::GlobalValue::PrivateLinkage, typeNameConstant);
			typeNameGlobal->setAlignment(1);
			
			// Convert array to a pointer.
			return constGen.getGetElementPtr(typeNameGlobal, std::vector<llvm::Constant*>{constGen.getI32(0), constGen.getI32(0)});
		}
		
		llvm::Constant* genCatchInfo(Module& module, SEM::TypeInstance* catchTypeInstance) {
			assert(catchTypeInstance->isException());
			
			const auto typeName = catchTypeInstance->name().toString();
			const auto typeNameGlobalPtr = getTypeNameGlobal(module, typeName);
			
			ConstantGenerator constGen(module);
			TypeGenerator typeGen(module);
			
			// Create a constant struct {i32, i8*} for the catch type info.
			const auto typeInfoType = typeGen.getStructType(std::vector<llvm::Type*>{typeGen.getI32Type(), typeGen.getI8PtrType()});
			
			// Calculate offset to check based on number of parents.
			size_t offset = 0;
			auto currentInstance = catchTypeInstance;
			while (currentInstance->parent() != NULL) {
				offset++;
				currentInstance = currentInstance->parent();
			}
			
			const auto castedTypeNamePtr = constGen.getPointerCast(typeNameGlobalPtr, typeGen.getI8PtrType());
			const auto typeInfoValue = constGen.getStruct(typeInfoType, std::vector<llvm::Constant*>{constGen.getI32(offset), castedTypeNamePtr});
			
			const auto typeInfoGlobal = module.createConstGlobal("catch_type_info", typeInfoType, llvm::GlobalValue::PrivateLinkage, typeInfoValue);
			
			return constGen.getGetElementPtr(typeInfoGlobal, std::vector<llvm::Constant*>{constGen.getI32(0), constGen.getI32(0)});
		}
		
		llvm::Constant* genThrowInfo(Module& module, SEM::TypeInstance* throwTypeInstance) {
			assert(throwTypeInstance->isException());
			
			std::vector<std::string> typeNames;
			
			// Add type names in REVERSE order.
			auto currentInstance = throwTypeInstance;
			while (currentInstance != NULL) {
				typeNames.push_back(currentInstance->name().toString());
				currentInstance = currentInstance->parent();
			}
			
			assert(!typeNames.empty());
			
			// Since type names were added in reverse
			// order, fix this by reversing the array.
			std::reverse(std::begin(typeNames), std::end(typeNames));
			
			ConstantGenerator constGen(module);
			TypeGenerator typeGen(module);
			
			// Create a constant struct {i32, i8*} for the throw type info.
			const auto typeNameArrayType = typeGen.getArrayType(typeGen.getI8PtrType(), typeNames.size());
			const auto typeInfoType = typeGen.getStructType(std::vector<llvm::Type*>{typeGen.getI32Type(), typeNameArrayType});
			
			std::vector<llvm::Constant*> typeNameConstants;
			for (const auto& typeName: typeNames) {
				const auto typeNameGlobalPtr = getTypeNameGlobal(module, typeName);
				const auto castedTypeNamePtr = constGen.getPointerCast(typeNameGlobalPtr, typeGen.getI8PtrType());
				typeNameConstants.push_back(castedTypeNamePtr);
			}
			
			const auto typeNameArray = constGen.getArray(typeNameArrayType, typeNameConstants);
			const auto typeInfoValue = constGen.getStruct(typeInfoType, std::vector<llvm::Constant*>{constGen.getI32(typeNames.size()), typeNameArray});
			
			const auto typeInfoGlobal = module.createConstGlobal("throw_type_info", typeInfoType, llvm::GlobalValue::PrivateLinkage, typeInfoValue);
			
			return constGen.getGetElementPtr(typeInfoGlobal, std::vector<llvm::Constant*>{constGen.getI32(0), constGen.getI32(0)});
		}
		
		TryScope::TryScope(UnwindStack& unwindStack, llvm::BasicBlock* catchBlock, const std::vector<llvm::Constant*>& catchTypeList)
			: unwindStack_(unwindStack), catchCount_(catchTypeList.size()) {
				assert(!catchTypeList.empty());
				for (size_t i = 0; i < catchTypeList.size(); i++) {
					// Push in reverse order.
					const auto catchType = catchTypeList.at(catchTypeList.size() - i - 1);
					unwindStack_.push_back(UnwindAction::CatchException(catchBlock, catchType));
				}
			}
		
		TryScope::~TryScope() {
			for (size_t i = 0; i < catchCount_; i++) {
				assert(unwindStack_.back().isCatch());
				unwindStack_.pop_back();
			}
		}
		
	}
	
}

