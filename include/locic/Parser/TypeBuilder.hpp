#ifndef LOCIC_PARSER_TYPEBUILDER_HPP
#define LOCIC_PARSER_TYPEBUILDER_HPP

#include <locic/AST.hpp>
#include <locic/Parser/Diagnostics.hpp>

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
			
			AST::Node<AST::Type> makeTypeNode(AST::Type* type,
			                                  const Debug::SourcePosition& start);
			
			AST::Node<AST::TypeList> makeTypeList(AST::TypeList list,
			                                      const Debug::SourcePosition& start);
			
			AST::Node<AST::Type> makePrimitiveType(PrimitiveID primitiveID,
			                                       const Debug::SourcePosition& start);
			
			AST::Node<AST::Type> makeSymbolType(AST::Node<AST::Symbol> symbol,
			                                    const Debug::SourcePosition& start);
			
			AST::Node<AST::Type>
			makeConstPredicateType(AST::Node<AST::Predicate> predicate,
			                       AST::Node<AST::Type> targetType,
			                       const Debug::SourcePosition& start);
			
			AST::Node<AST::Type> makeConstType(AST::Node<AST::Type> targetType,
			                                   const Debug::SourcePosition& start);
			
			AST::Node<AST::Type> makeNoTagType(AST::Node<AST::Type> targetType,
			                                   const Debug::SourcePosition& start);
			
			AST::Node<AST::Type> makeLvalType(AST::Node<AST::Type> targetType,
			                                  AST::Node<AST::Type> type,
			                                  const Debug::SourcePosition& start);
			AST::Node<AST::Type> makeRefType(AST::Node<AST::Type> targetType,
			                                 AST::Node<AST::Type> type,
			                                 const Debug::SourcePosition& start);
			AST::Node<AST::Type> makeStaticRefType(AST::Node<AST::Type> targetType,
			                                       AST::Node<AST::Type> type,
			                                       const Debug::SourcePosition& start);
			
			AST::Node<AST::Type> makeAutoType(const Debug::SourcePosition& start);
			AST::Node<AST::Type> makeReferenceType(AST::Node<AST::Type> targetType,
			                                       const Debug::SourcePosition& start);
			AST::Node<AST::Type> makePointerType(AST::Node<AST::Type> targetType,
			                                     const Debug::SourcePosition& start);
			AST::Node<AST::Type> makeStaticArrayType(AST::Node<AST::Type> targetType,
			                                         AST::Node<AST::Value> sizeValue,
			                                         const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif
