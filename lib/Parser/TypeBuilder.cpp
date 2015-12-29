#include <locic/AST.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/Diagnostics.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeBuilder.hpp>
#include <locic/Support/PrimitiveID.hpp>

namespace locic {
	
	namespace Parser {
		
		TypeBuilder::TypeBuilder(const TokenReader& reader)
		: reader_(reader) { }
		
		TypeBuilder::~TypeBuilder() { }
		
		AST::Node<AST::Type>
		TypeBuilder::makeTypeNode(AST::Type* const type,
		                          const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, type);
		}
		
		AST::Node<AST::Type>
		TypeBuilder::makePrimitiveType(const PrimitiveID /*primitiveID*/,
		                               const Debug::SourcePosition& /*start*/,
		                               const bool /*isSigned*/) {
			throw std::logic_error("TODO");
		}
		
		AST::Node<AST::Type>
		TypeBuilder::makeNamedType(const String& /*name*/,
		                           const Debug::SourcePosition& /*start*/) {
			throw std::logic_error("TODO");
		}
		
		AST::Node<AST::Type>
		TypeBuilder::makeLvalType(AST::Node<AST::Type> targetType,
		                          AST::Node<AST::Type> type,
		                          const Debug::SourcePosition& start) {
			return makeTypeNode(AST::Type::Lval(targetType, type), start);
		}
		
		AST::Node<AST::Type>
		TypeBuilder::makeRefType(AST::Node<AST::Type> targetType,
		                         AST::Node<AST::Type> type,
		                         const Debug::SourcePosition& start) {
			return makeTypeNode(AST::Type::Ref(targetType, type), start);
		}
		
		AST::Node<AST::Type>
		TypeBuilder::makeStaticRefType(AST::Node<AST::Type> targetType,
		                               AST::Node<AST::Type> type,
		                               const Debug::SourcePosition& start) {
			return makeTypeNode(AST::Type::StaticRef(targetType, type), start);
		}
		
		AST::Node<AST::Type>
		TypeBuilder::makeAutoType(const Debug::SourcePosition& start) {
			return makeTypeNode(AST::Type::Auto(), start);
		}
		
		AST::Node<AST::Type>
		TypeBuilder::makeReferenceType(AST::Node<AST::Type> targetType,
		                               const Debug::SourcePosition& start) {
			return makeTypeNode(AST::Type::Reference(targetType), start);
		}
		
		AST::Node<AST::Type>
		TypeBuilder::makePointerType(AST::Node<AST::Type> targetType,
		                             const Debug::SourcePosition& start) {
			return makeTypeNode(AST::Type::Pointer(targetType), start);
		}
		
	}
	
}
