#ifndef LOCIC_CODEGEN_INTERFACE_HPP
#define LOCIC_CODEGEN_INTERFACE_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/Module.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm::Value* makeTypeInfoValue(Function& function, llvm::Value* vtablePointer, llvm::Value* templateGenerator);
		
		/* Interface struct type is:
			struct {
				i8* context;
				struct {
					__vtable_type* vtable;
					struct {
						void* rootFn;
						uint32_t path;
					} templateGenerator;
				} typeInfo;
			};
		*/
		TypePair interfaceStructType(Module& module);
		
		llvm::Value* makeInterfaceStructValue(Function& function, llvm::Value* contextPointer, llvm::Value* typeInfoValue);
		
		/* Interface method type is:
			struct {
				struct {
					i8* context;
					struct {
						__vtable_type* vtable;
						struct {
							void* rootFn;
							uint32_t path;
						} templateGenerator;
					} typeInfo;
				} interfaceStruct;
				i64 methodHash;
			};
		*/
		TypePair interfaceMethodType(Module& module);
		
		llvm::Value* makeInterfaceMethodValue(Function& function, llvm::Value* interfaceStructValue, llvm::Value* hashValue);
		
		/* Static interface method type is:
			struct {
				struct {
					__vtable_type* vtable;
					struct {
						void* rootFn;
						uint32_t path;
					} templateGenerator;
				} typeInfo;
				i64 methodHash;
			};
		*/
		TypePair staticInterfaceMethodType(Module& module);
		
		llvm::Value* makeStaticInterfaceMethodValue(Function& function, llvm::Value* typeInfoValue, llvm::Value* hashValue);
	}
	
}

#endif
