#include <vector>

#include <llvm-abi/ABI.hpp>
#include <llvm-abi/ABI_x86_64.hpp>
#include <llvm-abi/Type.hpp>

namespace llvm_abi {

	namespace {
	
		size_t roundUpToAlign(size_t position, size_t align) {
			assert(align >= 1);
			const auto roundedUpPosition = position + (align - 1);
			return roundedUpPosition - (roundedUpPosition % align);
		}
		
		size_t getTypeAlign(const Type& type);
		
		size_t getTypeSize(const Type& type) {
			if (type.isPointer()) {
				return 8;
			} else if (type.isInteger()) {
				switch (type.integerKind()) {
					case Bool:
					case Char:
					case Int8:
						return 1;
						
					case Short:
					case Int16:
						return 2;
						
					case Int:
					case Int32:
						return 4;
						
					case Long:
					case SizeT:
					case LongLong:
					case Int64:
						return 8;
						
					case Int128:
						return 16;
				}
			} else if (type.isFloatingPoint()) {
				switch (type.floatingPointKind()) {
					case Float:
						return 4;
						
					case Double:
						return 8;
						
					case LongDouble:
						return 16;
						
					case Float128:
						return 16;
				}
			} else if (type.isComplex()) {
				switch (type.complexKind()) {
					case Float:
						return 8;
						
					case Double:
						return 16;
						
					case LongDouble:
						return 32;
						
					case Float128:
						return 32;
				}
			} else if (type.isStruct()) {
				size_t size = 0;
				
				for (const auto& member: type.structMembers()) {
					// Add necessary padding before this member.
					size = roundUpToAlign(size, getTypeAlign(member.type));
					
					// Add the member's size.
					size += getTypeSize(member.type);
				}
				
				// Add any final padding.
				return roundUpToAlign(size, getTypeAlign(type));
			}
			llvm_unreachable("Unknown ABI type.");
		}
		
		size_t getTypeAlign(const Type& type) {
			if (type.isStruct()) {
				size_t mostStrictAlign = 1;
				for (const auto& member: type.structMembers()) {
					const size_t align = getTypeAlign(member.type);
					mostStrictAlign = std::max<size_t>(mostStrictAlign, align);
				}
				
				return mostStrictAlign;
			} else if (type.isArray()) {
				const auto elementAlign = getTypeAlign(type.arrayElementType());
				const size_t minAlign = getTypeSize(type) >= 16 ? 16 : 1;
				return std::max<size_t>(elementAlign, minAlign);
			} else {
				return getTypeSize(type);
			}
		}
		
		bool hasUnalignedFields(const Type& type) {
			if (!type.isStruct()) return false;
			
			size_t offset = 0;
			
			for (const auto& member: type.structMembers()) {
				// Add necessary padding before this member.
				offset = roundUpToAlign(offset, getTypeAlign(member.type));
				
				if (member.offset != offset || hasUnalignedFields(member.type)) {
					return true;
				}
				
				// Add the member's size.
				offset += getTypeSize(member.type);
			}
			
			return false;
		}
		
		enum ArgClass {
			Integer,
			Sse,
			SseUp,
			X87,
			X87Up,
			ComplexX87,
			NoClass,
			Memory
		};
		
		// Class merge operation as specified in ABI.
		ArgClass merge(ArgClass first, ArgClass second) {
			if (first == second) {
				return first;
			}
			
			if (first == NoClass) {
				return second;
			}
			
			if (second == NoClass) {
				return first;
			}
			
			if (first == Memory || second == Memory) {
				return Memory;
			}
			
			if (first == Integer || second == Integer) {
				return Integer;
			}
			
			if (first == X87 || first == X87Up || first == ComplexX87 ||
				second == X87 || second == X87Up || second == ComplexX87) {
				return Memory;
			}
			
			return Sse;
		}
		
		class Classification {
			public:
				ArgClass classes[2];
				
				Classification() {
					classes[0] = NoClass;
					classes[1] = NoClass;
				}
				
				bool isMemory() const {
					return classes[0] == Memory;
				}
				
				void addField(size_t offset, ArgClass fieldClass) {
					if (isMemory()) {
						return;
					}
					
					// Note that we don't need to bother checking if it crosses 8 bytes.
					// We don't get here with unaligned fields, and anything that can be
					// big enough to cross 8 bytes (cdoubles, reals, structs and arrays)
					// is special-cased in classifyType()
					const size_t idx = (offset < 8 ? 0 : 1);
					
					const auto mergedClass = merge(classes[idx], fieldClass);
					
					if (mergedClass != classes[idx]) {
						classes[idx] = mergedClass;
						
						if (mergedClass == Memory) {
							classes[1 - idx] = Memory;
						}
					}
				}
				
		};
		
		void classifyType(Classification& classification, const Type& type, size_t offset) {
			if (type.isInteger() || type.isPointer()) {
				classification.addField(offset, Integer);
			} else if (type.isFloatingPoint()) {
				if (type.floatingPointKind() == LongDouble) {
					classification.addField(offset, X87);
					classification.addField(offset + 8, X87Up);
				} else {
					classification.addField(offset, Sse);
				}
			} else if (type.isComplex()) {
				if (type.complexKind() == Float) {
					classification.addField(offset, Sse);
					classification.addField(offset + 4, Sse);
				} else if (type.complexKind() == Double) {
					classification.addField(offset, Sse);
					classification.addField(offset + 8, Sse);
				} else if (type.complexKind() == LongDouble) {
					classification.addField(offset, ComplexX87);
					// make sure other half knows about it too:
					classification.addField(offset + 16, ComplexX87);
				}
			} else if (type.isArray()) {
				const auto& elementType = type.arrayElementType();
				const auto elementSize = getTypeSize(elementType);
				
				for (size_t i = 0; i < type.arrayElementCount(); i++) {
					classifyType(classification, elementType, offset + i * elementSize);
				}
			} else if (type.isStruct()) {
				const auto& structMembers = type.structMembers();
				
				for (const auto& member: structMembers) {
					classifyType(classification, member.type, offset + member.offset);
				}
			} else {
				llvm_unreachable("Unknown type kind.");
			}
		}
		
		Classification classify(const Type& type) {
			Classification classification;
			
			if (getTypeSize(type) > 32 || hasUnalignedFields(type)) {
				// If size exceeds "four eightbytes" or type
				// has "unaligned fields", pass in memory.
				classification.addField(0, Memory);
				return classification;
			}
			
			classifyType(classification, type, 0);
			return classification;
		}
		
		/// Returns the type to pass as, or null if no transformation is needed.
		llvm::Type* getAbiType(llvm::LLVMContext& context, const Type& type) {
			if (!((type.isComplex() && type.complexKind() == Float) || type.isStruct())) {
				return nullptr; // Nothing to do.
			}
			
			const auto classification = classify(type);
			if (classification.isMemory()) {
				// LLVM presumably handles passing values in memory correctly.
				return nullptr;
			}
			
			assert(!classification.isMemory());
			
			if (classification.classes[0] == NoClass) {
				assert(classification.classes[1] == NoClass && "Non-empty struct with empty first half?");
				return nullptr; // Empty structs should also be handled correctly by LLVM.
			}
			
			// Okay, we may need to transform. Figure out a canonical type:
			
			std::vector<llvm::Type*> parts;
			
			const auto size = getTypeSize(type);
			
			switch (classification.classes[0]) {
				case Integer: {
					const auto bits = (size >= 8 ? 64 : (size * 8));
					parts.push_back(llvm::IntegerType::get(context, bits));
					break;
				}
				
				case Sse:
					parts.push_back(size <= 4 ? llvm::Type::getFloatTy(context) : llvm::Type::getDoubleTy(context));
					break;
					
				case X87:
					assert(classification.classes[1] == X87Up && "Upper half of real not X87Up?");
					/// The type only contains a single real/ireal field,
					/// so just use that type.
					return llvm::Type::getX86_FP80Ty(context);
					
				default:
					llvm_unreachable("Unanticipated argument class.");
			}
			
			switch (classification.classes[1]) {
				case NoClass:
					assert(parts.size() == 1);
					// No need to use a single-element struct type.
					// Just use the element type instead.
					return parts.at(0);
					
				case Integer: {
					assert(size > 8);
					const auto bits = (size - 8) * 8;
					parts.push_back(llvm::IntegerType::get(context, bits));
					break;
				}
				
				case Sse:
					parts.push_back(size <= 12 ? llvm::Type::getFloatTy(context) : llvm::Type::getDoubleTy(context));
					break;
					
				case X87Up:
					if (classification.classes[0] == X87) {
						// This won't happen: it was short-circuited while
						// processing the first half.
					} else {
						// I can't find this anywhere in the ABI documentation,
						// but this is what gcc does (both regular and llvm-gcc).
						// (This triggers for types like union { real r; byte b; })
						parts.push_back(llvm::Type::getDoubleTy(context));
					}
					
					break;
					
				default:
					llvm_unreachable("Unanticipated argument class for second half.");
			}
			
			return llvm::StructType::get(context, parts);
		}
		
	}
	
