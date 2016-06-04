#include <locic/AST.hpp>
#include <locic/Debug/SourceLocation.hpp>
#include <locic/Debug/SourceRange.hpp>
#include <locic/Debug/SourcePosition.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Parser/ValueBuilder.hpp>

namespace locic {
	
	namespace Parser {
		
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
			return makeValueNode(AST::Value::Ternary(std::move(conditionValue), std::move(ifTrueValue),
			                                         std::move(ifFalseValue)), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeLogicalOrValue(AST::Node<AST::Value> leftValue,
		                                 AST::Node<AST::Value> rightValue,
		                                 const Debug::SourcePosition& start) {
			return makeBinaryOpValue(std::move(leftValue), std::move(rightValue),
			                         AST::OP_LOGICALOR, start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeLogicalAndValue(AST::Node<AST::Value> leftValue,
		                                  AST::Node<AST::Value> rightValue,
		                                  const Debug::SourcePosition& start) {
			return makeBinaryOpValue(std::move(leftValue), std::move(rightValue),
			                         AST::OP_LOGICALAND, start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeBitwiseOrValue(AST::Node<AST::Value> leftValue,
		                                 AST::Node<AST::Value> rightValue,
		                                 const Debug::SourcePosition& start) {
			return makeBinaryOpValue(std::move(leftValue), std::move(rightValue),
			                         AST::OP_BITWISEOR, start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeBitwiseXorValue(AST::Node<AST::Value> leftValue,
		                                  AST::Node<AST::Value> rightValue,
		                                  const Debug::SourcePosition& start) {
			return makeBinaryOpValue(std::move(leftValue), std::move(rightValue),
			                         AST::OP_BITWISEXOR, start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeBitwiseAndValue(AST::Node<AST::Value> leftValue,
		                                  AST::Node<AST::Value> rightValue,
		                                  const Debug::SourcePosition& start) {
			return makeBinaryOpValue(std::move(leftValue), std::move(rightValue),
			                         AST::OP_BITWISEAND, start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeCapabilityTest(AST::Node<AST::TypeDecl> leftType,
		                                 AST::Node<AST::TypeDecl> rightType,
		                                 const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::CapabilityTest(std::move(leftType), std::move(rightType)),
			                     start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeBinaryOpValue(AST::Node<AST::Value> leftValue,
		                                AST::Node<AST::Value> rightValue,
		                                const AST::BinaryOpKind opKind,
		                                const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::BinaryOp(opKind, std::move(leftValue),
			                                          std::move(rightValue)), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeUnaryOpValue(AST::Node<AST::Value> operand,
		                               const AST::UnaryOpKind opKind,
		                               const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::UnaryOp(opKind, std::move(operand)), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeCallValue(AST::Node<AST::Value> callableValue,
		                            AST::Node<AST::ValueList> parameters,
		                            const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::FunctionCall(std::move(callableValue), std::move(parameters)), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeIndexValue(AST::Node<AST::Value> value,
		                             AST::Node<AST::Value> indexValue,
		                             const Debug::SourcePosition& start) {
			return makeBinaryOpValue(std::move(value), std::move(indexValue), AST::OP_INDEX, start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeDerefValue(AST::Node<AST::Value> value,
		                             const Debug::SourcePosition& start) {
			return makeUnaryOpValue(std::move(value), AST::OP_DEREF, start);
		}
			
		AST::Node<AST::Value>
		ValueBuilder::makeMemberAccess(AST::Node<AST::Value> value, String name,
		                               const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::MemberAccess(std::move(value), name), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeTemplatedMemberAccess(AST::Node<AST::Value> value, String name,
		                                        AST::Node<AST::ValueList> templateArguments,
		                                        const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::TemplatedMemberAccess(std::move(value), name,
			                                                       std::move(templateArguments)), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeMergeValue(AST::Node<AST::Value> firstValue,
		                             AST::Node<AST::Value> secondValue,
		                             const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::Merge(std::move(firstValue), std::move(secondValue)), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeBracketedValue(AST::Node<AST::Value> value,
		                                 const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::Bracket(std::move(value)), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeTypeValue(AST::Node<AST::TypeDecl> type,
		                            const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::TypeRef(std::move(type)), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeLiteralValue(const Constant constant,
		                               const String literalSpecifier,
		                               const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			auto constantNode = AST::makeNode(location, new Constant(constant));
			return makeValueNode(AST::Value::Literal(literalSpecifier,
			                                         std::move(constantNode)), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeRefValue(AST::Node<AST::TypeDecl> targetType,
		                           AST::Node<AST::Value> value,
		                           const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::Ref(std::move(targetType), std::move(value)), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeLvalValue(AST::Node<AST::TypeDecl> targetType,
		                            AST::Node<AST::Value> value,
		                            const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::Lval(std::move(targetType), std::move(value)), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeNoRefValue(AST::Node<AST::Value> value,
		                             const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::NoRef(std::move(value)), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeNoLvalValue(AST::Node<AST::Value> value,
		                              const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::NoLval(std::move(value)), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeSymbolValue(AST::Node<AST::Symbol> symbol,
		                              const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::SymbolRef(std::move(symbol)), start);
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
		ValueBuilder::makeAlignOfValue(AST::Node<AST::TypeDecl> operand,
		                               const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::AlignOf(std::move(operand)), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeSizeOfValue(AST::Node<AST::TypeDecl> operand,
		                              const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::SizeOf(std::move(operand)), start);
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
			return makeValueNode(AST::Value::InternalConstruct(std::move(templateArguments),
			                                                   std::move(arguments)), start);
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeArrayLiteralValue(AST::Node<AST::ValueList> values,
		                                    const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::ArrayLiteral(std::move(values)), start);
		}
		
		AST::Node<AST::ValueList>
		ValueBuilder::makeValueList(AST::ValueList values,
		                            const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::ValueList(std::move(values)));
		}
		
		AST::Node<AST::Value>
		ValueBuilder::makeCastValue(const AST::Value::CastKind kind, AST::Node<AST::TypeDecl> fromType,
		                            AST::Node<AST::TypeDecl> toType, AST::Node<AST::Value> value,
		                            const Debug::SourcePosition& start) {
			return makeValueNode(AST::Value::Cast(kind, std::move(fromType), std::move(toType),
			                                      std::move(value)), start);
		}
		
	}
	
}
