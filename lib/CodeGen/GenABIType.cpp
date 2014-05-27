#include <string>
#include <stdexcept>
#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/Type.hpp>

#include <locic/CodeGen/GenABIType.hpp>
#include <locic/CodeGen/Mangling.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
	
		llvm_abi::Type templateGeneratorABIType() {
			return llvm_abi::Type::PaddedStruct({ llvm_abi::Type::Pointer(), llvm_abi::Type::Integer(llvm_abi::Int32)});
		}
		
		llvm_abi::Type typeInfoABIType() {
			return llvm_abi::Type::PaddedStruct({ llvm_abi::Type::Pointer(), llvm_abi::Type::Pointer(), llvm_abi::Type::Integer(llvm_abi::Int32)});
		}
		
		size_t roundUpToAlign(size_t position, size_t align) {
			assert(align >= 1);
			const auto roundedUpPosition = position + (align - 1);
			return roundedUpPosition - (roundedUpPosition % align);
		}
		
		llvm_abi::Type makeAlignedStructType(llvm_abi::ABI& abi, std::vector<llvm_abi::Type> memberTypes) {
			std::vector<llvm_abi::StructMember> members;
			size_t offset = 0;
			for (auto& memberType: memberTypes) {
				const auto typeSize = abi.typeSize(memberType);
				const auto typeAlign = abi.typeAlign(memberType);
				
				offset = roundUpToAlign(offset, typeAlign);
				
				members.push_back(llvm_abi::StructMember(std::move(memberType), offset));
				
				offset += typeSize;
			}
			return llvm_abi::Type::Struct(std::move(members));
		}
		
		llvm_abi::Type makePointerPairType(llvm_abi::ABI& abi) {
			std::vector<llvm_abi::Type> memberTypes;
			memberTypes.push_back(llvm_abi::Type::Pointer());
			memberTypes.push_back(llvm_abi::Type::Pointer());
			return makeAlignedStructType(abi, std::move(memberTypes));
		}
		
		llvm_abi::Type getPrimitiveABIType(Module& module, SEM::Type* type) {
			assert(isTypeSizeAlwaysKnown(module, type));
			
			const auto typeInstance = type->getObjectType();
			const auto& name = typeInstance->name().last();
			
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
				return makeAlignedStructType(module.abi(), std::move(members));
			}
			
			// TODO: more types to be handled here.
			llvm_unreachable("Unknown primitive ABI type.");
		}
		
		llvm_abi::Type genABIType(Module& module, SEM::Type* type) {
			if (!isTypeSizeAlwaysKnown(module, type)) {
				// For types with sizes that aren't
				// known in all modules (e.g. classes),
				// pass by pointer.
				return llvm_abi::Type::Pointer();
			}
			
			switch (type->kind()) {
				case SEM::Type::VOID: {
					// TODO: use a void type?
					return llvm_abi::Type::Struct({});
				}
				
				case SEM::Type::OBJECT: {
					const auto typeInstance = type->getObjectType();
					if (typeInstance->isPrimitive()) {
						return getPrimitiveABIType(module, type);
					} else {
						std::vector<llvm_abi::Type> members;
						
						if (typeInstance->isUnionDatatype()) {
							members.push_back(llvm_abi::Type::Integer(llvm_abi::Int8));
						}
						
						for (const auto var: typeInstance->variables()) {
							members.push_back(genABIType(module, var->type()));
						}
						return makeAlignedStructType(module.abi(), std::move(members));
					}
					llvm_unreachable("Unknown object ABI type with known size.");
				}
				
				case SEM::Type::REFERENCE: {
					if (type->getReferenceTarget()->isInterface()) {
						// Interface references are actually two pointers:
						// one to the class, and one to the class vtable.
						return makePointerPairType(module.abi());
					} else {
						return llvm_abi::Type::Pointer();
					}
				}
				
				case SEM::Type::FUNCTION: {
					// Function type is just a pointer.
					return llvm_abi::Type::Pointer();
				}
				
				case SEM::Type::METHOD: {
					/* Method type is:
						struct {
							i8* context,
							RetType (*func)(i8*, ArgTypes)
						};
					*/
					return makePointerPairType(module.abi());
				}
				
				case SEM::Type::INTERFACEMETHOD: {
					/* Interface method type is:
						struct {
							struct {
								i8* context,
								__vtable_type* vtable
							},
							i64 methodHash
						};
					*/
					std::vector<llvm_abi::Type> members;
					members.push_back(makePointerPairType(module.abi()));
					members.push_back(llvm_abi::Type::Integer(llvm_abi::Int64));
					return makeAlignedStructType(module.abi(), std::move(members));
				}
				
				default: {
					llvm_unreachable("Unknown type kind for generating ABI type.");
				}
			}
		}
		
		llvm_abi::FunctionType genABIFunctionType(Module& module, SEM::Type* functionType, llvm::Type* contextPointerType) {
			assert(functionType->isFunction());
			
			llvm_abi::FunctionType abiFunctionType;
			
			if (isTypeSizeAlwaysKnown(module, functionType->getFunctionReturnType())) {
				abiFunctionType.returnType = genABIType(module, functionType->getFunctionReturnType());
			} else {
				// If type is not known in all modules,
				// return via a pointer argument.
				abiFunctionType.returnType = llvm_abi::Type::Struct({});
				abiFunctionType.argTypes.push_back(llvm_abi::Type::Pointer());
			}
			
			if (contextPointerType != nullptr) {
				abiFunctionType.argTypes.push_back(llvm_abi::Type::Pointer());
			}
			
			for (const auto paramType: functionType->getFunctionParameterTypes()) {
				abiFunctionType.argTypes.push_back(genABIType(module, paramType));
			}
			
			return abiFunctionType;
		}
		
	}
	
}