	ABI_x86_64::ABI_x86_64(llvm::LLVMContext& llvmContext)
		: llvmContext_(llvmContext) { }
	
	ABI_x86_64::~ABI_x86_64() { }
	
	std::string ABI_x86_64::name() const {
		return "x86_64";
	}
	
	size_t ABI_x86_64::typeSize(const Type& type) {
		return getTypeSize(type);
	}
	
	size_t ABI_x86_64::typeAlign(const Type& type) {
		return getTypeAlign(type);
	}
	
	std::vector<llvm::Value*> ABI_x86_64::encodeValues(llvm::IRBuilder<>& builder, const std::vector<llvm::Value*>& argValues, const std::vector<Type>& argTypes) {
		assert(argValues.size() == argTypes.size());
		
		std::vector<llvm::Value*> encodedValues;
		
		for (size_t i = 0; i < argValues.size(); i++) {
			const auto argValue = argValues.at(i);
			const auto& argType = argTypes.at(i);
			const auto llvmAbiType = getAbiType(llvmContext_, argType);
			if (llvmAbiType == nullptr) {
				encodedValues.push_back(argValue);
				continue;
			}
			
			const auto argValuePtr = builder.CreateAlloca(argValue->getType());
			builder.CreateStore(argValue, argValuePtr);
			const auto encodedValuePtr = builder.CreateAlloca(llvmAbiType);
			builder.CreateMemCpy(encodedValuePtr, argValuePtr, getTypeSize(argType), getTypeAlign(argType));
			
			encodedValues.push_back(builder.CreateLoad(encodedValuePtr));
		}
		
		return encodedValues;
	}
	
	std::vector<llvm::Value*> ABI_x86_64::decodeValues(llvm::IRBuilder<>& builder, const std::vector<llvm::Value*>& argValues, const std::vector<Type>& argTypes, const std::vector<llvm::Type*>& llvmArgTypes) {
		assert(argValues.size() == argTypes.size());
		
		std::vector<llvm::Value*> decodedValues;
		
		for (size_t i = 0; i < argValues.size(); i++) {
			const auto encodedValue = argValues.at(i);
			const auto& argType = argTypes.at(i);
			const auto llvmAbiType = getAbiType(llvmContext_, argType);
			if (llvmAbiType == nullptr) {
				decodedValues.push_back(encodedValue);
				continue;
			}
			
			const auto encodedValuePtr = builder.CreateAlloca(encodedValue->getType());
			builder.CreateStore(encodedValue, encodedValuePtr);
			const auto argValuePtr = builder.CreateAlloca(llvmArgTypes.at(i));
			builder.CreateMemCpy(argValuePtr, encodedValuePtr, getTypeSize(argType), getTypeAlign(argType));
			
			decodedValues.push_back(builder.CreateLoad(argValuePtr));
		}
		
		return decodedValues;
	}
	
	namespace {
	
		// Helper function for rewriteFunctionType.
		llvm::Type* fixType(llvm::LLVMContext& llvmContext, llvm::Type* llvmType, const Type& type) {
			const auto llvmAbiType = getAbiType(llvmContext, type);
			return llvmAbiType != nullptr ? llvmAbiType : llvmType;
		}
		
	}
	
	llvm::FunctionType* ABI_x86_64::rewriteFunctionType(llvm::FunctionType* llvmFunctionType, const FunctionType& functionType) {
		const auto returnType = fixType(llvmContext_, llvmFunctionType->getReturnType(), functionType.returnType);
		
		assert(llvmFunctionType->getNumParams() == functionType.argTypes.size());
		
		std::vector<llvm::Type*> argTypes;
		
		for (size_t i = 0; i < llvmFunctionType->getNumParams(); i++) {
			argTypes.push_back(fixType(llvmContext_, llvmFunctionType->getParamType(i), functionType.argTypes.at(i)));
		}
		
		return llvm::FunctionType::get(returnType, argTypes, llvmFunctionType->isVarArg());
	}
	
}

