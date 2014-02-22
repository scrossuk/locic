#include <vector>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Destructor.hpp>
#include <locic/CodeGen/Exception.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/LLVMIncludes.hpp>
#include <locic/CodeGen/Module.hpp>
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
		
		llvm::Function* getExceptionPersonalityFunction(Module& module) {
			const std::string functionName = "__loci_personality_v0";
			const auto result = module.getFunctionMap().tryGet(functionName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			const auto functionType = typeGen.getVarArgsFunctionType(typeGen.getI32Type(), std::vector<llvm::Type*> {});
			
			const auto function = createLLVMFunction(module, functionType, llvm::Function::ExternalLinkage, functionName);
			
			module.getFunctionMap().insert(functionName, function);
			
			return function;
		}
		
		llvm::Function* getBeginCatchFunction(Module& module) {
			const std::string functionName = "__loci_begin_catch";
			const auto result = module.getFunctionMap().tryGet(functionName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			const auto functionType = typeGen.getFunctionType(typeGen.getI8PtrType(), std::vector<llvm::Type*>{typeGen.getI8PtrType()});
			
			const auto function = createLLVMFunction(module, functionType, llvm::Function::ExternalLinkage, functionName);
			
			module.getFunctionMap().insert(functionName, function);
			
			return function;
		}
		
		llvm::Function* getEndCatchFunction(Module& module) {
			const std::string functionName = "__loci_end_catch";
			const auto result = module.getFunctionMap().tryGet(functionName);
			
			if (result.hasValue()) {
				return result.getValue();
			}
			
			TypeGenerator typeGen(module);
			const auto functionType = typeGen.getVoidFunctionType(std::vector<llvm::Type*>{});
			
			const auto function = createLLVMFunction(module, functionType, llvm::Function::ExternalLinkage, functionName);
			
			module.getFunctionMap().insert(functionName, function);
			
			return function;
		}
		
		void genLandingPad(Function& function) {
			auto& module = function.getModule();
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
			
			// Record exception info.
			function.getBuilder().CreateStore(landingPad, function.exceptionInfo());
			
			// Unwind stack due to exception.
			genExceptionUnwind(function);
		}
		
		void genExceptionUnwind(Function& function) {
			const auto& unwindStack = function.unwindStack();
			llvm::BasicBlock* catchBlock = nullptr;
			
			// Call all destructors until the next catch block.
			for (size_t i = 0; i < unwindStack.size(); i++) {
				const size_t pos = unwindStack.size() - i - 1;
				const auto& action = unwindStack.at(pos);
				
				if (action.isCatch()) {
					catchBlock = action.catchBlock();
					break;
				}
				
				if (!action.isDestructor()) {
					continue;
				}
				
				genDestructorCall(function, action.destroyType(), action.destroyValue());
			}
			
			if (catchBlock != nullptr) {
				// Jump to the next catch block.
				function.getBuilder().CreateBr(catchBlock);
			} else {
				// There isn't a catch block; resume unwinding.
				const auto exceptionInfo = function.getBuilder().CreateLoad(function.exceptionInfo());
				function.getBuilder().CreateResume(exceptionInfo);
			}
		}
		
		llvm::Constant* genCatchInfo(Module& module, SEM::TypeInstance* catchTypeInstance) {
			const auto typeName = catchTypeInstance->name().toString();
			
			ConstantGenerator constGen(module);
			TypeGenerator typeGen(module);
			
			// Generate the name of the exception type as a constant C string.
			const auto typeNameConstant = constGen.getString(typeName.c_str());
			const auto typeNameType = typeGen.getArrayType(typeGen.getI8Type(), typeName.size() + 1);
			const auto typeNameGlobal = module.createConstGlobal("catch_type_name", typeNameType, llvm::GlobalValue::PrivateLinkage, typeNameConstant);
			typeNameGlobal->setAlignment(1);
			
			// Convert array to a pointer.
			const auto typeNameGlobalPtr = constGen.getGetElementPtr(typeNameGlobal, std::vector<llvm::Constant*>{constGen.getI32(0), constGen.getI32(0)});
			
			// Create a constant struct {i32, i8*} for the catch type info.
			const auto typeInfoType = typeGen.getStructType(std::vector<llvm::Type*>{typeGen.getI32Type(), typeGen.getI8PtrType()});
			
			const auto castedTypeNamePtr = constGen.getPointerCast(typeNameGlobalPtr, typeGen.getI8PtrType());
			const auto typeInfoValue = constGen.getStruct(typeInfoType, std::vector<llvm::Constant*>{constGen.getI32(0), castedTypeNamePtr});
			
			const auto typeInfoGlobal = module.createConstGlobal("catch_type_info", typeInfoType, llvm::GlobalValue::PrivateLinkage, typeInfoValue);
			
			return constGen.getGetElementPtr(typeInfoGlobal, std::vector<llvm::Constant*>{constGen.getI32(0), constGen.getI32(0)});
		}
		
		llvm::Constant* genThrowInfo(Module& module, SEM::TypeInstance* throwTypeInstance) {
			const auto typeName = throwTypeInstance->name().toString();
			
			ConstantGenerator constGen(module);
			TypeGenerator typeGen(module);
			
			// Generate the name of the exception type as a constant C string.
			const auto typeNameConstant = constGen.getString(typeName.c_str());
			const auto typeNameType = typeGen.getArrayType(typeGen.getI8Type(), typeName.size() + 1);
			const auto typeNameGlobal = module.createConstGlobal("throw_type_name", typeNameType, llvm::GlobalValue::PrivateLinkage, typeNameConstant);
			typeNameGlobal->setAlignment(1);
			
			// Convert array to a pointer.
			const auto typeNameGlobalPtr = constGen.getGetElementPtr(typeNameGlobal, std::vector<llvm::Constant*>{constGen.getI32(0), constGen.getI32(0)});
			
			// Create a constant struct {i32, i8*} for the throw type info.
			const auto typeInfoType = typeGen.getStructType(std::vector<llvm::Type*>{typeGen.getI32Type(), typeGen.getI8PtrType()});
			
			const auto castedTypeNamePtr = constGen.getPointerCast(typeNameGlobalPtr, typeGen.getI8PtrType());
			const auto typeInfoValue = constGen.getStruct(typeInfoType, std::vector<llvm::Constant*>{constGen.getI32(1), castedTypeNamePtr});
			
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

