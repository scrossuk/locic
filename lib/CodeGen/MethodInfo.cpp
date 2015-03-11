#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/Function.hpp>
#include <locic/CodeGen/MethodInfo.hpp>
#include <locic/CodeGen/TypeGenerator.hpp>

namespace locic {
	
	namespace CodeGen {
		
		TypeInfoComponents getTypeInfoComponents(Function& function, llvm::Value* const typeInfoValue) {
			auto& builder = function.getBuilder();
			const auto vtablePointer = builder.CreateExtractValue(typeInfoValue, { 0 }, "vtable");
			const auto templateGenerator = builder.CreateExtractValue(typeInfoValue, { 1 }, "templateGenerator");
			return TypeInfoComponents(vtablePointer, templateGenerator);
		}
		
		VirtualObjectComponents getVirtualObjectComponents(Function& function, llvm::Value* const interfaceStructValue) {
			auto& builder = function.getBuilder();
			const auto objectPointer = builder.CreateExtractValue(interfaceStructValue, { 0 }, "object");
			const auto typeInfoValue = builder.CreateExtractValue(interfaceStructValue, { 1 }, "typeInfo");
			return VirtualObjectComponents(getTypeInfoComponents(function, typeInfoValue), objectPointer);
		}
		
		VirtualMethodComponents getVirtualMethodComponents(Function& function, const bool isStatic, llvm::Value* const interfaceMethodValue) {
			auto& builder = function.getBuilder();
			auto& module = function.module();
			
			if (isStatic) {
				const auto objectPointer = ConstantGenerator(module).getNull(TypeGenerator(module).getI8PtrType());
				const auto typeInfoValue = builder.CreateExtractValue(interfaceMethodValue, { 0 }, "typeInfo");
				const VirtualObjectComponents virtualObjectComponents(getTypeInfoComponents(function, typeInfoValue), objectPointer);
				
				const auto hashValue = builder.CreateExtractValue(interfaceMethodValue, { 1 }, "methodHash");
				return VirtualMethodComponents(virtualObjectComponents, hashValue);
			} else {
				const auto interfaceStructValue = builder.CreateExtractValue(interfaceMethodValue, { 0 }, "interface");
				const auto hashValue = builder.CreateExtractValue(interfaceMethodValue, { 1 }, "methodHash");
				return VirtualMethodComponents(getVirtualObjectComponents(function, interfaceStructValue), hashValue);
			}
		}
		
	}
	
}
