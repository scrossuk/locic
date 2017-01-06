#ifndef LOCIC_PARSER_VALUEBUILDER_HPP
#define LOCIC_PARSER_VALUEBUILDER_HPP

#include <locic/AST.hpp>

namespace locic {
	
	class Constant;
	class String;
	
	namespace Debug {
		
		class SourcePosition;
		
	}
	
	namespace Parser {
		
		class TokenReader;
		
		class ValueBuilder {
		public:
			ValueBuilder(const TokenReader& reader);
			~ValueBuilder();
			
			AST::Node<AST::ValueDecl>
			makeValueNode(AST::ValueDecl* value,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeTernaryValue(AST::Node<AST::ValueDecl> conditionValue,
			                 AST::Node<AST::ValueDecl> ifTrueValue,
			                 AST::Node<AST::ValueDecl> ifFalseValue,
			                 const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeLogicalOrValue(AST::Node<AST::ValueDecl> leftValue,
			                   AST::Node<AST::ValueDecl> rightValue,
			                   const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeLogicalAndValue(AST::Node<AST::ValueDecl> leftValue,
			                    AST::Node<AST::ValueDecl> rightValue,
			                    const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeBitwiseOrValue(AST::Node<AST::ValueDecl> leftValue,
			                   AST::Node<AST::ValueDecl> rightValue,
			                   const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeBitwiseXorValue(AST::Node<AST::ValueDecl> leftValue,
			                    AST::Node<AST::ValueDecl> rightValue,
			                    const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeBitwiseAndValue(AST::Node<AST::ValueDecl> leftValue,
			                    AST::Node<AST::ValueDecl> rightValue,
			                    const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeCapabilityTest(AST::Node<AST::TypeDecl> leftType,
			                   AST::Node<AST::TypeDecl> rightType,
			                   const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeBinaryOpValue(AST::Node<AST::ValueDecl> leftValue,
			                  AST::Node<AST::ValueDecl> rightValue,
			                  AST::BinaryOpKind opKind,
			                  const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeUnaryOpValue(AST::Node<AST::ValueDecl> operand,
			                 AST::UnaryOpKind opKind,
			                 const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeCallValue(AST::Node<AST::ValueDecl> callableValue,
			              AST::Node<AST::ValueDeclList> parameters,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeIndexValue(AST::Node<AST::ValueDecl> value,
			               AST::Node<AST::ValueDecl> indexValue,
			               const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeDerefValue(AST::Node<AST::ValueDecl> value,
			               const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeMemberAccess(AST::Node<AST::ValueDecl> value, String name,
			                 const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeTemplatedMemberAccess(AST::Node<AST::ValueDecl> value, String name,
			                          AST::Node<AST::ValueDeclList> templateArguments,
			                          const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeMergeValue(AST::Node<AST::ValueDecl> firstValue,
			               AST::Node<AST::ValueDecl> secondValue,
			               const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeBracketedValue(AST::Node<AST::ValueDecl> value,
			                   const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeTypeValue(AST::Node<AST::TypeDecl> type,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeLiteralValue(Constant constant, String literalSpecifier,
			                 const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeSymbolValue(AST::Node<AST::Symbol> symbol,
			                const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeSelfValue(const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeThisValue(const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeAlignOfValue(AST::Node<AST::TypeDecl> operand,
			                 const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeSizeOfValue(AST::Node<AST::TypeDecl> operand,
			                const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeNewValue(AST::Node<AST::ValueDecl> placementArg,
			             AST::Node<AST::ValueDecl> operand,
			             const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeSelfMemberAccess(String name,
			                     const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeInternalConstruct(AST::Node<AST::ValueDeclList> templateArguments,
			                      AST::Node<AST::ValueDeclList> arguments,
			                      const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeArrayLiteralValue(AST::Node<AST::ValueDeclList> values,
			                      const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDeclList>
			makeValueList(AST::ValueDeclList values,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueDecl>
			makeCastValue(AST::ValueDecl::CastKind kind, AST::Node<AST::TypeDecl> fromType,
			              AST::Node<AST::TypeDecl> toType, AST::Node<AST::ValueDecl> value,
			              const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif
