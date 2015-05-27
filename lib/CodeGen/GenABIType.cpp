#include <string>
#include <stdexcept>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Type.hpp>

#include <locic/CodeGen/ArgInfo.hpp>
#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/Liveness.hpp>
#include <locic/CodeGen/LivenessIndicator.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm_abi::Type* genABIArgType(Module& module, const SEM::Type* type) {
			if (canPassByValue(module, type)) {
				return genABIType(module, type);
			} else {
				return llvm_abi::Type::Pointer(module.abiContext());
			}
		}
		
		llvm_abi::Type* genABIType(Module& module, const SEM::Type* type) {
			auto& abiContext = module.abiContext();
			
			switch (type->kind()) {
				case SEM::Type::OBJECT: {
					const auto typeInstance = type->getObjectType();
					
					if (typeInstance->isPrimitive()) {
						return getPrimitiveABIType(module, type);
					} else {
						std::vector<llvm_abi::Type*> members;
						
						if (typeInstance->isUnionDatatype()) {
							members.reserve(2);
							members.push_back(llvm_abi::Type::Integer(abiContext, llvm_abi::Int8));
							
							llvm_abi::Type* maxStructType = llvm_abi::Type::AutoStruct(abiContext, {});
							size_t maxStructSize = 0;
							size_t maxStructAlign = 0;
							
							for (auto variantTypeInstance: typeInstance->variants()) {
								const auto variantStructType = genABIType(module, variantTypeInstance->selfType());
								const auto variantStructSize = module.abi().typeSize(variantStructType);
								const auto variantStructAlign = module.abi().typeAlign(variantStructType);
								
								if (variantStructAlign > maxStructAlign || (variantStructAlign == maxStructAlign && variantStructSize > maxStructSize)) {
									maxStructType = variantStructType;
									maxStructSize = variantStructSize;
									maxStructAlign = variantStructAlign;
								} else {
									assert(variantStructAlign <= maxStructAlign && variantStructSize <= maxStructSize);
								}
							}
							
							members.push_back(maxStructType);
						} else {
							members.reserve(typeInstance->variables().size() + 1);
							
							for (const auto var: typeInstance->variables()) {
								members.push_back(genABIType(module, var->type()));
							}
							
							const auto livenessIndicator = getLivenessIndicator(module, *typeInstance);
							if (livenessIndicator.isSuffixByte()) {
								// Add suffix byte.
								members.push_back(llvm_abi::Type::Integer(abiContext, llvm_abi::Int8));
							}
						}
						
						return llvm_abi::Type::AutoStruct(abiContext, members);
					}
				}
				case SEM::Type::ALIAS: {
					return genABIType(module, type->resolveAliases());
				}
				default: {
					llvm_unreachable("Unknown type kind for generating ABI type.");
				}
			}
		}
		
	}
	
}

