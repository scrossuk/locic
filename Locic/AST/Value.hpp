#ifndef LOCIC_AST_VALUE_HPP
#define LOCIC_AST_VALUE_HPP

#include <string>
#include <vector>

#include <Locic/Constant.hpp>
#include <Locic/Name.hpp>

#include <Locic/AST/Node.hpp>
#include <Locic/AST/Symbol.hpp>
#include <Locic/AST/Type.hpp>

namespace Locic {

	namespace AST {
	
		struct Value;
		
		typedef std::vector<Node<Value>> ValueList;
		
		struct Value {
			enum TypeEnum {
				NONE,
				BRACKET,
				CONSTANT,
				SYMBOLREF,
				MEMBERREF,
				TERNARY,
				CAST,
				INTERNALCONSTRUCT,
				MEMBERACCESS,
				FUNCTIONCALL
			} typeEnum;
			
			Node<Constant> constant;
			
			struct {
				Node<Value> value;
			} bracket;
			
			struct {
				Node<Symbol> symbol;
			} symbolRef;
			
			struct {
				std::string name;
			} memberRef;
			
			struct {
				Node<Value> condition, ifTrue, ifFalse;
			} ternary;
			
			enum CastKind {
				CAST_STATIC,
				CAST_CONST,
				CAST_REINTERPRET,
				CAST_DYNAMIC
			};
			
			struct {
				CastKind castKind;
				Node<Type> sourceType;
				Node<Type> targetType;
				Node<Value> value;
			} cast;
			
			struct {
				Node<ValueList> parameters;
			} internalConstruct;
			
			struct {
				Node<Value> object;
				std::string memberName;
			} memberAccess;
			
			struct {
				Node<Value> functionValue;
				Node<ValueList> parameters;
			} functionCall;
			
			inline Value() : typeEnum(NONE) { }
			
			inline Value(TypeEnum e) : typeEnum(e) { }
			
			inline static Value* Bracket(Node<Value> operand) {
				Value* value = new Value(BRACKET);
				value->bracket.value = operand;
				return value;
			}
			
			inline static Value* ConstantValue(const Node<Constant>& constant) {
				Value* value = new Value(CONSTANT);
				value->constant = constant;
				return value;
			}
			
			inline static Value* SymbolRef(const Node<Symbol>& symbol) {
				Value* value = new Value(SYMBOLREF);
				value->symbolRef.symbol = symbol;
				return value;
			}
			
			inline static Value* MemberRef(const std::string& name) {
				Value* value = new Value(MEMBERREF);
				value->memberRef.name = name;
				return value;
			}
			
			inline static Value* Ternary(Node<Value> condition, Node<Value> ifTrue, Node<Value> ifFalse) {
				Value* value = new Value(TERNARY);
				value->ternary.condition = condition;
				value->ternary.ifTrue = ifTrue;
				value->ternary.ifFalse = ifFalse;
				return value;
			}
			
			inline static Value* Cast(CastKind castKind, Node<Type> sourceType, Node<Type> targetType, Node<Value> operand) {
				Value* value = new Value(CAST);
				value->cast.castKind = castKind;
				value->cast.sourceType = sourceType;
				value->cast.targetType = targetType;
				value->cast.value = operand;
				return value;
			}
			
			inline static Value* InternalConstruct(const Node<ValueList>& parameters) {
				Value* value = new Value(INTERNALCONSTRUCT);
				value->internalConstruct.parameters = parameters;
				return value;
			}
			
			inline static Value* MemberAccess(Node<Value> object, const std::string& memberName) {
				Value* value = new Value(MEMBERACCESS);
				value->memberAccess.object = object;
				value->memberAccess.memberName = memberName;
				return value;
			}
			
			inline static Value* FunctionCall(Node<Value> functionValue, const Node<ValueList>& parameters) {
				Value* value = new Value(FUNCTIONCALL);
				value->functionCall.functionValue = functionValue;
				value->functionCall.parameters = parameters;
				return value;
			}
		};
		
	}
	
}

#endif
