#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Support/IRBuilder.h>

#include <assert.h>

#include <string>
#include <vector>

#include <Locic/SEM.hpp>
#include <Locic/CodeGen/Primitives.hpp>
#include <Locic/CodeGen/TargetInfo.hpp>

namespace Locic {

	namespace CodeGen {
	
		void createPrimitiveMethods(llvm::Module& module) {
			TargetInfo targetInfo(module.getTargetTriple());
			
			llvm::LLVMContext& context = module.getContext();
			
			llvm::IRBuilder<> builder(context);
			
			std::vector< std::pair<std::string, std::size_t> > sizes;
			sizes.push_back(std::make_pair("short", targetInfo.getPrimitiveSize("short")));
			sizes.push_back(std::make_pair("int", targetInfo.getPrimitiveSize("int")));
			sizes.push_back(std::make_pair("long", targetInfo.getPrimitiveSize("long")));
			sizes.push_back(std::make_pair("longlong", targetInfo.getPrimitiveSize("longlong")));
			const size_t integerWidth = targetInfo.getPrimitiveSize("int");
			llvm::Type* boolType = llvm::Type::getInt1Ty(context);
			llvm::Type* integerType = llvm::IntegerType::get(context, integerWidth);
			
			{
				const std::string functionName = "bool__not";
				llvm::Type* ptrType = boolType->getPointerTo();
				llvm::FunctionType* functionType = llvm::FunctionType::get(boolType, std::vector<llvm::Type*>(1, ptrType), false);
				llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, &module);
				llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(context, "entry", function);
				builder.SetInsertPoint(basicBlock);
				builder.CreateRet(builder.CreateNot(builder.CreateLoad(function->arg_begin())));
			}
			
