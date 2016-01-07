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
			
			AST::Node<AST::Value>
			makeValueNode(AST::Value* value,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeTernaryValue(AST::Node<AST::Value> conditionValue,
			                 AST::Node<AST::Value> ifTrueValue,
			                 AST::Node<AST::Value> ifFalseValue,
			                 const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeLogicalOrValue(AST::Node<AST::Value> leftValue,
			                   AST::Node<AST::Value> rightValue,
			                   const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeLogicalAndValue(AST::Node<AST::Value> leftValue,
			                    AST::Node<AST::Value> rightValue,
			                    const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeBitwiseOrValue(AST::Node<AST::Value> leftValue,
			                   AST::Node<AST::Value> rightValue,
			                   const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeBitwiseXorValue(AST::Node<AST::Value> leftValue,
			                    AST::Node<AST::Value> rightValue,
			                    const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeBitwiseAndValue(AST::Node<AST::Value> leftValue,
			                    AST::Node<AST::Value> rightValue,
			                    const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeCapabilityTest(AST::Node<AST::Type> leftType,
			                   AST::Node<AST::Type> rightType,
			                   const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeBinaryOpValue(AST::Node<AST::Value> leftValue,
			                  AST::Node<AST::Value> rightValue,
			                  AST::BinaryOpKind opKind,
			                  const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeUnaryOpValue(AST::Node<AST::Value> operand,
			                 AST::UnaryOpKind opKind,
			                 const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeCallValue(AST::Node<AST::Value> callableValue,
			              const AST::Node<AST::ValueList>& parameters,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeIndexValue(AST::Node<AST::Value> value,
			               AST::Node<AST::Value> indexValue,
			               const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeDerefValue(AST::Node<AST::Value> value,
			               const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeMemberAccess(AST::Node<AST::Value> value, String name,
			                 const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeTemplatedMemberAccess(AST::Node<AST::Value> value, String name,
			                          AST::Node<AST::ValueList> templateArguments,
			                          const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeMergeValue(AST::Node<AST::Value> firstValue,
			               AST::Node<AST::Value> secondValue,
			               const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeTypeValue(AST::Node<AST::Type> type,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeLiteralValue(Constant constant, String literalSpecifier,
			                 const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeRefValue(AST::Node<AST::Type> targetType,
			             AST::Node<AST::Value> value,
			             const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeLvalValue(AST::Node<AST::Type> targetType,
			              AST::Node<AST::Value> value,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeNoRefValue(AST::Node<AST::Value> value,
			               const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeNoLvalValue(AST::Node<AST::Value> value,
			                const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeSymbolValue(AST::Node<AST::Symbol> symbol,
			                const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeSelfValue(const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeThisValue(const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeAlignOfValue(AST::Node<AST::Type> operand,
			                 const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeSizeOfValue(AST::Node<AST::Type> operand,
			                const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeSelfMemberAccess(String name,
			                     const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeInternalConstruct(AST::Node<AST::ValueList> templateArguments,
			                      AST::Node<AST::ValueList> arguments,
			                      const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeArrayLiteralValue(AST::Node<AST::ValueList> values,
			                      const Debug::SourcePosition& start);
			
			AST::Node<AST::ValueList>
			makeValueList(AST::ValueList values,
			              const Debug::SourcePosition& start);
			
			AST::Node<AST::Value>
			makeCastValue(AST::Value::CastKind kind, AST::Node<AST::Type> fromType,
			              AST::Node<AST::Type> toType, AST::Node<AST::Value> value,
			              const Debug::SourcePosition& start);
			
		private:
			const TokenReader& reader_;
			
		};
		
	}
	
}

#endif
