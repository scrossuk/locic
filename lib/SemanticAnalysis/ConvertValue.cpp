#include <assert.h>
#include <stdio.h>

#include <limits>
#include <list>
#include <map>
#include <stdexcept>
#include <string>

#include <locic/AST.hpp>
#include <locic/AST/MethodSet.hpp>
#include <locic/AST/Type.hpp>

#include <locic/Debug.hpp>
#include <locic/Support/MakeArray.hpp>

#include <locic/Frontend/DiagnosticArray.hpp>

#include <locic/SemanticAnalysis/AliasTypeResolver.hpp>
#include <locic/SemanticAnalysis/CallValue.hpp>
#include <locic/SemanticAnalysis/Cast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/GetMethod.hpp>
#include <locic/SemanticAnalysis/GetMethodSet.hpp>
#include <locic/SemanticAnalysis/Literal.hpp>
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
			
			locic_unreachable("Unknown op kind.");
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
			
			locic_unreachable("Unknown op kind.");
		}
		
		String binaryOpName(Context& context, const AST::BinaryOpKind kind) {
			const char* const cString = binaryOpNameCString(kind);
			return context.getCString(cString);
		}
		
		bool HasBinaryOp(Context& context, const AST::Value& value, AST::BinaryOpKind opKind,
		                 const Debug::SourceLocation& /*location*/) {
			const auto derefType = getDerefType(value.type());
			assert(derefType->isObjectOrTemplateVar());
			
			const auto methodSet = getTypeMethodSet(context, derefType);
			const auto methodName = binaryOpName(context, opKind);
			return methodSet->hasMethod(CanonicalizeMethodName(methodName));
		}
		
		AST::Value GetBinaryOp(Context& context, AST::Value value, const AST::BinaryOpKind opKind, const Debug::SourceLocation& location) {
			return GetMethod(context, std::move(value), binaryOpName(context, opKind), location);
		}
		
		class CannotFindMemberInTypeDiag: public ErrorDiag {
		public:
			CannotFindMemberInTypeDiag(String name, const AST::Type* type)
			: name_(name), typeString_(type->toDiagString()) { }
			
			std::string toString() const {
				return makeString("cannot find member '%s' in type '%s'",
				                  name_.c_str(), typeString_.c_str());
			}
			
		private:
			String name_;
			std::string typeString_;
			
		};
		
		AST::Value MakeMemberAccess(Context& context, AST::Value rawValue, const String& memberName, const Debug::SourceLocation& location) {
			auto value = derefValue(std::move(rawValue));
			const auto refDerefType = getDerefType(value.type()->resolveAliases());
			const auto derefType = refDerefType->isTypename() ?
				refDerefType->typenameTarget() : refDerefType;
			assert(derefType->isObjectOrTemplateVar());
			
			const auto methodSet = getTypeMethodSet(context, derefType);
			
			// Look for methods.
			if (methodSet->hasMethod(CanonicalizeMethodName(memberName))) {
				if (refDerefType->isTypename()) {
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
						return createMemberVarRef(context, std::move(value), *(variableIterator->second),
						                          /*isInternalAccess=*/false);
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
		
		class VarCannotHaveTemplateArgumentsDiag: public ErrorDiag {
		public:
			VarCannotHaveTemplateArgumentsDiag(const String name)
			: name_(name) { }
			
			std::string toString() const {
				return makeString("variable '%s' cannot have template arguments",
				                  name_.c_str());
			}
			
		private:
			String name_;
			
		};
		
		class UseOfUndeclaredIdentifierDiag: public ErrorDiag {
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
		
		class CannotFindMemberVariableDiag: public ErrorDiag {
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
		
		class EmptyArrayLiteralNotSupportedDiag: public ErrorDiag {
		public:
			 EmptyArrayLiteralNotSupportedDiag() { }
			
			std::string toString() const {
				return "empty array literals not currently supported";
			}
			
		};
		
		class NonMatchingArrayLiteralTypesDiag: public ErrorDiag {
		public:
			NonMatchingArrayLiteralTypesDiag(const AST::Type* const firstType,
			                                 const AST::Type* const secondType)
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
		
		class CannotConstructInterfaceTypeDiag: public ErrorDiag {
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
		
		class ConstCastNotImplementedDiag: public ErrorDiag {
		public:
			ConstCastNotImplementedDiag() { }
			
			std::string toString() const {
				return "const_cast not yet implemented.";
			}
			
		};
		
		class DynamicCastNotImplementedDiag: public ErrorDiag {
		public:
			DynamicCastNotImplementedDiag() { }
			
			std::string toString() const {
				return "dynamic_cast not yet implemented.";
			}
			
		};
		
		class ReinterpretCastOnlySupportsPointersDiag: public ErrorDiag {
		public:
			ReinterpretCastOnlySupportsPointersDiag(const AST::Type* sourceType,
			                                        const AST::Type* destType)
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
		
		class CannotApplyRefToRefDiag: public ErrorDiag {
		public:
			CannotApplyRefToRefDiag() { }
			
			std::string toString() const {
				return "cannot create ref of value that is already a ref";
			}
			
		};
		
		class CannotApplyNoRefToNonRefDiag: public ErrorDiag {
		public:
			CannotApplyNoRefToNonRefDiag() { }
			
			std::string toString() const {
				return "cannot use 'noref' operator on non-ref value";
			}
			
		};
		
		class CannotCallInternalConstructorInNonMethodDiag: public ErrorDiag {
		public:
			CannotCallInternalConstructorInNonMethodDiag() { }
			
			std::string toString() const {
				return "cannot call internal constructor in non-method";
			}
			
		};
		
		class InternalConstructorIncorrectTemplateArgCountDiag: public ErrorDiag {
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
		
		class InternalConstructorIncorrectArgCountDiag: public ErrorDiag {
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
		
		class InvalidOperandCapabilityCheckDiag: public ErrorDiag {
		public:
			InvalidOperandCapabilityCheckDiag() { }
			
			std::string toString() const {
				return "invalid operand for capability check";
			}
			
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
		
		class SelfConstInNonMethodDiag: public ErrorDiag {
		public:
			SelfConstInNonMethodDiag() { }
			
			std::string toString() const {
				return "cannot use 'selfconst' in non-method function";
			}
			
		};
		
		class SelfConstInStaticMethodDiag: public ErrorDiag {
		public:
			SelfConstInStaticMethodDiag() { }
			
			std::string toString() const {
				return "cannot use 'selfconst' in static method";
			}
			
		};
		
		AST::Value ConvertValueData(Context& context, const AST::Node<AST::ValueDecl>& astValueNode) {
			assert(astValueNode.get() != nullptr);
			const auto& location = astValueNode.location();
			
			switch (astValueNode->typeEnum) {
				case AST::ValueDecl::BRACKET: {
					return ConvertValue(context, astValueNode->bracket.value);
				}
				case AST::ValueDecl::SELF: {
					return getSelfValue(context, location);
				}
				case AST::ValueDecl::THIS: {
					return getThisValue(context, location);
				}
				case AST::ValueDecl::SELFCONST: {
					const auto thisTypeInstance = lookupParentType(context.scopeStack());
					const auto thisFunction = lookupParentFunction(context.scopeStack());
					
					if (thisTypeInstance == nullptr) {
						context.issueDiag(SelfConstInNonMethodDiag(), location);
					}
					
					if (thisFunction->isStaticMethod()) {
						context.issueDiag(SelfConstInStaticMethodDiag(), location);
					}
					
					const auto boolType = context.typeBuilder().getBoolType();
					return AST::Value::PredicateExpr(AST::Predicate::SelfConst(),
					                                 boolType);
				}
				case AST::ValueDecl::LITERAL: {
					const auto& specifier = astValueNode->literal.specifier;
					auto& constant = *(astValueNode->literal.constant);
					return getLiteralValue(context, specifier, constant, location);
				}
				case AST::ValueDecl::SYMBOLREF: {
					const auto& astSymbolNode = astValueNode->symbolRef.symbol;
					const Name name = astSymbolNode->createName();
					
					const auto searchResult = performSymbolLookup(context, name);
					
					// Get a map from template variables to their values (i.e. types).
					const auto templateVarMap = GenerateSymbolTemplateVarMap(context, astSymbolNode);
					
					if (searchResult.isNone()) {
						context.issueDiag(UseOfUndeclaredIdentifierDiag(name.copy()),
						                  location);
						return AST::Value::Constant(Constant::Integer(0), context.typeBuilder().getIntType());
					} else if (searchResult.isFunction()) {
						auto& function = searchResult.function();
						
						auto functionTemplateArguments = GetTemplateValues(templateVarMap, function.templateVariables());
						auto& typeBuilder = context.typeBuilder();
						const auto functionType = typeBuilder.getFunctionPointerType(
							function.type().substitute(templateVarMap,
							                           /*selfconst=*/AST::Predicate::SelfConst()));
						
						if (function.isMethod()) {
							assert(function.isStaticMethod());
							
							const auto typeSearchResult = performSearch(context, name.getPrefix());
							assert(typeSearchResult.isTypeInstance());
							
							auto& typeInstance = typeSearchResult.typeInstance();
							
							auto parentTemplateArguments = GetTemplateValues(templateVarMap, typeInstance.templateVariables());
							const auto parentType = AST::Type::Object(&typeInstance, std::move(parentTemplateArguments));
							
							return AST::Value::FunctionRef(parentType, function, std::move(functionTemplateArguments), functionType);
						} else {
							return AST::Value::FunctionRef(nullptr, function, std::move(functionTemplateArguments), functionType);
						}
					} else if (searchResult.isTypeInstance()) {
						auto& typeInstance = searchResult.typeInstance();
						const auto parentType = AST::Type::Object(&typeInstance, GetTemplateValues(templateVarMap, typeInstance.templateVariables()));
						
						if (typeInstance.isInterface()) {
							const auto abstractTypenameType = context.typeBuilder().getAbstractTypenameType();
							return AST::Value::TypeRef(parentType, abstractTypenameType);
						} else {
							const auto typenameType = context.typeBuilder().getTypenameType(parentType);
							return AST::Value::TypeRef(parentType, typenameType);
						}
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
						
						auto& var = searchResult.var();
						var.setUsed();
						
						if (!astSymbolNode->first()->templateArguments()->empty()) {
							context.issueDiag(VarCannotHaveTemplateArgumentsDiag(var.name()),
							                  astSymbolNode.location());
						}
						
						return AST::Value::LocalVar(var, getBuiltInType(context, context.getCString("ref_t"), { var.type() }));
					} else if (searchResult.isTemplateVar()) {
						assert(templateVarMap.empty() && "Template vars cannot have template arguments.");
						auto& templateVar = searchResult.templateVar();
						return templateVar.selfRefValue();
					}
					
					locic_unreachable("Unknown search result kind.");
				}
				case AST::ValueDecl::TYPEREF: {
					const auto type = TypeResolver(context).resolveType(astValueNode->typeRef.type);
					const auto typenameType = context.typeBuilder().getTypenameType(type);
					return AST::Value::TypeRef(type, typenameType);
				}
				case AST::ValueDecl::MEMBERREF: {
					const auto& memberName = astValueNode->memberRef.name;
					auto selfValue = getSelfValue(context, location);
					
					const auto derefType = getDerefType(selfValue.type());
					assert(derefType->isObject());
					
					const auto typeInstance = derefType->getObjectType();
					const auto variableIterator = typeInstance->namedVariables().find(memberName);
					
					if (variableIterator == typeInstance->namedVariables().end()) {
						context.issueDiag(CannotFindMemberVariableDiag(memberName),
						                  location);
						return AST::Value::Constant(Constant::Integer(0), context.typeBuilder().getIntType());
					}
					
					return createMemberVarRef(context, std::move(selfValue), *(variableIterator->second),
					                          /*isInternalAccess=*/true);
				}
				case AST::ValueDecl::ALIGNOF: {
					const auto type = TypeResolver(context).resolveType(astValueNode->alignOf.type);
					const auto typenameType = context.typeBuilder().getTypenameType(type);
					auto typeRefValue = AST::Value::TypeRef(type, typenameType);
					
					auto alignMaskMethod = GetStaticMethod(context, std::move(typeRefValue), context.getCString("__alignmask"), location);
					auto alignMaskValue = CallValue(context, std::move(alignMaskMethod), {}, location);
					
					const auto sizeType = context.typeBuilder().getSizeType();
					
					auto alignMaskValueAddMethod = GetMethod(context, std::move(alignMaskValue), context.getCString("add"), location);
					auto oneValue = AST::Value::Constant(Constant::Integer(1), sizeType);
					return CallValue(context, std::move(alignMaskValueAddMethod), makeHeapArray( std::move(oneValue) ), location);
				}
				case AST::ValueDecl::SIZEOF: {
					const auto type = TypeResolver(context).resolveType(astValueNode->sizeOf.type);
					const auto typenameType = context.typeBuilder().getTypenameType(type);
					auto typeRefValue = AST::Value::TypeRef(type, typenameType);
					
					auto sizeOfMethod = GetStaticMethod(context, std::move(typeRefValue), context.getCString("__sizeof"), location);
					return CallValue(context, std::move(sizeOfMethod), {}, location);
				}
				case AST::ValueDecl::NEW: {
					auto placementArg = ConvertValue(context, astValueNode->newValue.placementArg);
					auto operand = ConvertValue(context, astValueNode->newValue.operand);
					const auto operandType = getDerefType(operand.type());
					operand = ImplicitCast(context, std::move(operand), operandType, location);
					
					const auto operandPtrType = context.typeBuilder().getPointerType(operandType);
					placementArg = ImplicitCast(context, std::move(placementArg),
					                            operandPtrType, location);
					
					return AST::Value::New(std::move(placementArg), std::move(operand),
					                       TypeBuilder(context).getVoidType());
				}
				case AST::ValueDecl::UNARYOP: {
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
							// We want to get a reference to the reference, so that address()
							// is called on the reference.
							const auto targetRefCount = 2;
							operand = derefOrBindValue(context, std::move(operand), targetRefCount);
							auto opMethod = GetSpecialMethod(context, std::move(operand), context.getCString("address"), location);
							return CallValue(context, std::move(opMethod), {}, location);
						}
						case AST::OP_MOVE: {
							auto opMethod = GetMethod(context, std::move(operand), context.getCString("__move"), location);
							return CallValue(context, std::move(opMethod), {}, location);
						}
					}
					
					locic_unreachable("Unknown op kind.");
				}
				case AST::ValueDecl::BINARYOP: {
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
							auto objectValue = derefValue(std::move(leftOperand));
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
							auto objectValue = derefValue(std::move(leftOperand));
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
							auto objectValue = derefValue(std::move(leftOperand));
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
							auto objectValue = derefValue(std::move(leftOperand));
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
							auto objectValue = derefValue(std::move(leftOperand));
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
							auto objectValue = derefValue(std::move(leftOperand));
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
							rightOperand = ImplicitCast(context, std::move(rightOperand), boolType, location);
							
							// Logical AND only evaluates the right operand if the left
							// operand is TRUE, otherwise it returns FALSE.
							return AST::Value::Ternary(std::move(boolValue), std::move(rightOperand), AST::Value::Constant(Constant::False(), boolType));
						}
						case AST::OP_LOGICALOR: {
							const auto boolType = context.typeBuilder().getBoolType();
							auto boolValue = ImplicitCast(context, std::move(leftOperand), boolType, location);
							rightOperand = ImplicitCast(context, std::move(rightOperand), boolType, location);
							
							// Logical OR only evaluates the right operand if the left
							// operand is FALSE, otherwise it returns TRUE.
							return AST::Value::Ternary(std::move(boolValue), AST::Value::Constant(Constant::True(), boolType), std::move(rightOperand));
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
					
					locic_unreachable("Unknown op kind.");
				}
				case AST::ValueDecl::TERNARY: {
					auto cond = ConvertValue(context, astValueNode->ternary.condition);
					
					const auto boolType = context.typeBuilder().getBoolType();
					auto boolValue = ImplicitCast(context, std::move(cond), boolType, location);
					
					auto ifTrue = ConvertValue(context, astValueNode->ternary.ifTrue);
					auto ifFalse = ConvertValue(context, astValueNode->ternary.ifFalse);
					
					const auto targetType = UnifyTypes(context, ifTrue.type(), ifFalse.type(), location);
					
					auto castIfTrue = ImplicitCast(context, std::move(ifTrue), targetType, location);
					auto castIfFalse = ImplicitCast(context, std::move(ifFalse), targetType, location);
					
					return AST::Value::Ternary(std::move(boolValue), std::move(castIfTrue), std::move(castIfFalse));
				}
				case AST::ValueDecl::CAST: {
					auto sourceValue = ConvertValue(context, astValueNode->cast.value);
					const auto sourceType = TypeResolver(context).resolveType(astValueNode->cast.sourceType);
					const auto targetType = TypeResolver(context).resolveType(astValueNode->cast.targetType);
					
					switch(astValueNode->cast.castKind) {
						case AST::ValueDecl::CAST_CONST:
							context.issueDiag(ConstCastNotImplementedDiag(), location);
							throw SkipException();
						case AST::ValueDecl::CAST_DYNAMIC:
							context.issueDiag(DynamicCastNotImplementedDiag(), location);
							throw SkipException();
						case AST::ValueDecl::CAST_REINTERPRET:
							if (!sourceType->isPrimitive() || sourceType->getObjectType()->fullName().last() != "ptr_t"
								|| !targetType->isPrimitive() || targetType->getObjectType()->fullName().last() != "ptr_t") {
								context.issueDiag(ReinterpretCastOnlySupportsPointersDiag(sourceType, targetType),
								                  location);
								throw SkipException();
							}
							return AST::Value::Reinterpret(ImplicitCast(context, std::move(sourceValue), sourceType, location), targetType);
					}
					
					locic_unreachable("Unknown cast kind.");
				}
				case AST::ValueDecl::INTERNALCONSTRUCT: {
					const auto& astTemplateArgs = astValueNode->internalConstruct.templateArgs;
					const auto& astParameterValueNodes = astValueNode->internalConstruct.parameters;
					
					const auto thisTypeInstance = lookupParentType(context.scopeStack());
					
					if (thisTypeInstance == nullptr) {
						context.issueDiag(CannotCallInternalConstructorInNonMethodDiag(), location);
						return AST::Value::Constant(Constant::Integer(0), context.typeBuilder().getIntType());
					}
					
					AST::ValueArray templateArgs;
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
					const auto thisType = AST::Type::Object(thisTypeInstance, GetTemplateValues(templateVarMap, thisTypeInstance->templateVariables()));
					
					if (astParameterValueNodes->size() != thisTypeInstance->variables().size()) {
						const size_t argsGiven = astParameterValueNodes->size();
						const size_t argsExpected = thisTypeInstance->variables().size();
						context.issueDiag(InternalConstructorIncorrectArgCountDiag(argsGiven,
						                                                           argsExpected),
						                  location);
					}
					
					HeapArray<AST::Value> astValues;
					astValues.reserve(astParameterValueNodes->size());
					
					for (size_t i = 0; i < astParameterValueNodes->size(); i++) {
						auto astValue = ConvertValue(context, astParameterValueNodes->at(i));
						if (i < thisTypeInstance->variables().size()) {
							const auto astVar = thisTypeInstance->variables().at(i);
							const auto varType = astVar->type()->substitute(templateVarMap,
							                                                /*selfconst=*/AST::Predicate::SelfConst());
							auto astParam = ImplicitCast(context, std::move(astValue),
							                             varType, location);
							astValues.push_back(std::move(astParam));
						} else {
							astValues.push_back(std::move(astValue));
						}
					}
					
					return AST::Value::InternalConstruct(thisType, std::move(astValues));
				}
				case AST::ValueDecl::MEMBERACCESS: {
					const auto& memberName = astValueNode->memberAccess.memberName;
					
					auto object = ConvertValue(context, astValueNode->memberAccess.object);
					return MakeMemberAccess(context, std::move(object), memberName, astValueNode.location());
				}
				case AST::ValueDecl::TEMPLATEDMEMBERACCESS: {
					const auto& memberName = astValueNode->templatedMemberAccess.memberName;
					auto object = ConvertValue(context, astValueNode->templatedMemberAccess.object);
					
					AST::ValueArray templateArguments;
					templateArguments.reserve(astValueNode->templatedMemberAccess.templateArgs->size());
					
					for (const auto& arg: *(astValueNode->templatedMemberAccess.templateArgs)) {
						templateArguments.push_back(ConvertValue(context, arg));
					}
					
					return GetTemplatedMethod(context, std::move(object), memberName, std::move(templateArguments), astValueNode.location());
				}
				case AST::ValueDecl::FUNCTIONCALL: {
					auto functionValue = ConvertValue(context, astValueNode->functionCall.functionValue);
					
					HeapArray<AST::Value> argumentValues;
					
					for (const auto& astArgumentValueNode: *(astValueNode->functionCall.parameters)) {
						argumentValues.push_back(ConvertValue(context, astArgumentValueNode));
					}
					
					return CallValue(context, std::move(functionValue), std::move(argumentValues), location);
				}
				case AST::ValueDecl::CAPABILITYTEST: {
					const auto checkType = TypeResolver(context).resolveType(astValueNode->capabilityTest.checkType);
					if (checkType->isAlias() && !checkType->alias().type()->isTypename()) {
						context.issueDiag(InvalidOperandCapabilityCheckDiag(),
						                  astValueNode->capabilityTest.checkType.location());
					}
					
					const auto capabilityType = TypeResolver(context).resolveType(astValueNode->capabilityTest.capabilityType);
					if (capabilityType->isAlias() && !capabilityType->alias().type()->isTypename()) {
						context.issueDiag(InvalidOperandCapabilityCheckDiag(),
						                  astValueNode->capabilityTest.capabilityType.location());
					}
					
					const auto boolType = context.typeBuilder().getBoolType();
					return AST::Value::CapabilityTest(checkType,
					                                  capabilityType,
					                                  boolType);
				}
				case AST::ValueDecl::ARRAYLITERAL: {
					AST::ValueArray elementValues;
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
					
					return AST::Value::ArrayLiteral(arrayType,
					                                std::move(elementValues));
				}
				case AST::ValueDecl::MERGE: {
					// The parser wasn't able to resolve an ambiguity,
					// so it merged two values.
					const auto& first = astValueNode->merge.first;
					const auto& second = astValueNode->merge.second;
					
					const auto isFirstType = (first->kind() == AST::ValueDecl::TYPEREF);
					const auto isSecondType = (second->kind() == AST::ValueDecl::TYPEREF);
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
			
			locic_unreachable("Unknown value kind.");
		}
		
		Debug::ValueInfo makeValueInfo(const AST::Node<AST::ValueDecl>& astValueNode) {
			Debug::ValueInfo valueInfo;
			valueInfo.location = astValueNode.location();
			return valueInfo;
		}
		
		AST::Value ConvertValue(Context& context, const AST::Node<AST::ValueDecl>& astValueNode) {
			auto astValue = ConvertValueData(context, astValueNode);
			astValue.setDebugInfo(makeValueInfo(astValueNode));
			return astValue;
		}
		
	}
	
}


