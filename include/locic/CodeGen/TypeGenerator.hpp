#ifndef LOCIC_CODEGEN_TYPEGENERATOR_HPP
#define LOCIC_CODEGEN_TYPEGENERATOR_HPP

namespace locic {
	
	namespace CodeGen {
		
		class Module;
		
		class TypeGenerator {
			public:
				TypeGenerator(Module& module);
				
				llvm::Type* getVoidType() const;
				
				llvm::IntegerType* getI1Type() const;
				
				llvm::IntegerType* getI8Type() const;
				
				llvm::IntegerType* getI16Type() const;
				
				llvm::IntegerType* getI32Type() const;
				
				llvm::IntegerType* getI64Type() const;
				
				llvm::IntegerType* getIntType(size_t typeSizeInBits) const;
				
				llvm::IntegerType* getSizeTType() const;
				
				llvm::PointerType* getI8PtrType() const;
				
				llvm::Type* getFloatType() const;
				
				llvm::Type* getDoubleType() const;
				
				llvm::Type* getLongDoubleType() const;
				
				llvm::ArrayType* getArrayType(llvm::Type* elementType, size_t size) const;
				
				llvm::FunctionType* getVoidFunctionType(llvm::ArrayRef<llvm::Type*> args) const;
				
				llvm::FunctionType* getFunctionType(llvm::Type* returnType, llvm::ArrayRef<llvm::Type*> args, bool isVarArg = false) const;
				
				llvm::FunctionType* getVarArgsFunctionType(llvm::Type* returnType, llvm::ArrayRef<llvm::Type*> args) const;
				
				llvm::StructType* getStructType(llvm::ArrayRef<llvm::Type*> members) const;
				
				llvm::StructType* getOpaqueStructType() const;
				
				llvm::StructType* getForwardDeclaredStructType(const String& name) const;
				
			private:
				Module& module_;
				
		};
		
	}
	
}

#endif
