#ifndef LOCIC_PARSER_TYPEBUILDER_HPP
#define LOCIC_PARSER_TYPEBUILDER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	class PrimitiveID;
	class StringHost;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class TypeBuilder {
		public:
			TypeBuilder(const TokenReader& reader);
			~TypeBuilder();
			
			AST::Node<AST::TypeDecl>
			makeTypeNode(AST::TypeDecl* type,
			             const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeDeclList> makeTypeList(AST::TypeDeclList list,
			                                      const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeDecl> makePrimitiveType(PrimitiveID primitiveID,
			                                       const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeDecl> makeSymbolType(AST::Node<AST::Symbol> symbol,
			                                    const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeDecl>
			makeConstPredicateType(AST::Node<AST::PredicateDecl> predicate,
			                       AST::Node<AST::TypeDecl> targetType,
			                       const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeDecl>
			makeConstType(AST::Node<AST::TypeDecl> targetType,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeDecl>
			makeSelfConstType(AST::Node<AST::TypeDecl> targetType,
			                  const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeDecl>
			makeFunctionPointerType(AST::Node<AST::TypeDecl> returnType,
			                        AST::Node<AST::TypeDeclList> paramTypes,bool isVarArg,
			                        const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeDecl> makeAutoType(const Debug::SourcePosition& start);
			AST::Node<AST::TypeDecl> makeReferenceType(AST::Node<AST::TypeDecl> targetType,
			                                       const Debug::SourcePosition& start);
			AST::Node<AST::TypeDecl> makePointerType(AST::Node<AST::TypeDecl> targetType,
			                                     const Debug::SourcePosition& start);
			AST::Node<AST::TypeDecl> makeStaticArrayType(AST::Node<AST::TypeDecl> targetType,
			                                         AST::Node<AST::ValueDecl> sizeValue,
			                                         const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif
