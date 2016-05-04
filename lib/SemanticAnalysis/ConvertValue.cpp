#include <assert.h>
#include <stdio.h>

#include <limits>
#include <list>
#include <map>
#include <stdexcept>
#include <string>

#include <locic/AST.hpp>
#include <locic/Debug.hpp>
#include <locic/Support/MakeArray.hpp>
#include <locic/SEM.hpp>

#include <locic/Frontend/DiagnosticArray.hpp>

#include <locic/SemanticAnalysis/AliasTypeResolver.hpp>
#include <locic/SemanticAnalysis/CallValue.hpp>
#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/GetMethod.hpp>
#include <locic/SemanticAnalysis/GetMethodSet.hpp>
#include <locic/SemanticAnalysis/Literal.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/MethodSet.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/SearchResult.hpp>
#include <locic/SemanticAnalysis/Template.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>
#include <locic/SemanticAnalysis/TypeResolver.hpp>

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
				case AST::OP_BITWISEXOR:
					return "^";
				case AST::OP_LEFTSHIFT:
					return "<<";
				case AST::OP_RIGHTSHIFT:
					return ">>";
				case AST::OP_INDEX:
					return "[]";
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
				case AST::OP_BITWISEXOR:
					return "bitwise_xor";
				case AST::OP_LEFTSHIFT:
					return "left_shift";
				case AST::OP_RIGHTSHIFT:
					return "right_shift";
				case AST::OP_INDEX:
					return "index";
			}
			
			std::terminate();
		}
		
		String binaryOpName(Context& context, const AST::BinaryOpKind kind) {
			const char* const cString = binaryOpNameCString(kind);
			return context.getCString(cString);
		}
		
		bool HasBinaryOp(Context& context, const SEM::Value& value, AST::BinaryOpKind opKind,
		                 const Debug::SourceLocation& /*location*/) {
			const auto derefType = getDerefType(value.type());
			assert(derefType->isObjectOrTemplateVar());
			
			const auto methodSet = getTypeMethodSet(context, derefType);
			const auto methodName = binaryOpName(context, opKind);
			return methodSet->hasMethod(CanonicalizeMethodName(methodName));
		}
		
		SEM::Value GetBinaryOp(Context& context, SEM::Value value, const AST::BinaryOpKind opKind, const Debug::SourceLocation& location) {
			return GetMethod(context, std::move(value), binaryOpName(context, opKind), location);
		}
		
		class CannotFindMemberInTypeDiag: public Error {
		public:
			CannotFindMemberInTypeDiag(String name, const SEM::Type* type)
			: name_(name), typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("cannot find member '%s' in type '%s'",
				                  name_.c_str(), typeString_.c_str());
			}
			
		private:
			String name_;
			std::string typeString_;
			
		};
		
		SEM::Value MakeMemberAccess(Context& context, SEM::Value rawValue, const String& memberName, const Debug::SourceLocation& location) {
			auto value = tryDissolveValue(context, derefValue(std::move(rawValue)), location);
			const auto derefType = getStaticDerefType(getDerefType(value.type()->resolveAliases()));
			assert(derefType->isObjectOrTemplateVar());
			
			const auto methodSet = getTypeMethodSet(context, derefType);
			
			// Look for methods.
			if (methodSet->hasMethod(CanonicalizeMethodName(memberName))) {
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
				if (typeInstance->isDatatype() || typeInstance->isException() || typeInstance->isStruct() || typeInstance->isUnion()) {
					const auto variableIterator = typeInstance->namedVariables().find(memberName);
					if (variableIterator != typeInstance->namedVariables().end()) {
						return createMemberVarRef(context, std::move(value), *(variableIterator->second));
					}
				}
			}
			
			context.issueDiag(CannotFindMemberInTypeDiag(memberName, derefType),
			                  location);
			throw SkipException();
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
		
		class UseOfUndeclaredIdentifierDiag: public Error {
		public:
			UseOfUndeclaredIdentifierDiag(Name name)
			: name_(std::move(name)) { }
			
			std::string toString() const {
				return makeString("use of undeclared identifier '%s'",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		class CannotFindMemberVariableDiag: public Error {
		public:
			CannotFindMemberVariableDiag(const String name)
			: name_(name) { }
			
			std::string toString() const {
				return makeString("cannot find member variable '@%s'",
				                  name_.c_str());
			}
			
		private:
			String name_;
			
		};
		
		class EmptyArrayLiteralNotSupportedDiag: public Error {
		public:
			 EmptyArrayLiteralNotSupportedDiag() { }
			
			std::string toString() const {
				return "empty array literals not currently supported";
			}
			
		};
		
		class NonMatchingArrayLiteralTypesDiag: public Error {
		public:
			NonMatchingArrayLiteralTypesDiag(const SEM::Type* const firstType,
			                                 const SEM::Type* const secondType)
			: firstTypeString_(firstType->toDiagString()),
			secondTypeString_(secondType->toDiagString()) { }
			
			std::string toString() const {
				return makeString("array literal element types '%s' and '%s' do not match",
				                  firstTypeString_.c_str(), secondTypeString_.c_str());
			}
			
		private:
			std::string firstTypeString_;
			std::string secondTypeString_;
			
		};
		
		class CannotConstructInterfaceTypeDiag: public Error {
		public:
			CannotConstructInterfaceTypeDiag(const Name& name)
			: name_(name.copy()) { }
			
			std::string toString() const {
				return makeString("cannot construct interface type '%s'",
				                  name_.toString(/*addPrefix=*/false).c_str());
			}
			
		private:
			Name name_;
			
		};
		
		class ConstCastNotImplementedDiag: public Error {
		public:
			ConstCastNotImplementedDiag() { }
			
			std::string toString() const {
				return "const_cast not yet implemented.";
			}
			
		};
		
		class DynamicCastNotImplementedDiag: public Error {
		public:
			DynamicCastNotImplementedDiag() { }
			
			std::string toString() const {
				return "dynamic_cast not yet implemented.";
			}
			
		};
		
		class ReinterpretCastOnlySupportsPointersDiag: public Error {
		public:
			ReinterpretCastOnlySupportsPointersDiag(const SEM::Type* sourceType,
			                                        const SEM::Type* destType)
			: sourceTypeString_(sourceType->toDiagString()),
			destTypeString_(destType->toDiagString()) { }
			
			std::string toString() const {
				return makeString("reinterpret_cast only supports pointers, in cast "
				                  "from type '%s' to type '%s'",
				                  sourceTypeString_.c_str(), destTypeString_.c_str());
			}
			
		private:
			std::string sourceTypeString_;
			std::string destTypeString_;
			
		};
		
		class CannotApplyLvalToLvalDiag: public Error {
		public:
			CannotApplyLvalToLvalDiag() { }
			
			std::string toString() const {
				return "cannot create lval of value that is already a lval";
			}
			
		};
		
		class CannotApplyLvalToRefDiag: public Error {
		public:
			CannotApplyLvalToRefDiag() { }
			
			std::string toString() const {
				return "cannot create lval of value that is already a ref";
			}
			
		};
		
		class CannotApplyNoLvalToNonLvalDiag: public Error {
		public:
			CannotApplyNoLvalToNonLvalDiag() { }
			
			std::string toString() const {
				return "cannot use 'nolval' operator on non-lval value";
			}
			
		};
		
		class CannotApplyRefToRefDiag: public Error {
		public:
			CannotApplyRefToRefDiag() { }
			
			std::string toString() const {
				return "cannot create ref of value that is already a ref";
			}
			
		};
		
		class CannotApplyRefToLvalDiag: public Error {
		public:
			CannotApplyRefToLvalDiag() { }
			
			std::string toString() const {
				return "cannot create ref of value that is already a lval";
			}
			
		};
		
		class CannotApplyNoRefToNonRefDiag: public Error {
		public:
			CannotApplyNoRefToNonRefDiag() { }
			
			std::string toString() const {
				return "cannot use 'noref' operator on non-ref value";
			}
			
		};
		
		class CannotCallInternalConstructorInNonMethodDiag: public Error {
		public:
			CannotCallInternalConstructorInNonMethodDiag() { }
			
			std::string toString() const {
				return "cannot call internal constructor in non-method";
			}
			
		};
		
		class InternalConstructorIncorrectTemplateArgCountDiag: public Error {
		public:
			InternalConstructorIncorrectTemplateArgCountDiag(size_t argsGiven, size_t argsRequired)
			: argsGiven_(argsGiven), argsRequired_(argsRequired) { }
			
			std::string toString() const {
				return makeString("internal constructor given %zu "
				                  "template parameter(s); expected %zu",
				                  argsGiven_, argsRequired_);
			}
			
		private:
			size_t argsGiven_;
			size_t argsRequired_;
			
		};
		
		class InternalConstructorIncorrectArgCountDiag: public Error {
		public:
			InternalConstructorIncorrectArgCountDiag(size_t argsGiven, size_t argsRequired)
			: argsGiven_(argsGiven), argsRequired_(argsRequired) { }
			
			std::string toString() const {
				return makeString("internal constructor called with %zu "
				                  "parameter(s); expected %zu",
				                  argsGiven_, argsRequired_);
			}
			
		private:
			size_t argsGiven_;
			size_t argsRequired_;
			
		};
		
		class SetFakeDiagnosticReceiver {
		public:
			SetFakeDiagnosticReceiver(Context& context)
			: context_(context), previousDiagnosticReceiver_(context_.diagnosticReceiver()) {
				context_.setDiagnosticReceiver(diagnosticArray_);
			}
			
			~SetFakeDiagnosticReceiver() {
				context_.setDiagnosticReceiver(previousDiagnosticReceiver_);
			}
			
			bool anyErrors() const {
				return diagnosticArray_.anyErrors();
			}
			
			void forwardDiags() {
				for (auto& diagPair: diagnosticArray_.diags()) {
					previousDiagnosticReceiver_.issueDiag(std::move(diagPair.diag),
					                                      diagPair.location);
				}
			}
			
		private:
			Context& context_;
			DiagnosticReceiver& previousDiagnosticReceiver_;
			DiagnosticArray diagnosticArray_;
			
		};
		
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
					const auto templateVarMap = GenerateSymbolTemplateVarMap(context, astSymbolNode);
					
					if (searchResult.isNone()) {
						context.issueDiag(UseOfUndeclaredIdentifierDiag(name.copy()),
						                  location);
						return SEM::Value::Constant(Constant::Integer(0), context.typeBuilder().getIntType());
					} else if (searchResult.isFunction()) {
						auto& function = searchResult.function();
						
						auto functionTemplateArguments = GetTemplateValues(templateVarMap, function.templateVariables());
						auto& typeBuilder = context.typeBuilder();
						const auto functionType = typeBuilder.getFunctionPointerType(function.type().substitute(templateVarMap));
						
						if (function.isMethod()) {
							assert(function.isStaticMethod());
							
							const auto typeSearchResult = performSearch(context, name.getPrefix());
							assert(typeSearchResult.isTypeInstance());
							
							auto& typeInstance = typeSearchResult.typeInstance();
							
							auto parentTemplateArguments = GetTemplateValues(templateVarMap, typeInstance.templateVariables());
							const auto parentType = SEM::Type::Object(&typeInstance, std::move(parentTemplateArguments));
							
							return SEM::Value::FunctionRef(parentType, &function, std::move(functionTemplateArguments), functionType);
						} else {
							return SEM::Value::FunctionRef(nullptr, &function, std::move(functionTemplateArguments), functionType);
						}
					} else if (searchResult.isTypeInstance()) {
						auto& typeInstance = searchResult.typeInstance();
						
						if (typeInstance.isInterface()) {
							context.issueDiag(CannotConstructInterfaceTypeDiag(name),
							                  location);
						}
						
						const auto typenameType = context.typeBuilder().getTypenameType();
						const auto parentType = SEM::Type::Object(&typeInstance, GetTemplateValues(templateVarMap, typeInstance.templateVariables()));
						return SEM::Value::TypeRef(parentType, typenameType->createStaticRefType(parentType));
					} else if (searchResult.isAlias()) {
						auto& alias = searchResult.alias();
						(void) context.aliasTypeResolver().resolveAliasType(alias);
						auto templateArguments = GetTemplateValues(templateVarMap, alias.templateVariables());
						return alias.selfRefValue(std::move(templateArguments));
					} else if (searchResult.isVar()) {
						// Variables must just be a single plain string,
						// and be a relative name (so no precending '::').
						assert(astSymbolNode->size() == 1);
						assert(astSymbolNode->isRelative());
						assert(astSymbolNode->first()->templateArguments()->empty());
						auto& var = searchResult.var();
						var.setUsed();
						return SEM::Value::LocalVar(var, getBuiltInType(context, context.getCString("ref_t"), { var.type() })->createRefType(var.type()));
					} else if (searchResult.isTemplateVar()) {
						assert(templateVarMap.empty() && "Template vars cannot have template arguments.");
						auto& templateVar = searchResult.templateVar();
						return templateVar.selfRefValue();
					}
					
					std::terminate();
				}
				case AST::Value::TYPEREF: {
					const auto type = TypeResolver(context).resolveType(astValueNode->typeRef.type);
					const auto typenameType = context.typeBuilder().getTypenameType();
					return SEM::Value::TypeRef(type, typenameType->createStaticRefType(type));
				}
				case AST::Value::MEMBERREF: {
					const auto& memberName = astValueNode->memberRef.name;
					auto selfValue = getSelfValue(context, location);
					
					const auto derefType = getDerefType(selfValue.type());
					assert(derefType->isObject());
					
					const auto typeInstance = derefType->getObjectType();
					const auto variableIterator = typeInstance->namedVariables().find(memberName);
					
					if (variableIterator == typeInstance->namedVariables().end()) {
						context.issueDiag(CannotFindMemberVariableDiag(memberName),
						                  location);
						return SEM::Value::Constant(Constant::Integer(0), context.typeBuilder().getIntType());
					}
					
					return createMemberVarRef(context, std::move(selfValue), *(variableIterator->second));
				}
				case AST::Value::ALIGNOF: {
					const auto type = TypeResolver(context).resolveType(astValueNode->alignOf.type);
					const auto typenameType = context.typeBuilder().getTypenameType();
					auto typeRefValue = SEM::Value::TypeRef(type, typenameType->createStaticRefType(type));
					
					auto alignMaskMethod = GetStaticMethod(context, std::move(typeRefValue), context.getCString("__alignmask"), location);
					auto alignMaskValue = CallValue(context, std::move(alignMaskMethod), {}, location);
					
					const auto sizeType = context.typeBuilder().getSizeType();
					
					auto alignMaskValueAddMethod = GetMethod(context, std::move(alignMaskValue), context.getCString("add"), location);
					auto oneValue = SEM::Value::Constant(Constant::Integer(1), sizeType);
					return CallValue(context, std::move(alignMaskValueAddMethod), makeHeapArray( std::move(oneValue) ), location);
				}
				case AST::Value::SIZEOF: {
					const auto type = TypeResolver(context).resolveType(astValueNode->sizeOf.type);
					const auto typenameType = context.typeBuilder().getTypenameType();
					auto typeRefValue = SEM::Value::TypeRef(type, typenameType->createStaticRefType(type));
					
					auto sizeOfMethod = GetStaticMethod(context, std::move(typeRefValue), context.getCString("__sizeof"), location);
					return CallValue(context, std::move(sizeOfMethod), {}, location);
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
							return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
						}
						case AST::OP_SUBTRACT: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("subtract"), location);
							return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
						}
						case AST::OP_MULTIPLY: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("multiply"), location);
							return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
						}
						case AST::OP_DIVIDE: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("divide"), location);
							return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
						}
						case AST::OP_MODULO: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("modulo"), location);
							return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
						}
						case AST::OP_INDEX: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("index"), location);
							return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
						}
						case AST::OP_ISEQUAL: {
							auto objectValue = tryDissolveValue(context, derefValue(std::move(leftOperand)), location);
							if (HasBinaryOp(context, objectValue, binaryOp, location)) {
								auto opMethod = GetBinaryOp(context, std::move(objectValue), binaryOp, location);
								return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
							} else {
								// Fall back on 'compare' method.
								auto compareMethod = GetMethod(context, std::move(objectValue), context.getCString("compare"), location);
								auto compareResult = CallValue(context, std::move(compareMethod), makeHeapArray( std::move(rightOperand) ), location);
								auto isEqualMethod = GetMethod(context, std::move(compareResult), context.getCString("isEqual"), location);
								return CallValue(context, std::move(isEqualMethod), {}, location);
							}
						}
						case AST::OP_NOTEQUAL: {
							auto objectValue = tryDissolveValue(context, derefValue(std::move(leftOperand)), location);
							if (HasBinaryOp(context, objectValue, binaryOp, location)) {
								auto opMethod = GetBinaryOp(context, std::move(objectValue), binaryOp, location);
								return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
							} else {
								// Fall back on 'compare' method.
								auto compareMethod = GetMethod(context, std::move(objectValue), context.getCString("compare"), location);
								auto compareResult = CallValue(context, std::move(compareMethod), makeHeapArray( std::move(rightOperand) ), location);
								auto isNotEqualMethod = GetMethod(context, std::move(compareResult), context.getCString("isNotEqual"), location);
								return CallValue(context, std::move(isNotEqualMethod), {}, location);
							}
						}
						case AST::OP_LESSTHAN: {
							auto objectValue = tryDissolveValue(context, derefValue(std::move(leftOperand)), location);
							if (HasBinaryOp(context, objectValue, binaryOp, location)) {
								auto opMethod = GetBinaryOp(context, std::move(objectValue), binaryOp, location);
								return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
							} else {
								// Fall back on 'compare' method.
								auto compareMethod = GetMethod(context, std::move(objectValue), context.getCString("compare"), location);
								auto compareResult = CallValue(context, std::move(compareMethod), makeHeapArray( std::move(rightOperand) ), location);
								auto isNotEqualMethod = GetMethod(context, std::move(compareResult), context.getCString("isLessThan"), location);
								return CallValue(context, std::move(isNotEqualMethod), {}, location);
							}
						}
						case AST::OP_LESSTHANOREQUAL: {
							auto objectValue = tryDissolveValue(context, derefValue(std::move(leftOperand)), location);
							if (HasBinaryOp(context, objectValue, binaryOp, location)) {
								auto opMethod = GetBinaryOp(context, std::move(objectValue), binaryOp, location);
								return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
							} else {
								// Fall back on 'compare' method.
								auto compareMethod = GetMethod(context, std::move(objectValue), context.getCString("compare"), location);
								auto compareResult = CallValue(context, std::move(compareMethod), makeHeapArray( std::move(rightOperand) ), location);
								auto isNotEqualMethod = GetMethod(context, std::move(compareResult), context.getCString("isLessThanOrEqual"), location);
								return CallValue(context, std::move(isNotEqualMethod), {}, location);
							}
						}
						case AST::OP_GREATERTHAN: {
							auto objectValue = tryDissolveValue(context, derefValue(std::move(leftOperand)), location);
							if (HasBinaryOp(context, objectValue, binaryOp, location)) {
								auto opMethod = GetBinaryOp(context, std::move(objectValue), binaryOp, location);
								return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
							} else {
								// Fall back on 'compare' method.
								auto compareMethod = GetMethod(context, std::move(objectValue), context.getCString("compare"), location);
								auto compareResult = CallValue(context, std::move(compareMethod), makeHeapArray( std::move(rightOperand) ), location);
								auto isNotEqualMethod = GetMethod(context, std::move(compareResult), context.getCString("isGreaterThan"), location);
								return CallValue(context, std::move(isNotEqualMethod), {}, location);
							}
						}
						case AST::OP_GREATERTHANOREQUAL: {
							auto objectValue = tryDissolveValue(context, derefValue(std::move(leftOperand)), location);
							if (HasBinaryOp(context, objectValue, binaryOp, location)) {
								auto opMethod = GetBinaryOp(context, std::move(objectValue), binaryOp, location);
								return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
							} else {
								// Fall back on 'compare' method.
								auto compareMethod = GetMethod(context, std::move(objectValue), context.getCString("compare"), location);
								auto compareResult = CallValue(context, std::move(compareMethod), makeHeapArray( std::move(rightOperand) ), location);
								auto isNotEqualMethod = GetMethod(context, std::move(compareResult), context.getCString("isGreaterThanOrEqual"), location);
								return CallValue(context, std::move(isNotEqualMethod), {}, location);
							}
						}
						case AST::OP_LOGICALAND: {
							const auto boolType = context.typeBuilder().getBoolType();
							auto boolValue = ImplicitCast(context, std::move(leftOperand), boolType, location);
							
							// Logical AND only evaluates the right operand if the left
							// operand is TRUE, otherwise it returns FALSE.
							return SEM::Value::Ternary(std::move(boolValue), std::move(rightOperand), SEM::Value::Constant(Constant::False(), boolType));
						}
						case AST::OP_LOGICALOR: {
							const auto boolType = context.typeBuilder().getBoolType();
							auto boolValue = ImplicitCast(context, std::move(leftOperand), boolType, location);
							
							// Logical OR only evaluates the right operand if the left
							// operand is FALSE, otherwise it returns TRUE.
							return SEM::Value::Ternary(std::move(boolValue), SEM::Value::Constant(Constant::True(), boolType), std::move(rightOperand));
						}
						case AST::OP_BITWISEAND: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("bitwise_and"), location);
							return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
						}
						case AST::OP_BITWISEOR: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("bitwise_or"), location);
							return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
						}
						case AST::OP_BITWISEXOR: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("bitwise_xor"), location);
							return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
						}
						case AST::OP_LEFTSHIFT: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("left_shift"), location);
							return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
						}
						case AST::OP_RIGHTSHIFT: {
							auto opMethod = GetMethod(context, std::move(leftOperand), context.getCString("right_shift"), location);
							return CallValue(context, std::move(opMethod), makeHeapArray( std::move(rightOperand) ), location);
						}
					}
					
					std::terminate();
				}
				case AST::Value::TERNARY: {
					auto cond = ConvertValue(context, astValueNode->ternary.condition);
					
					const auto boolType = context.typeBuilder().getBoolType();
					auto boolValue = ImplicitCast(context, std::move(cond), boolType, location);
					
					auto ifTrue = ConvertValue(context, astValueNode->ternary.ifTrue);
					auto ifFalse = ConvertValue(context, astValueNode->ternary.ifFalse);
					
					const auto targetType = UnifyTypes(context, ifTrue.type(), ifFalse.type(), location);
					
					auto castIfTrue = ImplicitCast(context, std::move(ifTrue), targetType, location);
					auto castIfFalse = ImplicitCast(context, std::move(ifFalse), targetType, location);
					
					return SEM::Value::Ternary(std::move(boolValue), std::move(castIfTrue), std::move(castIfFalse));
				}
				case AST::Value::CAST: {
					auto sourceValue = ConvertValue(context, astValueNode->cast.value);
					const auto sourceType = TypeResolver(context).resolveType(astValueNode->cast.sourceType);
					const auto targetType = TypeResolver(context).resolveType(astValueNode->cast.targetType);
					
					switch(astValueNode->cast.castKind) {
						case AST::Value::CAST_CONST:
							context.issueDiag(ConstCastNotImplementedDiag(), location);
							throw SkipException();
						case AST::Value::CAST_DYNAMIC:
							context.issueDiag(DynamicCastNotImplementedDiag(), location);
							throw SkipException();
						case AST::Value::CAST_REINTERPRET:
							if (!sourceType->isPrimitive() || sourceType->getObjectType()->name().last() != "ptr_t"
								|| !targetType->isPrimitive() || targetType->getObjectType()->name().last() != "ptr_t") {
								context.issueDiag(ReinterpretCastOnlySupportsPointersDiag(sourceType, targetType),
								                  location);
								throw SkipException();
							}
							return SEM::Value::Reinterpret(ImplicitCast(context, std::move(sourceValue), sourceType, location), targetType);
					}
					
					std::terminate();
				}
				case AST::Value::LVAL: {
					auto sourceValue = ConvertValue(context, astValueNode->makeLval.value);
					
					if (sourceValue.type()->isLval()) {
						context.issueDiag(CannotApplyLvalToLvalDiag(), location);
						return sourceValue;
					}
					
					if (sourceValue.type()->isRef()) {
						context.issueDiag(CannotApplyLvalToRefDiag(), location);
						return sourceValue;
					}
					
					const auto targetType = TypeResolver(context).resolveType(astValueNode->makeLval.targetType);
					return SEM::Value::Lval(targetType, std::move(sourceValue));
				}
				case AST::Value::NOLVAL: {
					auto sourceValue = ConvertValue(context, astValueNode->makeNoLval.value);
					
					if (!getDerefType(sourceValue.type())->isLval()) {
						context.issueDiag(CannotApplyNoLvalToNonLvalDiag(), location);
						return sourceValue;
					}
					
					return SEM::Value::NoLval(std::move(sourceValue));
				}
				case AST::Value::REF: {
					auto sourceValue = ConvertValue(context, astValueNode->makeRef.value);
					
					if (sourceValue.type()->isLval()) {
						context.issueDiag(CannotApplyRefToLvalDiag(), location);
						return sourceValue;
					}
					
					if (sourceValue.type()->isRef()) {
						context.issueDiag(CannotApplyRefToRefDiag(), location);
						return sourceValue;
					}
					
					const auto targetType = TypeResolver(context).resolveType(astValueNode->makeRef.targetType);
					return SEM::Value::Ref(targetType, std::move(sourceValue));
				}
				case AST::Value::NOREF: {
					auto sourceValue = ConvertValue(context, astValueNode->makeNoRef.value);
					
					if (!sourceValue.type()->isRef()) {
						context.issueDiag(CannotApplyNoRefToNonRefDiag(), location);
						return sourceValue;
					}
					
					return SEM::Value::NoRef(std::move(sourceValue));
				}
				case AST::Value::INTERNALCONSTRUCT: {
					const auto& astTemplateArgs = astValueNode->internalConstruct.templateArgs;
					const auto& astParameterValueNodes = astValueNode->internalConstruct.parameters;
					
					const auto thisTypeInstance = lookupParentType(context.scopeStack());
					
					if (thisTypeInstance == nullptr) {
						context.issueDiag(CannotCallInternalConstructorInNonMethodDiag(), location);
						return SEM::Value::Constant(Constant::Integer(0), context.typeBuilder().getIntType());
					}
					
					SEM::ValueArray templateArgs;
					templateArgs.reserve(thisTypeInstance->templateVariables().size());
					for (const auto& astTemplateArg: *astTemplateArgs) {
						templateArgs.push_back(ConvertValue(context, astTemplateArg));
					}
					
					bool useSelfTemplateArgs = astTemplateArgs->empty();
					
					if (!useSelfTemplateArgs && templateArgs.size() != thisTypeInstance->templateVariables().size()) {
						const size_t argsGiven = templateArgs.size();
						const size_t argsExpected = thisTypeInstance->templateVariables().size();
						context.issueDiag(InternalConstructorIncorrectTemplateArgCountDiag(argsGiven,
						                                                                   argsExpected),
						                  location);
						useSelfTemplateArgs = true;
					}
					
					if (useSelfTemplateArgs) {
						templateArgs.clear();
						for (const auto& templateVar: thisTypeInstance->templateVariables()) {
							templateArgs.push_back(templateVar->selfRefValue());
						}
					} else {
						assert(templateArgs.size() == thisTypeInstance->templateVariables().size());
					}
					
					const auto templateVarMap = GenerateTemplateVarMap(context, *thisTypeInstance, std::move(templateArgs), location);
					const auto thisType = SEM::Type::Object(thisTypeInstance, GetTemplateValues(templateVarMap, thisTypeInstance->templateVariables()));
					
					if (astParameterValueNodes->size() != thisTypeInstance->variables().size()) {
						const size_t argsGiven = astParameterValueNodes->size();
						const size_t argsExpected = thisTypeInstance->variables().size();
						context.issueDiag(InternalConstructorIncorrectArgCountDiag(argsGiven,
						                                                           argsExpected),
						                  location);
					}
					
					HeapArray<SEM::Value> semValues;
					semValues.reserve(astParameterValueNodes->size());
					
					for (size_t i = 0; i < astParameterValueNodes->size(); i++) {
						auto semValue = ConvertValue(context, astParameterValueNodes->at(i));
						if (i < thisTypeInstance->variables().size()) {
							const auto semVar = thisTypeInstance->variables().at(i);
							auto semParam = ImplicitCast(context, std::move(semValue),
							                             semVar->constructType()->substitute(templateVarMap),
							                             location);
							semValues.push_back(std::move(semParam));
						} else {
							semValues.push_back(std::move(semValue));
						}
					}
					
					return SEM::Value::InternalConstruct(thisType, std::move(semValues));
				}
				case AST::Value::MEMBERACCESS: {
					const auto& memberName = astValueNode->memberAccess.memberName;
					
					auto object = ConvertValue(context, astValueNode->memberAccess.object);
					return MakeMemberAccess(context, std::move(object), memberName, astValueNode.location());
				}
				case AST::Value::TEMPLATEDMEMBERACCESS: {
					const auto& memberName = astValueNode->templatedMemberAccess.memberName;
					auto object = ConvertValue(context, astValueNode->templatedMemberAccess.object);
					
					SEM::ValueArray templateArguments;
					templateArguments.reserve(astValueNode->templatedMemberAccess.templateArgs->size());
					
					for (const auto& arg: *(astValueNode->templatedMemberAccess.templateArgs)) {
						templateArguments.push_back(ConvertValue(context, arg));
					}
					
					return GetTemplatedMethod(context, std::move(object), memberName, std::move(templateArguments), astValueNode.location());
				}
				case AST::Value::FUNCTIONCALL: {
					auto functionValue = ConvertValue(context, astValueNode->functionCall.functionValue);
					
					HeapArray<SEM::Value> argumentValues;
					
					for (const auto& astArgumentValueNode: *(astValueNode->functionCall.parameters)) {
						argumentValues.push_back(ConvertValue(context, astArgumentValueNode));
					}
					
					return CallValue(context, std::move(functionValue), std::move(argumentValues), location);
				}
				case AST::Value::CAPABILITYTEST: {
					const auto checkType = TypeResolver(context).resolveType(astValueNode->capabilityTest.checkType);
					const auto capabilityType = TypeResolver(context).resolveType(astValueNode->capabilityTest.capabilityType);
					const auto boolType = context.typeBuilder().getBoolType();
					return SEM::Value::CapabilityTest(checkType,
					                                  capabilityType,
					                                  boolType);
				}
				case AST::Value::ARRAYLITERAL: {
					SEM::ValueArray elementValues;
					for (const auto& astElementValueNode: *(astValueNode->arrayLiteral.values)) {
						elementValues.push_back(ConvertValue(context, astElementValueNode));
					}
					
					if (elementValues.empty()) {
						context.issueDiag(EmptyArrayLiteralNotSupportedDiag(),
						                  location);
					}
					
					auto& typeBuilder = context.typeBuilder();
					const auto elementType = elementValues.empty() ?
						typeBuilder.getIntType() : elementValues[0].type();
					
					for (const auto& elementValue: elementValues) {
						if (elementType != elementValue.type()) {
							context.issueDiag(NonMatchingArrayLiteralTypesDiag(elementType,
							                                                   elementValue.type()),
							                  location);
						}
					}
					
					const auto arrayType = typeBuilder.getConstantStaticArrayType(elementType,
					                                                              elementValues.size(),
					                                                              location);
					
					return SEM::Value::ArrayLiteral(arrayType,
					                                std::move(elementValues));
				}
				case AST::Value::MERGE: {
					// The parser wasn't able to resolve an ambiguity,
					// so it merged two values.
					const auto& first = astValueNode->merge.first;
					const auto& second = astValueNode->merge.second;
					
					const auto isFirstType = (first->kind() == AST::Value::TYPEREF);
					const auto isSecondType = (second->kind() == AST::Value::TYPEREF);
					(void) isSecondType;
					
					// We're expecting to see an ambiguity
					// between types and values where an
					// indexing operation (i.e. A[N]) could
					// be either an array type or indexing
					// inside an array.
					assert(isFirstType || isSecondType);
					assert(!(isFirstType && isSecondType));
					
					const auto& typeValue = isFirstType ? first : second;
					const auto& nonTypeValue = isFirstType ? second : first;
					
					try {
						// Try to convert the type.
						SetFakeDiagnosticReceiver setFakeDiagnosticReceiver(context);
						auto convertedValue = ConvertValue(context, typeValue);
						if (!setFakeDiagnosticReceiver.anyErrors()) {
							setFakeDiagnosticReceiver.forwardDiags();
							return convertedValue;
						}
					} catch (const Exception&) {
						// It failed, so try to convert the value.
					}
					
					return ConvertValue(context, nonTypeValue);
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


