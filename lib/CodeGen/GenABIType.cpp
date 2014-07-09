#include <string>
#include <stdexcept>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Type.hpp>

#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/Interface.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/Template.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
		
		llvm_abi::Type* genABIArgType(Module& module, SEM::Type* type) {
			if (isTypeSizeAlwaysKnown(module, type)) {
				return genABIType(module, type);
			} else {
				return llvm_abi::Type::Pointer(module.abiContext());
			}
		}
		
		llvm_abi::Type* genABIType(Module& module, SEM::Type* type) {
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
							members.reserve(typeInstance->variables().size());
							for (const auto var: typeInstance->variables()) {
								members.push_back(genABIType(module, var->type()));
							}
						}
						
						return llvm_abi::Type::AutoStruct(abiContext, members);
					}
				}
				
				case SEM::Type::FUNCTION: {
					// Generate struct of function pointer and template
					// generator if function type is templated.
					if (type->isFunctionTemplated()) {
						std::vector<llvm_abi::Type*> types;
						types.reserve(2);
						types.push_back(llvm_abi::Type::Pointer(abiContext));
						types.push_back(templateGeneratorType(module).first);
						return llvm_abi::Type::AutoStruct(abiContext, types);
					} else {
						return llvm_abi::Type::Pointer(abiContext);
					}
				}
				
				case SEM::Type::METHOD: {
					std::vector<llvm_abi::Type*> types;
					types.reserve(2);
					types.push_back(llvm_abi::Type::Pointer(abiContext));
					types.push_back(genABIType(module, type->getMethodFunctionType()));
					return llvm_abi::Type::AutoStruct(abiContext, types);
				}
				
				case SEM::Type::INTERFACEMETHOD: {
					return interfaceMethodType(module).first;
				}
				
				default: {
					llvm_unreachable("Unknown type kind for generating ABI type.");
				}
			}
		}
		
	}
	
}

