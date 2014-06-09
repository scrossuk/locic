#include <assert.h>

#include <memory>

#include <llvm-abi/Type.hpp>

namespace llvm_abi {

	struct TypeImpl {
		TypeKind kind;
		union {
			IntegerKind integerKind;
			FloatingPointKind floatingPointKind;
			FloatingPointKind complexKind;
		} subKind;
		
		struct {
			std::vector<StructMember> members;
		} structType;
		
		struct {
			size_t elementCount;
			Type elementType;
		} arrayType;
		
		inline TypeImpl(TypeKind pKind)
			: kind(pKind) { }
	};
	
	namespace {
	
		template<typename T, typename ...Args>
		std::unique_ptr<T> make_unique(Args&& ...args) {
			return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
		}
		
	}
	
	Type Type::Pointer() {
		return Type(make_unique<TypeImpl>(PointerType));
	}
	
	Type Type::Integer(IntegerKind kind) {
		auto typeImpl = make_unique<TypeImpl>(IntegerType);
		typeImpl->subKind.integerKind = kind;
		return Type(std::move(typeImpl));
	}
	
	Type Type::FloatingPoint(FloatingPointKind kind) {
		auto typeImpl = make_unique<TypeImpl>(FloatingPointType);
		typeImpl->subKind.floatingPointKind = kind;
		return Type(std::move(typeImpl));
	}
	
	Type Type::Complex(FloatingPointKind kind) {
		auto typeImpl = make_unique<TypeImpl>(ComplexType);
		typeImpl->subKind.complexKind = kind;
		return Type(std::move(typeImpl));
	}
	
	Type Type::Struct(std::vector<StructMember> members) {
		auto typeImpl = make_unique<TypeImpl>(StructType);
		typeImpl->structType.members = std::move(members);
		return Type(std::move(typeImpl));
	}
	
	Type Type::AutoStruct(std::vector<Type> memberTypes) {
		std::vector<StructMember> members;
		for (auto& memberType: memberTypes) {
			members.push_back(StructMember::AutoOffset(std::move(memberType)));
		}
		return Type::Struct(std::move(members));
	}
	
	Type Type::Array(size_t elementCount, Type elementType) {
		auto typeImpl = make_unique<TypeImpl>(ArrayType);
		typeImpl->arrayType.elementCount = elementCount;
		typeImpl->arrayType.elementType = std::move(elementType);
		return Type(std::move(typeImpl));
	}
	
	Type::Type()
		: impl_() { }
	
	Type::Type(std::unique_ptr<struct TypeImpl>&& impl)
		: impl_(std::move(impl)) { }
	
	Type::Type(Type&& other)
		: impl_(std::move(other.impl_)) { }
	
	Type& Type::operator=(Type other) {
		impl_ = std::move(other.impl_);
		return *this;
	}
		
	Type::~Type() { }
	
	TypeKind Type::kind() const {
		return impl_->kind;
	}
	
	bool Type::isPointer() const {
		return impl_->kind == PointerType;
	}
	
	bool Type::isInteger() const {
		return impl_->kind == IntegerType;
	}
	
	IntegerKind Type::integerKind() const {
		assert(isInteger());
		return impl_->subKind.integerKind;
	}
	
	bool Type::isFloatingPoint() const {
		return impl_->kind == FloatingPointType;
	}
	
	FloatingPointKind Type::floatingPointKind() const {
		assert(isFloatingPoint());
		return impl_->subKind.floatingPointKind;
	}
	
	bool Type::isComplex() const {
		return impl_->kind == ComplexType;
	}
	
	FloatingPointKind Type::complexKind() const {
		assert(isComplex());
		return impl_->subKind.complexKind;
	}
	
	bool Type::isStruct() const {
		return impl_->kind == StructType;
	}
	
	const std::vector<StructMember>& Type::structMembers() const {
		assert(isStruct());
		return impl_->structType.members;
	}
	
	bool Type::isArray() const {
		return impl_->kind == ArrayType;
	}
	
	size_t Type::arrayElementCount() const {
		assert(isArray());
		return impl_->arrayType.elementCount;
	}
	
	const Type& Type::arrayElementType() const {
		assert(isArray());
		return impl_->arrayType.elementType;
	}
	
	static std::string intKindToString(IntegerKind kind) {
		switch (kind) {
			case Bool:
				return "Bool";
			case Char:
				return "Char";
			case Short:
				return "Short";
			case Int:
				return "Int";
			case Long:
				return "Long";
			case LongLong:
				return "LongLong";
			case Int8:
				return "Int8";
			case Int16:
				return "Int16";
			case Int32:
				return "Int32";
			case Int64:
				return "Int64";
			case Int128:
				return "Int128";
			case SizeT:
				return "SizeT";
			default:
				return "[UNKNOWN]";
		}
	}
	
	static std::string floatKindToString(FloatingPointKind kind) {
		switch (kind) {
			case Float:
				return "Float";
			case Double:
				return "Double";
			case LongDouble:
				return "LongDouble";
			case Float128:
				return "Float128";
			default:
				return "[UNKNOWN]";
		}
	}
	
	std::string Type::toString() const {
		switch (kind()) {
			case PointerType:
				return "Pointer";
			case IntegerType:
				return std::string("Integer(") + intKindToString(integerKind()) + ")";
			case FloatingPointType:
				return std::string("FloatingPoint(") + floatKindToString(floatingPointKind()) + ")";
			case ComplexType:
				return std::string("Complex(") + floatKindToString(complexKind()) + ")";
			case StructType: {
				std::string s = "Struct(";
				const auto& members = structMembers();
				for (size_t i = 0; i < members.size(); i++) {
					if (i > 0) {
						s += ", ";
					}
					s += std::string("StructMember(") + members.at(i).type().toString() + ")";
				}
				return s + ")";
			}
			case ArrayType:
				return std::string("Array(") + arrayElementType().toString() + ")";
			default:
				return "[UNKNOWN]";
		}
	}
	
}

