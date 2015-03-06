#include <assert.h>
#include <stdio.h>

#include <limits>
#include <list>
#include <map>
#include <stdexcept>
#include <string>

#include <locic/AST.hpp>
#include <locic/Debug.hpp>
#include <locic/MakeArray.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/Literal.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/MethodSet.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/Template.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		const std::string binaryOpToString(const AST::BinaryOpKind kind) {
			switch (kind) {
				case AST::OP_ISEQUAL:
					return "==";
				case AST::OP_NOTEQUAL:
					return "!=";
				case AST::OP_LESSTHAN:
					return "<";
				case AST::OP_LESSTHANOREQUAL:
					return "<=";
				case AST::OP_GREATERTHAN:
					return ">";
				case AST::OP_GREATERTHANOREQUAL:
					return ">=";
				case AST::OP_ADD:
					return "+";
				case AST::OP_SUBTRACT:
					return "-";
				case AST::OP_MULTIPLY:
					return "*";
				case AST::OP_DIVIDE:
					return "/";
				case AST::OP_MODULO:
					return "%";
				case AST::OP_LOGICALAND:
					return "&&";
				case AST::OP_LOGICALOR:
					return "||";
				case AST::OP_BITWISEAND:
					return "&";
				case AST::OP_BITWISEOR:
					return "|";
				case AST::OP_LEFTSHIFT:
					return "<<";
				case AST::OP_RIGHTSHIFT:
					return ">>";
			}
			
			std::terminate();
		}
		
		const char* binaryOpNameCString(const AST::BinaryOpKind kind) {
			switch (kind) {
				case AST::OP_ISEQUAL:
					return "equal";
				case AST::OP_NOTEQUAL:
					return "not_equal";
				case AST::OP_LESSTHAN:
					return "less_than";
				case AST::OP_LESSTHANOREQUAL:
					return "less_than_or_equal";
				case AST::OP_GREATERTHAN:
					return "greater_than";
				case AST::OP_GREATERTHANOREQUAL:
					return "greater_than_or_equal";
				case AST::OP_ADD:
					return "add";
				case AST::OP_SUBTRACT:
					return "subtract";
				case AST::OP_MULTIPLY:
					return "multiply";
				case AST::OP_DIVIDE:
					return "divide";
				case AST::OP_MODULO:
					return "modulo";
				case AST::OP_LOGICALAND:
					return "logical_and";
				case AST::OP_LOGICALOR:
					return "logical_or";
				case AST::OP_BITWISEAND:
					return "bitwise_and";
				case AST::OP_BITWISEOR:
					return "bitwise_or";
				case AST::OP_LEFTSHIFT:
					return "left_shift";
				case AST::OP_RIGHTSHIFT:
					return "right_shift";
			}
			
			std::terminate();
		}
		
		String binaryOpName(Context& context, const AST::BinaryOpKind kind) {
			const char* const cString = binaryOpNameCString(kind);
			return context.getCString(cString);
		}
		
		bool HasBinaryOp(Context& context, const SEM::Value& value, AST::BinaryOpKind opKind, const Debug::SourceLocation& location) {
			const auto derefType = getDerefType(value.type());
			
			if (!derefType->isObjectOrTemplateVar()) {
				throw ErrorException(makeString("Can't perform binary operator '%s' for non-object type '%s' at position %s.",
					binaryOpToString(opKind).c_str(), derefType->toString().c_str(), location.toString().c_str()));
			}
			
			const auto methodSet = getTypeMethodSet(context, derefType);
			const auto methodName = binaryOpName(context, opKind);
			return methodSet->hasMethod(CanonicalizeMethodName(methodName));
		}
		
		SEM::Value GetBinaryOp(Context& context, SEM::Value value, const AST::BinaryOpKind opKind, const Debug::SourceLocation& location) {
			return GetMethod(context, std::move(value), binaryOpName(context, opKind), location);
		}
		
		SEM::Value MakeMemberAccess(Context& context, SEM::Value rawValue, const String& memberName, const Debug::SourceLocation& location) {
			auto value = tryDissolveValue(context, derefValue(std::move(rawValue)), location);
			const auto derefType = getStaticDerefType(getDerefType(value.type()));
			
			if (!derefType->isObjectOrTemplateVar()) {
				throw ErrorException(makeString("Can't access member '%s' of type '%s' at position %s.",
					memberName.c_str(), derefType->toString().c_str(), location.toString().c_str()));
			}
			
			const auto methodSet = getTypeMethodSet(context, derefType);
			
			const auto filterReason = methodSet->getFilterReason(CanonicalizeMethodName(memberName));
			
			// Look for methods.
			if (filterReason != MethodSet::NotFound) {
				if (getDerefType(value.type())->isStaticRef()) {
					return GetStaticMethod(context, std::move(value), memberName, location);
				} else {
					return GetMethod(context, std::move(value), memberName, location);
				}
			}
			
			// TODO: this should be replaced by falling back on 'property' methods.
			// Look for variables.
			if (derefType->isObject()) {
				const auto typeInstance = derefType->getObjectType();
				if (typeInstance->isDatatype() || typeInstance->isException() || typeInstance->isStruct()) {
					const auto variableIterator = typeInstance->namedVariables().find(memberName);
					if (variableIterator != typeInstance->namedVariables().end()) {
						return createMemberVarRef(context, std::move(value), variableIterator->second);
					}
				}
			}
			
			throw ErrorException(makeString("Can't access member '%s' in type '%s' at position %s.",
				memberName.c_str(), derefType->toString().c_str(), location.toString().c_str()));
		}
		
		static Name getCanonicalName(const Name& name) {
			return name.getPrefix() + CanonicalizeMethodName(name.last());
		}
		
		static SearchResult performSymbolLookup(Context& context, const Name& name) {
			const auto searchResult = performSearch(context, name);
			if (!searchResult.isNone()) return searchResult;
			
			// Fall back on looking for canonicalized static method names.
			const auto functionSearchResult = performSearch(context, getCanonicalName(name));
			if (!functionSearchResult.isFunction()) {
				return SearchResult::None();
			}
			
			return functionSearchResult;
		}
		
		SEM::Value ConvertValueData(Context& context, const AST::Node<AST::Value>& astValueNode) {
			assert(astValueNode.get() != nullptr);
			const auto& location = astValueNode.location();
			
			switch (astValueNode->typeEnum) {
				case AST::Value::BRACKET: {
					return ConvertValue(context, astValueNode->bracket.value);
				}
				case AST::Value::SELF: {
					return getSelfValue(context, location);
				}
				case AST::Value::THIS: {
					return getThisValue(context, location);
				}
				case AST::Value::LITERAL: {
					const auto& specifier = astValueNode->literal.specifier;
					auto& constant = *(astValueNode->literal.constant);
					return getLiteralValue(context, specifier, constant, location);
				}
				case AST::Value::SYMBOLREF: {
					const auto& astSymbolNode = astValueNode->symbolRef.symbol;
					const Name name = astSymbolNode->createName();
					
					const auto searchResult = performSymbolLookup(context, name);
					
					// Get a map from template variables to their values (i.e. types).
					const auto templateVarMap = GenerateTemplateVarMap(context, astSymbolNode);
					
					if (searchResult.isNone()) {
						throw ErrorException(makeString("Couldn't find symbol or value '%s' at %s.",
							name.toString().c_str(), location.toString().c_str()));
					} else if (searchResult.isFunction()) {
						const auto function = searchResult.function();
						assert(function != nullptr && "Function pointer must not be NULL (as indicated by isFunction() being true)");
						
						auto functionTemplateArguments = GetTemplateValues(templateVarMap, function->templateVariables());
						const auto functionType = function->type()->substitute(templateVarMap);
						
						if (function->isMethod()) {
							if (!function->isStaticMethod()) {
								throw ErrorException(makeString("Cannot refer directly to non-static class method '%s' at %s.",
									name.toString().c_str(), location.toString().c_str()));
							}
							
							const auto typeSearchResult = performSearch(context, name.getPrefix());
							assert(typeSearchResult.isTypeInstance());
							
							const auto typeInstance = typeSearchResult.typeInstance();
							
							auto parentTemplateArguments = GetTemplateValues(templateVarMap, typeInstance->templateVariables());
							const auto parentType = SEM::Type::Object(typeInstance, std::move(parentTemplateArguments));
							
							return SEM::Value::FunctionRef(parentType, function, std::move(functionTemplateArguments), functionType);
						} else {
							return SEM::Value::FunctionRef(nullptr, function, std::move(functionTemplateArguments), functionType);
						}
					} else if (searchResult.isTypeInstance()) {
						const auto typeInstance = searchResult.typeInstance();
						
						if (typeInstance->isInterface()) {
							throw ErrorException(makeString("Can't construct interface type '%s' at %s.",
								name.toString().c_str(), location.toString().c_str()));
						}
						
						const auto typenameType = getBuiltInType(context.scopeStack(), context.getCString("typename_t"), {});
						const auto parentType = SEM::Type::Object(typeInstance, GetTemplateValues(templateVarMap, typeInstance->templateVariables()));
						return SEM::Value::TypeRef(parentType, typenameType->createStaticRefType(parentType));
					} else if (searchResult.isTypeAlias()) {
						const auto typeAlias = searchResult.typeAlias();
						auto templateArguments = GetTemplateValues(templateVarMap, typeAlias->templateVariables());
						assert(templateArguments.size() == typeAlias->templateVariables().size());
						
						const auto typenameType = getBuiltInType(context.scopeStack(), context.getCString("typename_t"), {});
						const auto resolvedType = SEM::Type::Alias(typeAlias, std::move(templateArguments))->resolveAliases();
						return SEM::Value::TypeRef(resolvedType, typenameType->createStaticRefType(resolvedType));
					} else if (searchResult.isVar()) {
						// Variables must just be a single plain string,
						// and be a relative name (so no precending '::').
						assert(astSymbolNode->size() == 1);
						assert(astSymbolNode->isRelative());
						assert(astSymbolNode->first()->templateArguments()->empty());
						const auto var = searchResult.var();
						var->setUsed();
						return SEM::Value::LocalVar(var, getBuiltInType(context.scopeStack(), context.getCString("__ref"), { var->type() })->createRefType(var->type()));
					} else if (searchResult.isTemplateVar()) {
						assert(templateVarMap.empty() && "Template vars cannot have template arguments.");
						const auto templateVar = searchResult.templateVar();
						const auto typenameType = getBuiltInType(context.scopeStack(), context.getCString("typename_t"), {});
						const auto templateVarType = SEM::Type::TemplateVarRef(templateVar);
						return SEM::Value::TypeRef(templateVarType, typenameType->createStaticRefType(templateVarType));
					}
					
					std::terminate();
				}
				case AST::Value::MEMBERREF: {
					const auto& memberName = astValueNode->memberRef.name;
					auto selfValue = getSelfValue(context, location);
					
					const auto derefType = getDerefType(selfValue.type());
					assert(derefType->isObject());
					
					const auto typeInstance = derefType->getObjectType();
					const auto variableIterator = typeInstance->namedVariables().find(memberName);
					
					if (variableIterator == typeInstance->namedVariables().end()) {
						throw ErrorException(makeString("Member variable '@%s' not found at position %s.",
							memberName.c_str(), location.toString().c_str()));
					}
					
					return createMemberVarRef(context, std::move(selfValue), variableIterator->second);
				}
				case AST::Value::SIZEOF: {
					return SEM::Value::SizeOf(ConvertType(context, astValueNode->sizeOf.type), getBuiltInType(context.scopeStack(), context.getCString("size_t"), {}));
				}
				case AST::Value::UNARYOP: {
					const auto unaryOp = astValueNode->unaryOp.kind;
					auto operand = ConvertValue(context, astValueNode->unaryOp.operand);
					
					switch (unaryOp) {
						case AST::OP_PLUS: {
							auto opMethod = GetMethod(context, std::move(operand), context.getCString("plus"), location);
							return CallValue(context, std::move(opMethod), {}, location);
						}
						case AST::OP_MINUS: {
							auto opMethod = GetMethod(context, std::move(operand), context.getCString("minus"), location);
							return CallValue(context, std::move(opMethod), {}, location);
						}
						case AST::OP_NOT: {
							auto opMethod = GetMethod(context, std::move(operand), context.getCString("not"), location);
							return CallValue(context, std::move(opMethod), {}, location);
						}
						case AST::OP_DEREF: {
							auto opMethod = GetMethod(context, std::move(operand), context.getCString("deref"), location);
							return CallValue(context, std::move(opMethod), {}, location);
						}
						case AST::OP_ADDRESS: {
							auto opMethod = GetSpecialMethod(context, derefOrBindValue(context, std::move(operand)), context.getCString("address"), location);
							return CallValue(context, std::move(opMethod), {}, location);
						}
						case AST::OP_MOVE: {
							auto opMethod = GetSpecialMethod(context, derefOrBindValue(context, std::move(operand)), context.getCString("move"), location);
							return CallValue(context, std::move(opMethod), {}, location);
						}
					}
					
					std::terminate();
				}
				case AST::Value::BINARYOP: {
					const auto binaryOp = astValueNode->binaryOp.kind;
					auto leftOperand = ConvertValue(context, astValueNode->binaryOp.leftOperand);
					auto rightOperand = ConvertValue(context, astValueNode->binaryOp.rightOperand);
					
					switch (binaryOp) {
						case AST::OP_ADD: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("add"), location);
							return CallValue(context, std::move(opMethod), makeArray( std::move(rightOperand) ), location);
						}
						case AST::OP_SUBTRACT: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("subtract"), location);
							return CallValue(context, std::move(opMethod), makeArray( std::move(rightOperand) ), location);
						}
						case AST::OP_MULTIPLY: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("multiply"), location);
							return CallValue(context, std::move(opMethod), makeArray( std::move(rightOperand) ), location);
						}
						case AST::OP_DIVIDE: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("divide"), location);
							return CallValue(context, std::move(opMethod), makeArray( std::move(rightOperand) ), location);
						}
						case AST::OP_MODULO: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("modulo"), location);
							return CallValue(context, std::move(opMethod), makeArray( std::move(rightOperand) ), location);
						}
						case AST::OP_ISEQUAL: {
							auto objectValue = tryDissolveValue(context, derefValue(std::move(leftOperand)), location);
							if (HasBinaryOp(context, objectValue, binaryOp, location)) {
								auto opMethod = GetBinaryOp(context, std::move(objectValue), binaryOp, location);
								return CallValue(context, std::move(opMethod), makeArray( std::move(rightOperand) ), location);
							} else {
								// Fall back on 'compare' method.
								auto compareMethod = GetMethod(context, std::move(objectValue), context.getCString("compare"), location);
								auto compareResult = CallValue(context, std::move(compareMethod), makeArray( std::move(rightOperand) ), location);
								auto isEqualMethod = GetMethod(context, std::move(compareResult), context.getCString("isEqual"), location);
								return CallValue(context, std::move(isEqualMethod), {}, location);
							}
						}
						case AST::OP_NOTEQUAL: {
							auto objectValue = tryDissolveValue(context, derefValue(std::move(leftOperand)), location);
							if (HasBinaryOp(context, objectValue, binaryOp, location)) {
								auto opMethod = GetBinaryOp(context, std::move(objectValue), binaryOp, location);
								return CallValue(context, std::move(opMethod), makeArray( std::move(rightOperand) ), location);
							} else {
								// Fall back on 'compare' method.
								auto compareMethod = GetMethod(context, std::move(objectValue), context.getCString("compare"), location);
								auto compareResult = CallValue(context, std::move(compareMethod), makeArray( std::move(rightOperand) ), location);
								auto isNotEqualMethod = GetMethod(context, std::move(compareResult), context.getCString("isNotEqual"), location);
								return CallValue(context, std::move(isNotEqualMethod), {}, location);
							}
						}
						case AST::OP_LESSTHAN: {
							auto objectValue = tryDissolveValue(context, derefValue(std::move(leftOperand)), location);
							if (HasBinaryOp(context, objectValue, binaryOp, location)) {
								auto opMethod = GetBinaryOp(context, std::move(objectValue), binaryOp, location);
								return CallValue(context, std::move(opMethod), makeArray( std::move(rightOperand) ), location);
							} else {
								// Fall back on 'compare' method.
								auto compareMethod = GetMethod(context, std::move(objectValue), context.getCString("compare"), location);
								auto compareResult = CallValue(context, std::move(compareMethod), makeArray( std::move(rightOperand) ), location);
								auto isNotEqualMethod = GetMethod(context, std::move(compareResult), context.getCString("isLessThan"), location);
								return CallValue(context, std::move(isNotEqualMethod), {}, location);
							}
						}
						case AST::OP_LESSTHANOREQUAL: {
							auto objectValue = tryDissolveValue(context, derefValue(std::move(leftOperand)), location);
							if (HasBinaryOp(context, objectValue, binaryOp, location)) {
								auto opMethod = GetBinaryOp(context, std::move(objectValue), binaryOp, location);
								return CallValue(context, std::move(opMethod), makeArray( std::move(rightOperand) ), location);
							} else {
								// Fall back on 'compare' method.
								auto compareMethod = GetMethod(context, std::move(objectValue), context.getCString("compare"), location);
								auto compareResult = CallValue(context, std::move(compareMethod), makeArray( std::move(rightOperand) ), location);
								auto isNotEqualMethod = GetMethod(context, std::move(compareResult), context.getCString("isLessThanOrEqual"), location);
								return CallValue(context, std::move(isNotEqualMethod), {}, location);
							}
						}
						case AST::OP_GREATERTHAN: {
							auto objectValue = tryDissolveValue(context, derefValue(std::move(leftOperand)), location);
							if (HasBinaryOp(context, objectValue, binaryOp, location)) {
								auto opMethod = GetBinaryOp(context, std::move(objectValue), binaryOp, location);
								return CallValue(context, std::move(opMethod), makeArray( std::move(rightOperand) ), location);
							} else {
								// Fall back on 'compare' method.
								auto compareMethod = GetMethod(context, std::move(objectValue), context.getCString("compare"), location);
								auto compareResult = CallValue(context, std::move(compareMethod), makeArray( std::move(rightOperand) ), location);
								auto isNotEqualMethod = GetMethod(context, std::move(compareResult), context.getCString("isGreaterThan"), location);
								return CallValue(context, std::move(isNotEqualMethod), {}, location);
							}
						}
						case AST::OP_GREATERTHANOREQUAL: {
							auto objectValue = tryDissolveValue(context, derefValue(std::move(leftOperand)), location);
							if (HasBinaryOp(context, objectValue, binaryOp, location)) {
								auto opMethod = GetBinaryOp(context, std::move(objectValue), binaryOp, location);
								return CallValue(context, std::move(opMethod), makeArray( std::move(rightOperand) ), location);
							} else {
								// Fall back on 'compare' method.
								auto compareMethod = GetMethod(context, std::move(objectValue), context.getCString("compare"), location);
								auto compareResult = CallValue(context, std::move(compareMethod), makeArray( std::move(rightOperand) ), location);
								auto isNotEqualMethod = GetMethod(context, std::move(compareResult), context.getCString("isGreaterThanOrEqual"), location);
								return CallValue(context, std::move(isNotEqualMethod), {}, location);
							}
						}
						case AST::OP_LOGICALAND: {
							const auto boolType = getBuiltInType(context.scopeStack(), context.getCString("bool"), {});
							auto boolValue = ImplicitCast(context, std::move(leftOperand), boolType->createConstType(), location);
							
							// Logical AND only evaluates the right operand if the left
							// operand is TRUE, otherwise it returns FALSE.
							return SEM::Value::Ternary(std::move(boolValue), std::move(rightOperand), SEM::Value::Constant(Constant::False(), boolType));
						}
						case AST::OP_LOGICALOR: {
							const auto boolType = getBuiltInType(context.scopeStack(), context.getCString("bool"), {});
							auto boolValue = ImplicitCast(context, std::move(leftOperand), boolType->createConstType(), location);
							
							// Logical OR only evaluates the right operand if the left
							// operand is FALSE, otherwise it returns TRUE.
							return SEM::Value::Ternary(std::move(boolValue), SEM::Value::Constant(Constant::True(), boolType), std::move(rightOperand));
						}
						case AST::OP_BITWISEAND: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("bitwise_and"), location);
							return CallValue(context, std::move(opMethod), makeArray( std::move(rightOperand) ), location);
						}
						case AST::OP_BITWISEOR: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("bitwise_or"), location);
							return CallValue(context, std::move(opMethod), makeArray( std::move(rightOperand) ), location);
						}
						case AST::OP_LEFTSHIFT: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("left_shift"), location);
							return CallValue(context, std::move(opMethod), makeArray( std::move(rightOperand) ), location);
						}
						case AST::OP_RIGHTSHIFT: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("right_shift"), location);
							return CallValue(context, std::move(opMethod), makeArray( std::move(rightOperand) ), location);
						}
					}
					
					std::terminate();
				}
				case AST::Value::TERNARY: {
					auto cond = ConvertValue(context, astValueNode->ternary.condition);
					
					const auto boolType = getBuiltInType(context.scopeStack(), context.getCString("bool"), {});
					auto boolValue = ImplicitCast(context, std::move(cond), boolType->createConstType(), location);
					
					auto ifTrue = ConvertValue(context, astValueNode->ternary.ifTrue);
					auto ifFalse = ConvertValue(context, astValueNode->ternary.ifFalse);
					
					const auto targetType = UnifyTypes(context, ifTrue.type(), ifFalse.type(), location);
					
					auto castIfTrue = ImplicitCast(context, std::move(ifTrue), targetType, location);
					auto castIfFalse = ImplicitCast(context, std::move(ifFalse), targetType, location);
					
					return SEM::Value::Ternary(std::move(boolValue), std::move(castIfTrue), std::move(castIfFalse));
				}
				case AST::Value::CAST: {
					auto sourceValue = ConvertValue(context, astValueNode->cast.value);
					const auto sourceType = ConvertType(context, astValueNode->cast.sourceType);
					const auto targetType = ConvertType(context, astValueNode->cast.targetType);
					
					switch(astValueNode->cast.castKind) {
						case AST::Value::CAST_CONST:
							throw ErrorException("const_cast not yet implemented.");
						case AST::Value::CAST_DYNAMIC:
							throw ErrorException("dynamic_cast not yet implemented.");
						case AST::Value::CAST_REINTERPRET:
							if (!sourceType->isPrimitive() || sourceType->getObjectType()->name().last() != "__ptr"
								|| !targetType->isPrimitive() || targetType->getObjectType()->name().last() != "__ptr") {
								throw ErrorException(makeString("reinterpret_cast currently only supports ptr<T>, "
									"in cast from value %s of type %s to type %s at position %s.",
									sourceValue.toString().c_str(), sourceType->toString().c_str(),
									targetType->toString().c_str(), location.toString().c_str()));
							}
							return SEM::Value::Reinterpret(ImplicitCast(context, std::move(sourceValue), sourceType, location), targetType);
					}
					
					std::terminate();
				}
				case AST::Value::LVAL: {
					auto sourceValue = ConvertValue(context, astValueNode->makeLval.value);
					
					if (sourceValue.type()->isLval()) {
						throw ErrorException(makeString("Can't create lval of value that is already a lval, for value '%s' at position %s.",
							sourceValue.toString().c_str(), location.toString().c_str()));
					}
					
					if (sourceValue.type()->isRef()) {
						throw ErrorException(makeString("Can't create value that is both an lval and a ref, for value '%s' at position %s.",
							sourceValue.toString().c_str(), location.toString().c_str()));
					}
					
					const auto targetType = ConvertType(context, astValueNode->makeLval.targetType);
					return SEM::Value::Lval(targetType, std::move(sourceValue));
				}
				case AST::Value::NOLVAL: {
					auto sourceValue = ConvertValue(context, astValueNode->makeNoLval.value);
					
					if (!getDerefType(sourceValue.type())->isLval()) {
						throw ErrorException(makeString("Can't use 'nolval' operator on non-lval value '%s' at position '%s'.",
							sourceValue.toString().c_str(), location.toString().c_str()));
					}
					
					return SEM::Value::NoLval(std::move(sourceValue));
				}
				case AST::Value::REF: {
					auto sourceValue = ConvertValue(context, astValueNode->makeRef.value);
					
					if (sourceValue.type()->isLval()) {
						throw ErrorException(makeString("Can't create value that is both an lval and a ref, for value '%s' at position %s.",
							sourceValue.toString().c_str(), location.toString().c_str()));
					}
					
					if (sourceValue.type()->isRef()) {
						throw ErrorException(makeString("Can't create ref of value that is already a ref, for value '%s' at position %s.",
							sourceValue.toString().c_str(), location.toString().c_str()));
					}
					
					const auto targetType = ConvertType(context, astValueNode->makeRef.targetType);
					return SEM::Value::Ref(targetType, std::move(sourceValue));
				}
				case AST::Value::NOREF: {
					auto sourceValue = ConvertValue(context, astValueNode->makeNoRef.value);
					
					if (!sourceValue.type()->isRef()) {
						throw ErrorException(makeString("Can't use 'noref' operator on non-ref value '%s' at position '%s'.",
							sourceValue.toString().c_str(), location.toString().c_str()));
					}
					
					return SEM::Value::NoRef(std::move(sourceValue));
				}
				case AST::Value::INTERNALCONSTRUCT: {
					const auto& astParameterValueNodes = astValueNode->internalConstruct.parameters;
					
					const auto thisTypeInstance = lookupParentType(context.scopeStack());
					
					if (thisTypeInstance == nullptr) {
						throw ErrorException(makeString("Cannot call internal constructor in non-method at position %s.",
							location.toString().c_str()));
					}
					
					if (astParameterValueNodes->size() != thisTypeInstance->variables().size()) {
						throw ErrorException(makeString("Internal constructor called "
							   "with wrong number of arguments; received %llu, expected %llu at position %s.",
							(unsigned long long) astParameterValueNodes->size(),
							(unsigned long long) thisTypeInstance->variables().size(),
							location.toString().c_str()));
					}
					
					std::vector<SEM::Value> semValues;
					
					for (size_t i = 0; i < thisTypeInstance->variables().size(); i++) {
						const auto semVar = thisTypeInstance->variables().at(i);
						auto semValue = ConvertValue(context, astParameterValueNodes->at(i));
						auto semParam = ImplicitCast(context, std::move(semValue), semVar->constructType(), location);
						semValues.push_back(std::move(semParam));
					}
					
					return SEM::Value::InternalConstruct(thisTypeInstance, std::move(semValues));
				}
				case AST::Value::MEMBERACCESS: {
					const auto& memberName = astValueNode->memberAccess.memberName;
					
					auto object = ConvertValue(context, astValueNode->memberAccess.object);
					return MakeMemberAccess(context, std::move(object), memberName, astValueNode.location());
				}
				case AST::Value::TEMPLATEDMEMBERACCESS: {
					const auto& memberName = astValueNode->templatedMemberAccess.memberName;
					auto object = ConvertValue(context, astValueNode->templatedMemberAccess.object);
					
					SEM::TypeArray templateArguments;
					templateArguments.reserve(astValueNode->templatedMemberAccess.typeList->size());
					
					for (const auto typeArg: *(astValueNode->templatedMemberAccess.typeList)) {
						templateArguments.push_back(ConvertType(context, typeArg));
					}
					
					return GetTemplatedMethod(context, std::move(object), memberName, std::move(templateArguments), astValueNode.location());
				}
				case AST::Value::FUNCTIONCALL: {
					auto functionValue = ConvertValue(context, astValueNode->functionCall.functionValue);
					
					std::vector<SEM::Value> argumentValues;
					
					for (const auto& astArgumentValueNode: *(astValueNode->functionCall.parameters)) {
						argumentValues.push_back(ConvertValue(context, astArgumentValueNode));
					}
					
					return CallValue(context, std::move(functionValue), std::move(argumentValues), location);
				}
			}
			
			std::terminate();
		}
		
		Debug::ValueInfo makeValueInfo(const AST::Node<AST::Value>& astValueNode) {
			Debug::ValueInfo valueInfo;
			valueInfo.location = astValueNode.location();
			return valueInfo;
		}
		
		SEM::Value ConvertValue(Context& context, const AST::Node<AST::Value>& astValueNode) {
			auto semValue = ConvertValueData(context, astValueNode);
			semValue.setDebugInfo(makeValueInfo(astValueNode));
			return semValue;
		}
		
	}
	
}


