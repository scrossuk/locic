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
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeValueNode(AST::ValueDecl* const value,
		                            const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, value);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeTernaryValue(AST::Node<AST::ValueDecl> conditionValue,
		                               AST::Node<AST::ValueDecl> ifTrueValue,
		                               AST::Node<AST::ValueDecl> ifFalseValue,
		                               const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::Ternary(std::move(conditionValue), std::move(ifTrueValue),
			                                         std::move(ifFalseValue)), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeLogicalOrValue(AST::Node<AST::ValueDecl> leftValue,
		                                 AST::Node<AST::ValueDecl> rightValue,
		                                 const Debug::SourcePosition& start) {
			return makeBinaryOpValue(std::move(leftValue), std::move(rightValue),
			                         AST::OP_LOGICALOR, start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeLogicalAndValue(AST::Node<AST::ValueDecl> leftValue,
		                                  AST::Node<AST::ValueDecl> rightValue,
		                                  const Debug::SourcePosition& start) {
			return makeBinaryOpValue(std::move(leftValue), std::move(rightValue),
			                         AST::OP_LOGICALAND, start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeBitwiseOrValue(AST::Node<AST::ValueDecl> leftValue,
		                                 AST::Node<AST::ValueDecl> rightValue,
		                                 const Debug::SourcePosition& start) {
			return makeBinaryOpValue(std::move(leftValue), std::move(rightValue),
			                         AST::OP_BITWISEOR, start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeBitwiseXorValue(AST::Node<AST::ValueDecl> leftValue,
		                                  AST::Node<AST::ValueDecl> rightValue,
		                                  const Debug::SourcePosition& start) {
			return makeBinaryOpValue(std::move(leftValue), std::move(rightValue),
			                         AST::OP_BITWISEXOR, start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeBitwiseAndValue(AST::Node<AST::ValueDecl> leftValue,
		                                  AST::Node<AST::ValueDecl> rightValue,
		                                  const Debug::SourcePosition& start) {
			return makeBinaryOpValue(std::move(leftValue), std::move(rightValue),
			                         AST::OP_BITWISEAND, start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeCapabilityTest(AST::Node<AST::TypeDecl> leftType,
		                                 AST::Node<AST::TypeDecl> rightType,
		                                 const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::CapabilityTest(std::move(leftType), std::move(rightType)),
			                     start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeBinaryOpValue(AST::Node<AST::ValueDecl> leftValue,
		                                AST::Node<AST::ValueDecl> rightValue,
		                                const AST::BinaryOpKind opKind,
		                                const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::BinaryOp(opKind, std::move(leftValue),
			                                          std::move(rightValue)), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeUnaryOpValue(AST::Node<AST::ValueDecl> operand,
		                               const AST::UnaryOpKind opKind,
		                               const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::UnaryOp(opKind, std::move(operand)), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeCallValue(AST::Node<AST::ValueDecl> callableValue,
		                            AST::Node<AST::ValueDeclList> parameters,
		                            const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::FunctionCall(std::move(callableValue), std::move(parameters)), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeIndexValue(AST::Node<AST::ValueDecl> value,
		                             AST::Node<AST::ValueDecl> indexValue,
		                             const Debug::SourcePosition& start) {
			return makeBinaryOpValue(std::move(value), std::move(indexValue), AST::OP_INDEX, start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeDerefValue(AST::Node<AST::ValueDecl> value,
		                             const Debug::SourcePosition& start) {
			return makeUnaryOpValue(std::move(value), AST::OP_DEREF, start);
		}
			
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeMemberAccess(AST::Node<AST::ValueDecl> value, String name,
		                               const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::MemberAccess(std::move(value), name), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeTemplatedMemberAccess(AST::Node<AST::ValueDecl> value, String name,
		                                        AST::Node<AST::ValueDeclList> templateArguments,
		                                        const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::TemplatedMemberAccess(std::move(value), name,
			                                                       std::move(templateArguments)), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeMergeValue(AST::Node<AST::ValueDecl> firstValue,
		                             AST::Node<AST::ValueDecl> secondValue,
		                             const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::Merge(std::move(firstValue), std::move(secondValue)), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeBracketedValue(AST::Node<AST::ValueDecl> value,
		                                 const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::Bracket(std::move(value)), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeTypeValue(AST::Node<AST::TypeDecl> type,
		                            const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::TypeRef(std::move(type)), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeLiteralValue(const Constant constant,
		                               const String literalSpecifier,
		                               const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			auto constantNode = AST::makeNode(location, new Constant(constant));
			return makeValueNode(AST::ValueDecl::Literal(literalSpecifier,
			                                         std::move(constantNode)), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeSymbolValue(AST::Node<AST::Symbol> symbol,
		                              const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::SymbolRef(std::move(symbol)), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeSelfValue(const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::Self(), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeThisValue(const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::This(), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeAlignOfValue(AST::Node<AST::TypeDecl> operand,
		                               const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::AlignOf(std::move(operand)), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeSizeOfValue(AST::Node<AST::TypeDecl> operand,
		                              const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::SizeOf(std::move(operand)), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeNewValue(AST::Node<AST::ValueDecl> placementArg,
		                           AST::Node<AST::ValueDecl> operand,
		                           const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::New(std::move(placementArg),
			                                         std::move(operand)),
			                     start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeSelfMemberAccess(const String name,
		                                   const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::MemberRef(name), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeInternalConstruct(AST::Node<AST::ValueDeclList> templateArguments,
		                                    AST::Node<AST::ValueDeclList> arguments,
		                                    const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::InternalConstruct(std::move(templateArguments),
			                                                   std::move(arguments)), start);
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeArrayLiteralValue(AST::Node<AST::ValueDeclList> values,
		                                    const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::ArrayLiteral(std::move(values)), start);
		}
		
		AST::Node<AST::ValueDeclList>
		ValueBuilder::makeValueList(AST::ValueDeclList values,
		                            const Debug::SourcePosition& start) {
			const auto location = reader_.locationWithRangeFrom(start);
			return AST::makeNode(location, new AST::ValueDeclList(std::move(values)));
		}
		
		AST::Node<AST::ValueDecl>
		ValueBuilder::makeCastValue(const AST::ValueDecl::CastKind kind, AST::Node<AST::TypeDecl> fromType,
		                            AST::Node<AST::TypeDecl> toType, AST::Node<AST::ValueDecl> value,
		                            const Debug::SourcePosition& start) {
			return makeValueNode(AST::ValueDecl::Cast(kind, std::move(fromType), std::move(toType),
			                                      std::move(value)), start);
		}
		
	}
	
}
