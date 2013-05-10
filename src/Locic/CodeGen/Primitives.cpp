#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/IRBuilder.h>

#include <assert.h>

#include <string>
#include <vector>

#include <Locic/SEM.hpp>
#include <Locic/CodeGen/Primitives.hpp>
#include <Locic/CodeGen/TargetInfo.hpp>

namespace Locic {

	namespace CodeGen {
	
		void createPrimitiveSizeOf(llvm::Module& module, const std::string& name, llvm::Function* function) {
			assert(function->isDeclaration());
			
			llvm::LLVMContext& context = module.getContext();
			TargetInfo targetInfo(module.getTargetTriple());
			
			llvm::IRBuilder<> builder(context);
			
			const size_t sizeTypeWidth = targetInfo.getPrimitiveSize("size_t");
			
			llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(context, "entry", function);
			builder.SetInsertPoint(basicBlock);
			
			LOG(LOG_INFO, "Generating sizeof() for primitive type '%s'.",
				name.c_str());
				
			const size_t size = targetInfo.getPrimitiveSize(name);
			assert((size % 8) == 0);
			builder.CreateRet(llvm::ConstantInt::get(context, llvm::APInt(sizeTypeWidth, size / 8)));
		}
		
		bool isIntegerType(const std::string& name) {
			return name == "short" || name == "int" || name == "long" || name == "longlong";
		}
		
		bool isUnaryOp(const std::string& methodName) {
			return methodName == "implicitCopy" ||
				   methodName == "not" ||
				   methodName == "isZero" ||
				   methodName == "isPositive" ||
				   methodName == "isNegative" ||
				   methodName == "abs";
		}
		
		bool isBinaryOp(const std::string& methodName) {
			return methodName == "add" ||
				   methodName == "subtract" ||
				   methodName == "multiply" ||
				   methodName == "divide" ||
				   methodName == "modulo" ||
				   methodName == "compare";
		}
		
		void createPrimitiveMethod(llvm::Module& module, const std::string& typeName, const std::string& methodName, llvm::Function* function) {
			assert(function->isDeclaration());
			
			llvm::LLVMContext& context = module.getContext();
			TargetInfo targetInfo(module.getTargetTriple());
			
			llvm::IRBuilder<> builder(context);
			
			llvm::BasicBlock* basicBlock = llvm::BasicBlock::Create(context, "entry", function);
			builder.SetInsertPoint(basicBlock);
			
			const size_t integerWidth = targetInfo.getPrimitiveSize("int");
			const size_t size = targetInfo.getPrimitiveSize(typeName);
			
			// TODO: generate correct ops for unsigned and floating point types.
			if (isUnaryOp(methodName)) {
				llvm::Value* arg = builder.CreateLoad(function->arg_begin());
				llvm::Value* zero = llvm::ConstantInt::get(context, llvm::APInt(size, 0));
				
				if (methodName == "implicitCopy") {
					builder.CreateRet(arg);
				} else if (methodName == "not") {
					assert(typeName == "bool");
					builder.CreateRet(builder.CreateNot(arg));
				} else if (methodName == "isZero") {
					builder.CreateRet(builder.CreateICmpEQ(arg, zero));
				} else if (methodName == "isPositive") {
					builder.CreateRet(builder.CreateICmpSGT(arg, zero));
				} else if (methodName == "isNegative") {
					builder.CreateRet(builder.CreateICmpSLT(arg, zero));
				} else if (methodName == "abs") {
					// Generates: (value < 0) ? -value : value.
					llvm::Value* lessThanZero = builder.CreateICmpSLT(arg, zero);
					builder.CreateRet(builder.CreateSelect(lessThanZero, builder.CreateNeg(arg), arg));
				} else {
					assert(false && "Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				llvm::Function::arg_iterator arg = function->arg_begin();
				llvm::Value* firstArg = builder.CreateLoad(arg++);
				llvm::Value* secondArg = arg;
				
				if (methodName == "add") {
					builder.CreateRet(builder.CreateAdd(firstArg, secondArg));
				} else if (methodName == "subtract") {
					builder.CreateRet(builder.CreateSub(firstArg, secondArg));
				} else if (methodName == "multiply") {
					builder.CreateRet(builder.CreateMul(firstArg, secondArg));
				} else if (methodName == "divide") {
					builder.CreateRet(builder.CreateSDiv(firstArg, secondArg));
				} else if (methodName == "modulo") {
					builder.CreateRet(builder.CreateSRem(firstArg, secondArg));
				} else if (methodName == "compare") {
					llvm::Value* isLessThan = builder.CreateICmpSLT(firstArg, secondArg);
					llvm::Value* isGreaterThan = builder.CreateICmpSGT(firstArg, secondArg);
					llvm::Value* minusOne = llvm::ConstantInt::get(context, llvm::APInt(integerWidth, -1));
					llvm::Value* zero = llvm::ConstantInt::get(context, llvm::APInt(integerWidth, 0));
					llvm::Value* plusOne = llvm::ConstantInt::get(context, llvm::APInt(integerWidth, 1));
					llvm::Value* returnValue =
						builder.CreateSelect(isLessThan, minusOne,
											 builder.CreateSelect(isGreaterThan, plusOne, zero));
					builder.CreateRet(returnValue);
				} else {
					assert(false && "Unknown primitive binary op.");
				}
			} else {
				LOG(LOG_INFO, "Unknown primitive method: %s::%s.",
					typeName.c_str(), methodName.c_str());
				assert(false && "Unknown primitive method.");
			}
		}
		
		llvm::Type* createPrimitiveType(llvm::Module& module, SEM::TypeInstance* type) {
			TargetInfo targetInfo(module.getTargetTriple());
			
			llvm::LLVMContext& context = module.getContext();
			
			const std::string name = type->name().last();
			
			if (name == "bool") {
				return llvm::Type::getInt1Ty(context);
			}
			
			if (name == "char") {
				return llvm::Type::getInt8Ty(context);
			}
			
			if (name == "short") {
				return llvm::IntegerType::get(context, targetInfo.getPrimitiveSize("short"));
			}
			
			if (name == "int") {
				return llvm::IntegerType::get(context, targetInfo.getPrimitiveSize("int"));
			}
			
			if (name == "long") {
				return llvm::IntegerType::get(context, targetInfo.getPrimitiveSize("long"));
			}
			
			if (name == "longlong") {
				return llvm::IntegerType::get(context, targetInfo.getPrimitiveSize("longlong"));
			}
			
			if (name == "float") {
				return llvm::Type::getFloatTy(context);
			}
			
			if (name == "double") {
				return llvm::Type::getDoubleTy(context);
			}
			
			if (name == "longdouble") {
				return llvm::Type::getFP128Ty(context);
			}
			
			assert(false && "Unrecognised primitive type");
			return NULL;
		}
		
	}
	
}

