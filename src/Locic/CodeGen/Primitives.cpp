#include <llvm/LLVMContext.h>
#include <llvm/IRBuilder.h>

#include <assert.h>

#include <string>
#include <vector>

#include <Locic/CodeGen/ConstantGenerator.hpp>
#include <Locic/CodeGen/Function.hpp>
#include <Locic/CodeGen/Module.hpp>
#include <Locic/CodeGen/Primitives.hpp>
#include <Locic/CodeGen/TargetInfo.hpp>
#include <Locic/CodeGen/TypeGenerator.hpp>

namespace Locic {

	namespace CodeGen {
	
		void createPrimitiveSizeOf(Module& module, const std::string& name, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			Function function(module, llvmFunction, ArgInfo::None());
			
			LOG(LOG_INFO, "Generating sizeof() for primitive type '%s'.",
				name.c_str());
				
			function.getBuilder().CreateRet(
				ConstantGenerator(module).getSize(module.getTargetInfo().getPrimitiveSizeInBytes(name)));
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
			assert((methodName == "Default") xor isUnaryOp(methodName) xor isBinaryOp(methodName));
			
			if (methodName == "Default") {
				return ArgInfo::None();
			}
			
			const bool hasReturnVarArg = false;
			const bool hasContextArg = true;
			const size_t numStandardArguments =
				isUnaryOp(methodName) ? 0 : 1;
			return ArgInfo(hasReturnVarArg, hasContextArg, numStandardArguments);
		}
		
		void createPrimitiveMethod(Module& module, const std::string& typeName, const std::string& methodName, llvm::Function& llvmFunction) {
			assert(llvmFunction.isDeclaration());
			
			Function function(module, llvmFunction, getPrimitiveMethodArgInfo(methodName));
			
			llvm::IRBuilder<>& builder = function.getBuilder();
			
			llvm::Value* methodOwner =
				methodName == "Default" ?
					NULL :
					builder.CreateLoad(function.getContextValue());
			
			// TODO: generate correct ops for unsigned and floating point types.
			if (methodName == "Default") {
				llvm::Value* zero = ConstantGenerator(module).getPrimitiveInt(typeName, 0);
				builder.CreateRet(zero);
			} else if (isUnaryOp(methodName)) {
				llvm::Value* zero = ConstantGenerator(module).getPrimitiveInt(typeName, 0);
				
				if (methodName == "implicitCopy") {
					builder.CreateRet(methodOwner);
				} else if (methodName == "not") {
					assert(typeName == "bool");
					builder.CreateRet(builder.CreateNot(methodOwner));
				} else if (methodName == "isZero") {
					builder.CreateRet(builder.CreateICmpEQ(methodOwner, zero));
				} else if (methodName == "isPositive") {
					builder.CreateRet(builder.CreateICmpSGT(methodOwner, zero));
				} else if (methodName == "isNegative") {
					builder.CreateRet(builder.CreateICmpSLT(methodOwner, zero));
				} else if (methodName == "abs") {
					// Generates: (value < 0) ? -value : value.
					llvm::Value* lessThanZero = builder.CreateICmpSLT(methodOwner, zero);
					builder.CreateRet(
						builder.CreateSelect(lessThanZero, builder.CreateNeg(methodOwner), methodOwner));
				} else {
					assert(false && "Unknown primitive unary op.");
				}
			} else if (isBinaryOp(methodName)) {
				llvm::Value* operand = function.getArg(0);
				
				if (methodName == "add") {
					builder.CreateRet(
						builder.CreateAdd(methodOwner, operand));
				} else if (methodName == "subtract") {
					builder.CreateRet(
						builder.CreateSub(methodOwner, operand));
				} else if (methodName == "multiply") {
					builder.CreateRet(
						builder.CreateMul(methodOwner, operand));
				} else if (methodName == "divide") {
					builder.CreateRet(
						builder.CreateSDiv(methodOwner, operand));
				} else if (methodName == "modulo") {
					builder.CreateRet(
						builder.CreateSRem(methodOwner, operand));
				} else if (methodName == "compare") {
					llvm::Value* isLessThan = builder.CreateICmpSLT(methodOwner, operand);
					llvm::Value* isGreaterThan = builder.CreateICmpSGT(methodOwner, operand);
					llvm::Value* minusOne = ConstantGenerator(module).getPrimitiveInt("int", -1);
					llvm::Value* zero = ConstantGenerator(module).getPrimitiveInt("int", 0);
					llvm::Value* plusOne = ConstantGenerator(module).getPrimitiveInt("int", 1);
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
			
			LOG(LOG_INFO, "Generated primitive method:");
			llvmFunction.dump();
			
			// Check the generated function is correct.
			function.verify();
		}
		
		llvm::Type* getPrimitiveType(const Module& module, const std::string& name) {
			if (name == "bool") {
				return TypeGenerator(module).getI1Type();
			}
			
			if (name == "char") {
				return TypeGenerator(module).getI8Type();
			}
			
			if (name == "short" || name == "int" || name == "long" || name == "longlong" || name == "size_t") {
				return TypeGenerator(module).getIntType(module.getTargetInfo().getPrimitiveSize(name));
			}
			if (name == "float") {
				return TypeGenerator(module).getFloatType();
			}
			
			if (name == "double") {
				return TypeGenerator(module).getDoubleType();
			}
			
			if (name == "longdouble") {
				return TypeGenerator(module).getLongDoubleType();
			}
			
			assert(false && "Unrecognised primitive type");
			return NULL;
		}
		
	}
	
}