			// Generate integer methods.
			for(std::size_t i = 0; i < sizes.size(); i++) {
				const std::string name = sizes.at(i).first;
				const std::size_t size = sizes.at(i).second;
				llvm::Type* intType = llvm::IntegerType::get(context, size);
				llvm::Type* ptrType = intType->getPointerTo();
				{
					const std::string functionName = name + "__implicitCopy";
					llvm::FunctionType* functionType = llvm::FunctionType::get(intType, std::vector<llvm::Type*>(1, ptrType), false);
					llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, &module);
					llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(context, "entry", function);
					builder.SetInsertPoint(basicBlock);
					builder.CreateRet(builder.CreateLoad(function->arg_begin()));
				}
				{
					const std::string functionName = name + "__add";
					std::vector<llvm::Type*> argumentTypes;
					argumentTypes.push_back(ptrType);
					argumentTypes.push_back(intType);
					llvm::FunctionType* functionType = llvm::FunctionType::get(intType, argumentTypes, false);
					llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, &module);
					llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(context, "entry", function);
					builder.SetInsertPoint(basicBlock);
					llvm::Function::arg_iterator arg = function->arg_begin();
					llvm::Value* firstArg = builder.CreateLoad(arg++);
					llvm::Value* secondArg = arg;
					builder.CreateRet(builder.CreateAdd(firstArg, secondArg));
				}
				{
					const std::string functionName = name + "__subtract";
					std::vector<llvm::Type*> argumentTypes;
					argumentTypes.push_back(ptrType);
					argumentTypes.push_back(intType);
					llvm::FunctionType* functionType = llvm::FunctionType::get(intType, argumentTypes, false);
					llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, &module);
					llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(context, "entry", function);
					builder.SetInsertPoint(basicBlock);
					llvm::Function::arg_iterator arg = function->arg_begin();
					llvm::Value* firstArg = builder.CreateLoad(arg++);
					llvm::Value* secondArg = arg;
					builder.CreateRet(builder.CreateSub(firstArg, secondArg));
				}
				{
					const std::string functionName = name + "__multiply";
					std::vector<llvm::Type*> argumentTypes;
					argumentTypes.push_back(ptrType);
					argumentTypes.push_back(intType);
					llvm::FunctionType* functionType = llvm::FunctionType::get(intType, argumentTypes, false);
					llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, &module);
					llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(context, "entry", function);
					builder.SetInsertPoint(basicBlock);
					llvm::Function::arg_iterator arg = function->arg_begin();
					llvm::Value* firstArg = builder.CreateLoad(arg++);
					llvm::Value* secondArg = arg;
					builder.CreateRet(builder.CreateMul(firstArg, secondArg));
				}
				{
					const std::string functionName = name + "__divide";
					std::vector<llvm::Type*> argumentTypes;
					argumentTypes.push_back(ptrType);
					argumentTypes.push_back(intType);
					llvm::FunctionType* functionType = llvm::FunctionType::get(intType, argumentTypes, false);
					llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, &module);
					llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(context, "entry", function);
					builder.SetInsertPoint(basicBlock);
					llvm::Function::arg_iterator arg = function->arg_begin();
					llvm::Value* firstArg = builder.CreateLoad(arg++);
					llvm::Value* secondArg = arg;
					builder.CreateRet(builder.CreateSDiv(firstArg, secondArg));
				}
				{
					const std::string functionName = name + "__modulo";
					std::vector<llvm::Type*> argumentTypes;
					argumentTypes.push_back(ptrType);
					argumentTypes.push_back(intType);
					llvm::FunctionType* functionType = llvm::FunctionType::get(intType, argumentTypes, false);
					llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, &module);
					llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(context, "entry", function);
					builder.SetInsertPoint(basicBlock);
					llvm::Function::arg_iterator arg = function->arg_begin();
					llvm::Value* firstArg = builder.CreateLoad(arg++);
					llvm::Value* secondArg = arg;
					builder.CreateRet(builder.CreateSRem(firstArg, secondArg));
				}
				{
					const std::string functionName = name + "__compare";
					std::vector<llvm::Type*> argumentTypes;
					argumentTypes.push_back(ptrType);
					argumentTypes.push_back(intType);
					llvm::FunctionType* functionType = llvm::FunctionType::get(integerType, argumentTypes, false);
					llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, &module);
					llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(context, "entry", function);
					builder.SetInsertPoint(basicBlock);
					llvm::Function::arg_iterator arg = function->arg_begin();
					llvm::Value* firstArg = builder.CreateLoad(arg++);
					llvm::Value* secondArg = arg;
					llvm::Value* isLessThan = builder.CreateICmpSLT(firstArg, secondArg);
					llvm::Value* isGreaterThan = builder.CreateICmpSGT(firstArg, secondArg);
					llvm::Value* minusOne = llvm::ConstantInt::get(context, llvm::APInt(integerWidth, -1));
					llvm::Value* zero = llvm::ConstantInt::get(context, llvm::APInt(integerWidth, 0));
					llvm::Value* plusOne = llvm::ConstantInt::get(context, llvm::APInt(integerWidth, 1));
					llvm::Value* returnValue =
						builder.CreateSelect(isLessThan, minusOne,
											 builder.CreateSelect(isGreaterThan, plusOne, zero));
					builder.CreateRet(returnValue);
				}
				{
					const std::string functionName = name + "__isZero";
					std::vector<llvm::Type*> argumentTypes;
					argumentTypes.push_back(ptrType);
					llvm::FunctionType* functionType = llvm::FunctionType::get(boolType, argumentTypes, false);
					llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, &module);
					llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(context, "entry", function);
					builder.SetInsertPoint(basicBlock);
					llvm::Value* arg = builder.CreateLoad(function->arg_begin());
					llvm::Value* zero = llvm::ConstantInt::get(context, llvm::APInt(size, 0));
					builder.CreateRet(builder.CreateICmpEQ(arg, zero));
				}
				{
					const std::string functionName = name + "__isPositive";
					std::vector<llvm::Type*> argumentTypes;
					argumentTypes.push_back(ptrType);
					llvm::FunctionType* functionType = llvm::FunctionType::get(boolType, argumentTypes, false);
					llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, &module);
					llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(context, "entry", function);
					builder.SetInsertPoint(basicBlock);
					llvm::Value* arg = builder.CreateLoad(function->arg_begin());
					llvm::Value* zero = llvm::ConstantInt::get(context, llvm::APInt(size, 0));
					builder.CreateRet(builder.CreateICmpSGT(arg, zero));
				}
				{
					const std::string functionName = name + "__isNegative";
					std::vector<llvm::Type*> argumentTypes;
					argumentTypes.push_back(ptrType);
					llvm::FunctionType* functionType = llvm::FunctionType::get(boolType, argumentTypes, false);
					llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, &module);
					llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(context, "entry", function);
					builder.SetInsertPoint(basicBlock);
					llvm::Value* arg = builder.CreateLoad(function->arg_begin());
					llvm::Value* zero = llvm::ConstantInt::get(context, llvm::APInt(size, 0));
					builder.CreateRet(builder.CreateICmpSLT(arg, zero));
				}
				{
					const std::string functionName = name + "__abs";
					llvm::FunctionType* functionType = llvm::FunctionType::get(intType, std::vector<llvm::Type*>(1, ptrType), false);
					llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, &module);
					llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(context, "entry", function);
					builder.SetInsertPoint(basicBlock);
					llvm::Function::arg_iterator arg = function->arg_begin();
					llvm::Value* firstArg = builder.CreateLoad(arg++);
					// Generates: (value < 0) ? -value : value
					llvm::Value* lessThanZero = builder.CreateICmpSLT(firstArg, llvm::ConstantInt::get(context, llvm::APInt(size, 0)));
					builder.CreateRet(builder.CreateSelect(lessThanZero, builder.CreateNeg(firstArg), firstArg));
				}
				{
					const size_t sizeTypeWidth = targetInfo.getPrimitiveSize("size_t");
					llvm::Type* sizeType = llvm::IntegerType::get(context, sizeTypeWidth);
					const std::string functionName = std::string("__BUILTIN__") + name + "__sizeof";
					llvm::FunctionType* functionType = llvm::FunctionType::get(sizeType, std::vector<llvm::Type*>(), false);
					llvm::Function* function = llvm::Function::Create(functionType, llvm::Function::LinkOnceODRLinkage, functionName, &module);
					llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(context, "entry", function);
					builder.SetInsertPoint(basicBlock);
					builder.CreateRet(llvm::ConstantInt::get(context, llvm::APInt(sizeTypeWidth, size / 8)));
				}
			}
		}
		
		llvm::Type* createPrimitiveType(llvm::Module& module, SEM::TypeInstance* type) {
			TargetInfo targetInfo(module.getTargetTriple());
			
			llvm::LLVMContext& context = module.getContext();
			
			const std::string name = type->name().toString();
			
			if(name == "::bool") return llvm::Type::getInt1Ty(context);
			
			if(name == "::char") return llvm::Type::getInt8Ty(context);
			
			if(name == "::short") return llvm::IntegerType::get(context, targetInfo.getPrimitiveSize("short"));
			
			if(name == "::int") return llvm::IntegerType::get(context, targetInfo.getPrimitiveSize("int"));
			
			if(name == "::long") return llvm::IntegerType::get(context, targetInfo.getPrimitiveSize("long"));
			
			if(name == "::longlong") return llvm::IntegerType::get(context, targetInfo.getPrimitiveSize("longlong"));
			
			if(name == "::float") return llvm::Type::getFloatTy(context);
			
			if(name == "::double") return llvm::Type::getDoubleTy(context);
			
			if(name == "::longdouble") return llvm::Type::getFP128Ty(context);
			
			assert(false && "Unrecognised primitive type");
			return NULL;
		}
		
	}
	
}

