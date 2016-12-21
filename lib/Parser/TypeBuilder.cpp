#include <locic/AST.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/TypeBuilder.hpp>
#include <locic/Support/PrimitiveID.hpp>

namespace locic {
	
	namespace Parser {
		
		TypeBuilder::TypeBuilder(const TokenReader& reader)
		: reader_(reader) { }
		
		TypeBuilder::~TypeBuilder() { }
		
		AST::Node<AST::TypeDecl>
		TypeBuilder::makeTypeNode(AST::TypeDecl* const type,
		                          const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, std::move(type));
		}
		
		AST::Node<AST::TypeDeclList>
		TypeBuilder::makeTypeList(AST::TypeDeclList list,
		                          const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::TypeDeclList(std::move(list)));
		}
		
		AST::Node<AST::TypeDecl>
		TypeBuilder::makePrimitiveType(const PrimitiveID primitiveID,
		                               const Debug::SourcePosition& start) {
			return makeTypeNode(AST::TypeDecl::Primitive(primitiveID), start);
		}
		
		AST::Node<AST::TypeDecl>
		TypeBuilder::makeSymbolType(AST::Node<AST::Symbol> symbol,
		                            const Debug::SourcePosition& start) {
			return makeTypeNode(AST::TypeDecl::Object(std::move(symbol)), start);
		}
		
		AST::Node<AST::TypeDecl>
		TypeBuilder::makeConstPredicateType(AST::Node<AST::PredicateDecl> predicate,
		                                    AST::Node<AST::TypeDecl> targetType,
		                                    const Debug::SourcePosition& start) {
			return makeTypeNode(AST::TypeDecl::ConstPredicate(std::move(predicate), std::move(targetType)),
			                    start);
		}
		
		AST::Node<AST::TypeDecl>
		TypeBuilder::makeConstType(AST::Node<AST::TypeDecl> targetType,
		                           const Debug::SourcePosition& start) {
			return makeTypeNode(AST::TypeDecl::Const(std::move(targetType)), start);
		}
		
		AST::Node<AST::TypeDecl>
		TypeBuilder::makeNoTagType(AST::Node<AST::TypeDecl> targetType,
		                           const Debug::SourcePosition& start) {
			return makeTypeNode(AST::TypeDecl::NoTag(std::move(targetType)), start);
		}
		
		AST::Node<AST::TypeDecl>
		TypeBuilder::makeLvalType(AST::Node<AST::TypeDecl> type,
		                          const Debug::SourcePosition& start) {
			return makeTypeNode(AST::TypeDecl::Lval(std::move(type)), start);
		}
		
		AST::Node<AST::TypeDecl>
		TypeBuilder::makeRefType(AST::Node<AST::TypeDecl> targetType,
		                         AST::Node<AST::TypeDecl> type,
		                         const Debug::SourcePosition& start) {
			return makeTypeNode(AST::TypeDecl::Ref(std::move(targetType), std::move(type)), start);
		}
		
		AST::Node<AST::TypeDecl>
		TypeBuilder::makeStaticRefType(AST::Node<AST::TypeDecl> targetType,
		                               AST::Node<AST::TypeDecl> type,
		                               const Debug::SourcePosition& start) {
			return makeTypeNode(AST::TypeDecl::StaticRef(std::move(targetType), std::move(type)), start);
		}
		
		AST::Node<AST::TypeDecl>
		TypeBuilder::makeFunctionPointerType(AST::Node<AST::TypeDecl> returnType,
		                                     AST::Node<AST::TypeDeclList> paramTypes,
		                                     const bool isVarArg,
		                                     const Debug::SourcePosition& start) {
			if (isVarArg) {
				return makeTypeNode(AST::TypeDecl::VarArgFunction(std::move(returnType), std::move(paramTypes)), start);
			} else {
				return makeTypeNode(AST::TypeDecl::Function(std::move(returnType), std::move(paramTypes)), start);
			}
		}
		
		AST::Node<AST::TypeDecl>
		TypeBuilder::makeAutoType(const Debug::SourcePosition& start) {
			return makeTypeNode(AST::TypeDecl::Auto(), start);
		}
		
		AST::Node<AST::TypeDecl>
		TypeBuilder::makeReferenceType(AST::Node<AST::TypeDecl> targetType,
		                               const Debug::SourcePosition& start) {
			return makeTypeNode(AST::TypeDecl::Reference(std::move(targetType)), start);
		}
		
		AST::Node<AST::TypeDecl>
		TypeBuilder::makePointerType(AST::Node<AST::TypeDecl> targetType,
		                             const Debug::SourcePosition& start) {
			return makeTypeNode(AST::TypeDecl::Pointer(std::move(targetType)), start);
		}
		
		AST::Node<AST::TypeDecl>
		TypeBuilder::makeStaticArrayType(AST::Node<AST::TypeDecl> targetType,
		                                 AST::Node<AST::ValueDecl> sizeValue,
		                                 const Debug::SourcePosition& start) {
			return makeTypeNode(AST::TypeDecl::StaticArray(std::move(targetType),
			                                           std::move(sizeValue)), start);
		}
		
	}
	
}
