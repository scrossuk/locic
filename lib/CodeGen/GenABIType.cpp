#include <string>
#include <stdexcept>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABITypeInfo.hpp>
#include <llvm-abi/Type.hpp>
#include <llvm-abi/TypeBuilder.hpp>

#include <locic/AST/Type.hpp>
#include <locic/AST/TypeInstance.hpp>
#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Var.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/LivenessIndicator.hpp>
#include <locic/CodeGen/LivenessInfo.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeInfo.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm_abi::Type genABIArgType(Module& module, const AST::Type* type) {
			if (!TypeInfo(module).isPassedByValue(type)) {
				return module.abiTypeBuilder().getPointerTy();
			}
			
			return genABIType(module, type);
		}
		
		llvm_abi::Type genABIObjectType(Module& module, const AST::TypeInstance& typeInstance,
		                                const AST::ValueArray& templateArguments) {
			auto& abiTypeBuilder = module.abiTypeBuilder();
			
			if (typeInstance.isPrimitive()) {
				return getPrimitiveABIType(module, AST::Type::Object(&typeInstance, templateArguments.copy()));
			} else {
				if (typeInstance.isEnum()) {
					// Enums have underlying type 'int'.
					return llvm_abi::IntTy;
				}
				
				const auto mangledName = mangleObjectType(module, &typeInstance).asStdString();
				
				llvm::SmallVector<llvm_abi::Type, 8> members;
				
				if (typeInstance.isUnion()) {
					members.reserve(typeInstance.variables().size() + 1);
					
					for (const auto var: typeInstance.variables()) {
						members.push_back(genABIType(module, var->type()));
					}
					
					return abiTypeBuilder.getUnionTy(members, mangledName);
				} else if (typeInstance.isVariant()) {
					members.reserve(typeInstance.variantTypes().size());
					
					for (const auto variantType: typeInstance.variantTypes()) {
						members.push_back(genABIType(module, variantType));
					}
					
					const auto unionType = abiTypeBuilder.getUnionTy(members);
					return abiTypeBuilder.getStructTy({llvm_abi::Int8Ty,
					                                   unionType}, mangledName);
				} else {
					members.reserve(typeInstance.variables().size() + 1);
					
					for (const auto var: typeInstance.variables()) {
						members.push_back(genABIType(module, var->type()));
					}
					
					const auto livenessIndicator =
					    LivenessInfo(module).getLivenessIndicator(typeInstance);
					if (livenessIndicator.isSuffixByte()) {
						// Add suffix byte.
						members.push_back(llvm_abi::Int8Ty);
					}
					
					// Class sizes must be at least one byte; empty structs
					// are zero size for compatibility with the GCC extension.
					if (!typeInstance.isStruct() && members.empty()) {
						members.push_back(llvm_abi::Int8Ty);
					}
					
					return abiTypeBuilder.getStructTy(members, mangledName);
				}
			}
		}
		
		llvm_abi::Type genABIType(Module& module, const AST::Type* type) {
			switch (type->kind()) {
				case AST::Type::OBJECT: {
					const auto typeInstance = type->getObjectType();
					
					if (typeInstance->isPrimitive()) {
						return getPrimitiveABIType(module, type);
					}
					
					return genABIObjectType(module, *typeInstance,
					                        type->templateArguments());
				}
				case AST::Type::ALIAS: {
					return genABIType(module, type->resolveAliases());
				}
				default: {
					llvm_unreachable("Unknown type kind for generating ABI type.");
				}
			}
		}
		
	}
	
}

