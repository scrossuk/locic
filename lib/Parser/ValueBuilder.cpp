#include <locic/AST.hpp>
#include <locic/Debug/SourceLocation.hpp>
#include <locic/Debug/SourceRange.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/ValueBuilder.hpp>

namespace locic {
	
	namespace Parser {
		
// 		static AST::Value* UnaryOp(const String& name, AST::Node<AST::Value> operand) {
// 			const auto paramNode = AST::Node<AST::ValueList>(operand.location(), new AST::ValueList());
// 			return AST::Value::FunctionCall(AST::makeNode(operand.location(), AST::Value::MemberAccess(operand, name)), paramNode);
// 		}
// 		
// 		static AST::Value* BinaryOp(const String& name, AST::Node<AST::Value> leftOperand, AST::Node<AST::Value> rightOperand) {
// 			const auto paramNode = AST::makeNode(rightOperand.location(), new AST::ValueList(1, rightOperand));
// 			return AST::Value::FunctionCall(AST::makeNode(leftOperand.location(), AST::Value::MemberAccess(leftOperand, name)), paramNode);
// 		}
		
		ValueBuilder::ValueBuilder(const TokenReader& reader)
		: reader_(reader) { }
		
		ValueBuilder::~ValueBuilder() { }
		
		AST::Node<AST::Value>
		ValueBuilder::makeValueNode(AST::Value* const value,
		                            const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, value);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeTernaryValue(AST::Node<AST::Value> conditionValue,
		                               AST::Node<AST::Value> ifTrueValue,
		                               AST::Node<AST::Value> ifFalseValue,
		                               const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::Ternary(conditionValue, ifTrueValue,
			                                         ifFalseValue), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeLogicalOrValue(AST::Node<AST::Value> leftValue,
		                                 AST::Node<AST::Value> rightValue,
		                                 const Debug::SourcePosition& start) {
			return makeBinaryOpValue(leftValue, rightValue,
			                         AST::OP_LOGICALOR, start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeLogicalAndValue(AST::Node<AST::Value> leftValue,
		                                  AST::Node<AST::Value> rightValue,
		                                  const Debug::SourcePosition& start) {
			return makeBinaryOpValue(leftValue, rightValue,
			                         AST::OP_LOGICALAND, start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeBitwiseOrValue(AST::Node<AST::Value> leftValue,
		                                 AST::Node<AST::Value> rightValue,
		                                 const Debug::SourcePosition& start) {
			return makeBinaryOpValue(leftValue, rightValue,
			                         AST::OP_BITWISEOR, start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeBitwiseXorValue(AST::Node<AST::Value> /*leftValue*/,
		                                  AST::Node<AST::Value> /*rightValue*/,
		                                  const Debug::SourcePosition& /*start*/) {
			throw std::logic_error("TODO");
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeBitwiseAndValue(AST::Node<AST::Value> leftValue,
		                                  AST::Node<AST::Value> rightValue,
		                                  const Debug::SourcePosition& start) {
			return makeBinaryOpValue(leftValue, rightValue,
			                         AST::OP_BITWISEAND, start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeCapabilityTest(AST::Node<AST::Type> leftType,
		                                 AST::Node<AST::Type> rightType,
		                                 const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::CapabilityTest(leftType, rightType), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeBinaryOpValue(AST::Node<AST::Value> leftValue,
		                                AST::Node<AST::Value> rightValue,
		                                const AST::BinaryOpKind opKind,
		                                const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::BinaryOp(opKind, leftValue,
			                                          rightValue), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeUnaryOpValue(AST::Node<AST::Value> operand,
		                               const AST::UnaryOpKind opKind,
		                               const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::UnaryOp(opKind, operand), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeCallValue(AST::Node<AST::Value> callableValue,
		                            const AST::Node<AST::ValueList>& parameters,
		                            const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::FunctionCall(callableValue, parameters), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeIndexValue(AST::Node<AST::Value> value,
		                             AST::Node<AST::Value> indexValue,
		                             const Debug::SourcePosition& start) {
			return makeBinaryOpValue(value, indexValue, AST::OP_INDEX, start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeDerefValue(AST::Node<AST::Value> value,
		                             const Debug::SourcePosition& start) {
			return makeUnaryOpValue(value, AST::OP_DEREF, start);
		}
			
		AST::Node<AST::Value>
		ValueBuilder::makeMemberAccess(AST::Node<AST::Value> value, String name,
		                               const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::MemberAccess(value, name), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeTemplatedMemberAccess(AST::Node<AST::Value> value, String name,
		                                        AST::Node<AST::ValueList> templateArguments,
		                                        const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::TemplatedMemberAccess(value, name,
			                                                       templateArguments), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeMergeValue(AST::Node<AST::Value> firstValue,
		                             AST::Node<AST::Value> secondValue,
		                             const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::Merge(firstValue, secondValue), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeBracketedValue(AST::Node<AST::Value> value,
		                                 const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::Bracket(value), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeTypeValue(AST::Node<AST::Type> type,
		                            const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::TypeRef(type), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeLiteralValue(const Constant constant,
		                               const String literalSpecifier,
		                               const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			const auto constantNode = AST::makeNode(location, new Constant(constant));
			return makeValueNode(AST::Value::Literal(literalSpecifier,
			                                         constantNode), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeRefValue(AST::Node<AST::Type> targetType,
		                           AST::Node<AST::Value> value,
		                           const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::Ref(targetType, value), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeLvalValue(AST::Node<AST::Type> targetType,
		                            AST::Node<AST::Value> value,
		                            const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::Lval(targetType, value), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeNoRefValue(AST::Node<AST::Value> value,
		                             const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::NoRef(value), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeNoLvalValue(AST::Node<AST::Value> value,
		                              const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::NoLval(value), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeSymbolValue(AST::Node<AST::Symbol> symbol,
		                              const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::SymbolRef(symbol), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeSelfValue(const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::Self(), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeThisValue(const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::This(), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeAlignOfValue(const AST::Node<AST::Type> operand,
		                               const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::AlignOf(operand), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeSizeOfValue(AST::Node<AST::Type> operand,
		                              const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::SizeOf(operand), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeSelfMemberAccess(const String name,
		                                   const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::MemberRef(name), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeInternalConstruct(AST::Node<AST::ValueList> templateArguments,
		                                    AST::Node<AST::ValueList> arguments,
		                                    const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::InternalConstruct(templateArguments,
			                                                   arguments), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeArrayLiteralValue(AST::Node<AST::ValueList> values,
		                                    const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::ArrayLiteral(values), start);
		}
		
		AST::Node<AST::ValueList>
		ValueBuilder::makeValueList(AST::ValueList values,
		                            const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::ValueList(std::move(values)));
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeCastValue(const AST::Value::CastKind kind, AST::Node<AST::Type> fromType,
		                            AST::Node<AST::Type> toType, AST::Node<AST::Value> value,
		                            const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::Cast(kind, fromType, toType, value), start);
		}
		
	}
	
}
