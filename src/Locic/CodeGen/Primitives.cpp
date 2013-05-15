#include <llvm/LLVMContext.h>
#include <llvm/IRBuilder.h>

#include <assert.h>

#include <string>
#include <vector>

#include <Locic/SEM.hpp>
#include <Locic/CodeGen/Module.hpp>
#include <Locic/CodeGen/Primitives.hpp>
#include <Locic/CodeGen/TargetInfo.hpp>

namespace Locic {

	namespace CodeGen {
	
		void createPrimitiveSizeOf(Module& module, const std::string& name, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			Function genFunction(module, llvmFunction, ArgInfo::None());
			
			genFunction.selectBasicBlock(genFunction.createBasicBlock("entry"));
			
			LOG(LOG_INFO, "Generating sizeof() for primitive type '%s'.",
				name.c_str());
				
			genFunction.getBuilder().CreateRet(ConstantGenerator(module).getSize(
												   targetInfo.getPrimitiveSizeInBytes(name)));
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
		
		ArgInfo getPrimitiveMethodArgInfo(const std::string& methodName) {
			assert(isUnaryOp(methodName) xor isBinaryOp(methodName));
			const bool hasReturnVarArg = false;
			const bool hasContextArg = true;
			const size_t numStandardArguments =
				isUnaryOp(methodName) ? 0 : 1;
			return ArgInfo(hasReturnVarArg, hasContextArg, numStandardArguments);
		}
		
		void createPrimitiveMethod(Module& module, const std::string& typeName, const std::string& methodName, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			Function genFunction(module, llvmFunction, getPrimitiveMethodArgInfo(methodName));
			
			llvm::Value* methodOwner = builder.CreateLoad(function.getContextValue());
			
			// TODO: generate correct ops for unsigned and floating point types.
			if (isUnaryOp(methodName)) {
				llvm::Value* zero = ConstantGenerator(module).getPrimitiveInt(typeName, 0);
				
				if (methodName == "implicitCopy") {
					function.getBuilder().CreateRet(methodOwner);
				} else if (methodName == "not") {
					assert(typeName == "bool");
					function.getBuilder().CreateRet(builder.CreateNot(methodOwner));
				} else if (methodName == "isZero") {
					function.getBuilder().CreateRet(builder.CreateICmpEQ(methodOwner, zero));
				} else if (methodName == "isPositive") {
					function.getBuilder().CreateRet(builder.CreateICmpSGT(methodOwner, zero));
				} else if (methodName == "isNegative") {
					function.getBuilder().CreateRet(builder.CreateICmpSLT(methodOwner, zero));
				} else if (methodName == "abs") {
					// Generates: (value < 0) ? -value : value.
					llvm::Value* lessThanZero = function.getBuilder().CreateICmpSLT(methodOwner, zero);
					function.getBuilder().CreateRet(
						builder.CreateSelect(lessThanZero, builder.CreateNeg(methodOwner), methodOwner));
				} else {
					assert(false && "Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				llvm::Value* operand = function.getArg(0);
				
				if (methodName == "add") {
					function.getBuilder().CreateRet(
						function.getBuilder().CreateAdd(methodOwner, operand));
				} else if (methodName == "subtract") {
					function.getBuilder().CreateRet(
						function.getBuilder().CreateSub(methodOwner, operand));
				} else if (methodName == "multiply") {
					function.getBuilder().CreateRet(
						function.getBuilder().CreateMul(methodOwner, operand));
				} else if (methodName == "divide") {
					function.getBuilder().CreateRet(
						function.getBuilder().CreateSDiv(methodOwner, operand));
				} else if (methodName == "modulo") {
					function.getBuilder().CreateRet(
						function.getBuilder().CreateSRem(methodOwner, operand));
				} else if (methodName == "compare") {
					llvm::Value* isLessThan = function.getBuilder().CreateICmpSLT(methodOwner, operand);
					llvm::Value* isGreaterThan = function.getBuilder().CreateICmpSGT(methodOwner, operand);
					llvm::Value* minusOne = ConstantGenerator(module).getPrimitiveInt("int", -1);
					llvm::Value* zero = ConstantGenerator(module).getPrimitiveInt("int", 0);
					llvm::Value* plusOne = ConstantGenerator(module).getPrimitiveInt("int", 1);
					llvm::Value* returnValue =
						function.getBuilder().CreateSelect(isLessThan, minusOne,
														   function.getBuilder().CreateSelect(isGreaterThan, plusOne, zero));
					function.getBuilder().CreateRet(returnValue);
				} else {
					assert(false && "Unknown primitive binary op.");
				}
			} else {
				LOG(LOG_INFO, "Unknown primitive method: %s::%s.",
					typeName.c_str(), methodName.c_str());
				assert(false && "Unknown primitive method.");
			}
		}
		
		llvm::Type* getPrimitiveType(const Module& module, SEM::TypeInstance* type) {
			const std::string name = type->name().last();
			
			/*if (name == "bool") {
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
			
			assert(false && "Unrecognised primitive type");*/
			return TypeGenerator(module).getPrimitiveType(name);
		}
		
	}
	
}

