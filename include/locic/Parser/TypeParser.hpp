#ifndef LOCIC_PARSER_TYPEPARSER_HPP
#define LOCIC_PARSER_TYPEPARSER_HPP

#include <locic/AST.hpp>
#include <locic/Parser/Diagnostics.hpp>
#include <locic/Parser/Token.hpp>

namespace locic {
	
	class PrimitiveID;
	class StringHost;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class TypeParser {
		public:
			TypeParser(TokenReader& reader);
			~TypeParser();
			
			void issueError(Diag diag, const Debug::SourcePosition& start);
			
			AST::Node<AST::Type> makePrimitiveType(PrimitiveID primitiveID,
			                                       const Debug::SourcePosition& start,
			                                       bool isSigned = true);
			AST::Node<AST::Type> makeNamedType(const String& name,
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
			
			AST::Node<AST::Type> parseType();
			
			AST::Node<AST::Type> parseQualifiedType();
			
			AST::Node<AST::Type> parseConstType();
			
			AST::Node<AST::Type> parseTypeWithQualifier(const Debug::SourcePosition& start,
			                                            Token::Kind qualifier);
			
			AST::Node<AST::Type> parseBasicType();
			
			AST::Node<AST::Type> parseLongIntOrFloatType(const Debug::SourcePosition& start);
			
			AST::Node<AST::Type> parseIntegerTypeWithSignedness(const Debug::SourcePosition& start,
			                                                    bool isSigned);
			
			AST::Node<AST::Type> parseLongIntegerType(const Debug::SourcePosition& start,
			                                          bool isSigned);
			
		private:
			TokenReader& reader_;
			
		};
		
	}
	
}

#endif
