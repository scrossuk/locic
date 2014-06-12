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
		
		llvm_abi::Type genABIArgType(Module& module, SEM::Type* type) {
			if (isTypeSizeAlwaysKnown(module, type)) {
				return genABIType(module, type);
			} else {
				return llvm_abi::Type::Pointer();
			}
		}
		
		llvm_abi::Type getPrimitiveABIType(Module& module, SEM::Type* type) {
			assert(isTypeSizeKnownInThisModule(module, type));
			
			const auto typeInstance = type->getObjectType();
			const auto& name = typeInstance->name().last();
			
			if (name == "void_t") {
				// TODO: use a void type?
				return llvm_abi::Type::Struct({});
			}
			
			if (name == "null_t") {
				return llvm_abi::Type::Pointer();
			}
			
			if (name == "ptr" || name == "ptr_lval") {
				return llvm_abi::Type::Pointer();
			}
			
			if (name == "bool") {
				return llvm_abi::Type::Integer(llvm_abi::Bool);
			} else if (name == "char_t" || name == "uchar_t") {
				return llvm_abi::Type::Integer(llvm_abi::Char);
			} else if (name == "short_t" || name == "ushort_t") {
				return llvm_abi::Type::Integer(llvm_abi::Short);
			} else if (name == "int_t" || name == "uint_t") {
				return llvm_abi::Type::Integer(llvm_abi::Int);
			} else if (name == "long_t" || name == "ulong_t") {
				return llvm_abi::Type::Integer(llvm_abi::Long);
			} else if (name == "longlong_t" || name == "ulonglong_t") {
				return llvm_abi::Type::Integer(llvm_abi::LongLong);
			}
			
			if (name == "int8_t" || name == "uint8_t") {
				return llvm_abi::Type::Integer(llvm_abi::Int8);
			} else if (name == "int16_t" || name == "uint16_t") {
				return llvm_abi::Type::Integer(llvm_abi::Int16);
			} else if (name == "int32_t" || name == "uint32_t") {
				return llvm_abi::Type::Integer(llvm_abi::Int32);
			} else if (name == "int64_t" || name == "uint64_t") {
				return llvm_abi::Type::Integer(llvm_abi::Int64);
			}
			
			if (name == "size_t" || name == "ssize_t") {
				return llvm_abi::Type::Integer(llvm_abi::SizeT);
			}
			
			if (name == "float_t") {
				return llvm_abi::Type::FloatingPoint(llvm_abi::Float);
			} else if (name == "double_t") {
				return llvm_abi::Type::FloatingPoint(llvm_abi::Double);
			} else if (name == "longdouble_t") {
				return llvm_abi::Type::FloatingPoint(llvm_abi::LongDouble);
			}
			
			if (name == "member_lval") {
				return genABIType(module, type->templateArguments().at(0));
			} else if (name == "value_lval") {
				std::vector<llvm_abi::Type> members;
				members.push_back(genABIType(module, type->templateArguments().at(0)));
				members.push_back(llvm_abi::Type::Integer(llvm_abi::Bool));
				return llvm_abi::Type::AutoStruct(std::move(members));
			}
			
			if (name == "__ref") {
				if (type->templateArguments().at(0)->isInterface()) {
					return std::move(interfaceStructType(module).first);
				} else {
					return llvm_abi::Type::Pointer();
				}
			}
			
			if (name == "typename_t") {
				return std::move(typeInfoType(module).first);
			}
			
			// TODO: more types to be handled here.
			llvm_unreachable("Unknown primitive ABI type.");
		}
		
		llvm_abi::Type genABIType(Module& module, SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::OBJECT: {
					const auto typeInstance = type->getObjectType();
					
					if (typeInstance->isPrimitive()) {
						return getPrimitiveABIType(module, type);
					} else {
						std::vector<llvm_abi::Type> members;
						
						if (typeInstance->isUnionDatatype()) {
							members.push_back(llvm_abi::Type::Integer(llvm_abi::Int8));
							
							llvm_abi::Type maxStructType = llvm_abi::Type::AutoStruct({});
							size_t maxStructSize = 0;
							size_t maxStructAlign = 0;
							
							for (auto variantTypeInstance: typeInstance->variants()) {
								auto variantStructType = genABIType(module, variantTypeInstance->selfType());
								const auto variantStructSize = module.abi().typeSize(variantStructType);
								const auto variantStructAlign = module.abi().typeAlign(variantStructType);
								
								if (variantStructAlign > maxStructAlign || (variantStructAlign == maxStructAlign && variantStructSize > maxStructSize)) {
									maxStructType = std::move(variantStructType);
									maxStructSize = variantStructSize;
									maxStructAlign = variantStructAlign;
								} else {
									assert(variantStructAlign <= maxStructAlign && variantStructSize <= maxStructSize);
								}
							}
							
							members.push_back(std::move(maxStructType));
						} else {
							for (const auto var: typeInstance->variables()) {
								members.push_back(genABIType(module, var->type()));
							}
						}
						
						return llvm_abi::Type::AutoStruct(std::move(members));
					}
				}
				
				case SEM::Type::FUNCTION: {
					// Generate struct of function pointer and template
					// generator if function type is templated method.
					if (type->isFunctionTemplatedMethod()) {
						std::vector<llvm_abi::Type> types;
						types.push_back(llvm_abi::Type::Pointer());
						types.push_back(std::move(templateGeneratorType(module).first));
						return llvm_abi::Type::AutoStruct(std::move(types));
					} else {
						return llvm_abi::Type::Pointer();
					}
				}
				
				case SEM::Type::METHOD: {
					std::vector<llvm_abi::Type> types;
					types.push_back(llvm_abi::Type::Pointer());
					types.push_back(genABIType(module, type->getMethodFunctionType()));
					return llvm_abi::Type::AutoStruct(std::move(types));
				}
				
				case SEM::Type::INTERFACEMETHOD: {
					return std::move(interfaceMethodType(module).first);
				}
				
				default: {
					llvm_unreachable("Unknown type kind for generating ABI type.");
				}
			}
		}
		
	}
	
}

