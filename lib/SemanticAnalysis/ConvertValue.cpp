#include <assert.h>
#include <stdio.h>

#include <limits>
#include <list>
#include <map>
#include <stdexcept>
#include <string>

#include <locic/AST.hpp>
#include <locic/Debug.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/CanCast.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertType.hpp>
#include <locic/SemanticAnalysis/ConvertValue.hpp>
#include <locic/SemanticAnalysis/Lval.hpp>
#include <locic/SemanticAnalysis/NameSearch.hpp>
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/TypeProperties.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		SEM::Type* getFunctionType(SEM::Type* type) {
			if (type->isInterfaceMethod()) {
				return type->getInterfaceMethodFunctionType();
			}
			
			if (type->isMethod()) {
				return type->getMethodFunctionType();
			}
			
			return type;
		}
		
		bool CanValueThrow(SEM::Value* value) {
			// TODO...
			if (value->kind() == SEM::Value::FUNCTIONCALL) {
				const auto functionValue = value->functionCall.functionValue;
				const auto functionType = getFunctionType(functionValue->type());
				assert(functionType->isFunction());
				return !functionType->isFunctionNoExcept();
			} else {
				return false;
			}
		}
		
		std::string binaryOpToString(AST::BinaryOpKind kind) {
			switch (kind) {
				case AST::OP_ISEQUAL:
					return "==";
				case AST::OP_NOTEQUAL:
					return "!=";
				default:
					throw std::runtime_error("Unknown binary op.");
			}
		}
		
		std::string binaryOpName(AST::BinaryOpKind kind) {
			switch (kind) {
				case AST::OP_ISEQUAL:
					return "equal";
				case AST::OP_NOTEQUAL:
					return "not_equal";
				default:
					throw std::runtime_error("Unknown binary op.");
			}
		}
		
		SEM::Value* GetBinaryOp(Context& context, SEM::Value* value, AST::BinaryOpKind opKind, const Debug::SourceLocation& location) {
			const auto derefType = getDerefType(value->type());
			
			if (!derefType->isObjectOrTemplateVar()) {
				throw ErrorException(makeString("Can't perform binary operator '%s' for non-object type '%s' at position %s.",
					binaryOpToString(opKind).c_str(), derefType->toString().c_str(), location.toString().c_str()));
			}
			
			const auto typeInstance = derefType->getObjectOrSpecType();
			
			const auto methodName = binaryOpName(opKind);
			
			// Look for the 
			if (typeInstance->functions().find(CanonicalizeMethodName(methodName)) != typeInstance->functions().end()) {
				return GetMethod(context, value, methodName, location);
			} else {
				return nullptr;
			}
		}
		
		SEM::Value* MakeMemberAccess(Context& context, SEM::Value* value, const std::string& memberName, const Debug::SourceLocation& location) {
			const auto derefType = getDerefType(value->type());
			
			if (!derefType->isObjectOrTemplateVar()) {
				throw ErrorException(makeString("Can't access member '%s' of type '%s' at position %s.",
					memberName.c_str(), derefType->toString().c_str(), location.toString().c_str()));
			}
			
			const auto typeInstance = derefType->getObjectOrSpecType();
			
			// Look for methods.
			if (typeInstance->functions().find(CanonicalizeMethodName(memberName)) != typeInstance->functions().end()) {
				return GetMethod(context, value, memberName, location);
			}
			
			// TODO: this should be replaced by falling back on 'property' methods.
			// Look for variables.
			if (typeInstance->isDatatype() || typeInstance->isException() || typeInstance->isStruct()) {
				const auto variableIterator = typeInstance->namedVariables().find(memberName);
				if (variableIterator != typeInstance->namedVariables().end()) {
					return createMemberVarRef(context, value, variableIterator->second);
				}
			}
			
			throw ErrorException(makeString("Can't access member '%s' in type '%s' at position %s.",
				memberName.c_str(), typeInstance->name().toString().c_str(), location.toString().c_str()));
		}
		
		std::string integerSpecifierType(const std::string& specifier) {
			if (specifier == "i8") {
				return "int8_t";
			} else if (specifier == "i16") {
				return "int16_t";
			} else if (specifier == "i32") {
				return "int32_t";
			} else if (specifier == "i64") {
				return "int64_t";
			} else if (specifier == "u8") {
				return "uint8_t";
			} else if (specifier == "u16") {
				return "uint16_t";
			} else if (specifier == "u32") {
				return "uint32_t";
			} else if (specifier == "u64") {
				return "uint64_t";
			}
			
			throw ErrorException(makeString("Invalid integer literal specifier '%s'.", specifier.c_str()));
		}
		
		unsigned long long integerMax(const std::string& typeName) {
			if (typeName == "int8_t") {
				return std::numeric_limits<int8_t>::max();
			} else if (typeName == "int16_t") {
				return std::numeric_limits<int16_t>::max();
			} else if (typeName == "int32_t") {
				return std::numeric_limits<int32_t>::max();
			} else if (typeName == "int64_t") {
				return std::numeric_limits<int64_t>::max();
			} else if (typeName == "uint8_t") {
				return std::numeric_limits<uint8_t>::max();
			} else if (typeName == "uint16_t") {
				return std::numeric_limits<uint16_t>::max();
			} else if (typeName == "uint32_t") {
				return std::numeric_limits<uint32_t>::max();
			} else if (typeName == "uint64_t") {
				return std::numeric_limits<uint64_t>::max();
			}
			
			throw std::runtime_error(makeString("Invalid integer type '%s'.", typeName.c_str()));
		}
		
		std::string getIntegerConstantType(const std::string& specifier, const Constant& constant) {
			assert(constant.kind() == Constant::INTEGER);
			
			const auto integerValue = constant.integerValue();
			
			// Use a specifier if available.
			if (!specifier.empty() && specifier != "u") {
				const auto typeName = integerSpecifierType(specifier);
				const auto typeMax = integerMax(typeName);
				if (integerValue > typeMax) {
					throw ErrorException(makeString("Integer literal '%llu' exceeds maximum of specifier '%s'.",
						(unsigned long long) integerValue, specifier.c_str()));
				}
				return typeName;
			}
			
			// Otherwise determine type based on value.
			std::vector<std::string> types;
			types.push_back("int8_t");
			types.push_back("int16_t");
			types.push_back("int32_t");
			types.push_back("int64_t");
			
			for (const auto& typeName: types) {
				const auto specTypeName = specifier + typeName;
				// TODO: use arbitary-precision arithmetic.
				if (integerValue <= integerMax(specTypeName)) {
					return specTypeName;
				}
			}
			
			throw ErrorException(makeString("Integer literal '%llu' is too large to be represented in a fixed width type.",
				integerValue));
		}
		
		std::string getFloatingPointConstantType(const std::string& specifier, const Constant& constant) {
			assert(constant.kind() == Constant::FLOATINGPOINT);
			(void) constant;
			
			if (specifier == "f") {
				return "float_t";
			} else if (specifier.empty() || specifier == "d") {
				return "double_t";
			} else {
				throw ErrorException(makeString("Invalid floating point literal specifier '%s'.",
					specifier.c_str()));
			}
		}
		
		std::string getLiteralTypeName(const std::string& specifier, const Constant& constant) {
			switch (constant.kind()) {
				case Constant::NULLVAL: {
					if (specifier.empty()) {
						return "null_t";
					} else {
						throw ErrorException(makeString("Invalid null literal specifier '%s'.",
							specifier.c_str()));
					}
				}
				case Constant::BOOLEAN: {
					if (specifier.empty()) {
						return "bool";
					} else {
						throw ErrorException(makeString("Invalid boolean literal specifier '%s'.",
							specifier.c_str()));
					}
				}
				case Constant::INTEGER: {
					return getIntegerConstantType(specifier, constant);
				}
				case Constant::FLOATINGPOINT: {
					return getFloatingPointConstantType(specifier, constant);
				}
				default:
					throw std::runtime_error("Unknown constant kind.");
			}
		}
		
		SEM::Type* getLiteralType(Context& context, const std::string& specifier, const Constant& constant) {
			switch (constant.kind()) {
				case Constant::STRING: {
					// C strings have the type 'const byte * const', as opposed to just a
					// type name, so their type needs to be generated specially.
					const auto byteTypeInstance = getBuiltInType(context.scopeStack(), "byte_t");
					const auto ptrTypeInstance = getBuiltInType(context.scopeStack(), "__ptr");
					
					// Generate type 'const byte'.
					const auto constByteType = byteTypeInstance->selfType()->createConstType();
					
					// Generate type 'const ptr<const byte>'.
					return SEM::Type::Object(ptrTypeInstance, { constByteType })->createConstType();
				}
				default: {
					const auto typeName = getLiteralTypeName(specifier, constant);
					const auto typeInstance = getBuiltInType(context.scopeStack(), typeName);
					if (typeInstance == nullptr) {
						throw ErrorException(makeString("Couldn't find constant type '%s' when generating value constant.",
							typeName.c_str()));
					}
					
					return SEM::Type::Object(typeInstance, SEM::Type::NO_TEMPLATE_ARGS)->createConstType();
				}
			}
		}
		
		SEM::Value* getLiteralValue(Context& context, const std::string& specifier, Constant& constant, const Debug::SourceLocation& location) {
			const auto constantValue = SEM::Value::Constant(&constant, getLiteralType(context, specifier, constant));
			
			if (constant.kind() != Constant::STRING || specifier == "C") {
				return constantValue;
			}
			
			const auto functionName = std::string("string_literal") + (!specifier.empty() ? std::string("_") + specifier : std::string(""));
			
			const auto searchResult = performSearch(context, Name::Absolute() + functionName);
			if (!searchResult.isFunction()) {
				throw ErrorException(makeString("Invalid string literal specifier '%s' at %s; failed to find relevant function '%s'.",
					specifier.c_str(), location.toString().c_str(), functionName.c_str()));
			}
			
			const auto functionRef = SEM::Value::FunctionRef(nullptr, searchResult.function(), {});
			return CallValue(context, functionRef, { constantValue }, location);
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
		
		SEM::Value* ConvertValueData(Context& context, const AST::Node<AST::Value>& astValueNode) {
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
					const auto thisTypeInstance = lookupParentType(context.scopeStack());
					
					if (thisTypeInstance == nullptr) {
						throw ErrorException(makeString("Cannot access 'this' in non-method at %s.",
							location.toString().c_str()));
					}
					
					// TODO: make const type when in const methods.
					const auto selfType = thisTypeInstance->selfType();
					const auto ptrTypeInstance = getBuiltInType(context.scopeStack(), "__ptr");
					return SEM::Value::This(SEM::Type::Object(ptrTypeInstance, { selfType })->createConstType());
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
						
						if (function->isMethod()) {
							if (!function->isStaticMethod()) {
								throw ErrorException(makeString("Cannot refer directly to non-static class method '%s' at %s.",
									name.toString().c_str(), location.toString().c_str()));
							}
							
							const auto typeSearchResult = performSearch(context, name.getPrefix());
							assert(typeSearchResult.isTypeInstance());
							
							const auto typeInstance = typeSearchResult.typeInstance();
							
							const auto parentType = SEM::Type::Object(typeInstance, GetTemplateValues(context, astSymbolNode));
							
							return SEM::Value::FunctionRef(parentType, function, templateVarMap);
						} else {
							return SEM::Value::FunctionRef(nullptr, function, templateVarMap);
						}
					} else if (searchResult.isTypeInstance()) {
						const auto typeInstance = searchResult.typeInstance();
						
						if (typeInstance->isInterface()) {
							throw ErrorException(makeString("Can't construct interface type '%s' at %s.",
								name.toString().c_str(), location.toString().c_str()));
						}
						
						const auto parentType = SEM::Type::Object(typeInstance, GetTemplateValues(context, astSymbolNode));
						return GetStaticMethod(parentType, "create", location);
					} else if (searchResult.isVar()) {
						// Variables must just be a single plain string,
						// and be a relative name (so no precending '::').
						// TODO: make these throw exceptions.
						assert(astSymbolNode->size() == 1);
						assert(astSymbolNode->isRelative());
						assert(astSymbolNode->first()->templateArguments()->empty());
						const auto referenceTypeInst = getBuiltInType(context.scopeStack(), "__ref");
						const auto var = searchResult.var();
						return SEM::Value::LocalVar(var, SEM::Type::Object(referenceTypeInst, { var->type() })->createRefType(var->type()));
					} else if (searchResult.isTemplateVar()) {
						assert(templateVarMap.empty() && "Template vars cannot have template arguments.");
						const auto templateVar = searchResult.templateVar();
						return GetStaticMethod(SEM::Type::TemplateVarRef(templateVar), "create", location);
					} else {
						throw std::runtime_error("Unknown search result for name reference.");
					}
					
					throw std::runtime_error("Invalid if-statement fallthrough in ConvertValue for name reference.");
				}
				case AST::Value::MEMBERREF: {
					const auto& memberName = astValueNode->memberRef.name;
					const auto selfValue = getSelfValue(context, location);
					
					const auto derefType = getDerefType(selfValue->type());
					assert(derefType->isObject());
					
					const auto typeInstance = derefType->getObjectType();
					const auto variableIterator = typeInstance->namedVariables().find(memberName);
					
					if (variableIterator == typeInstance->namedVariables().end()) {
						throw ErrorException(makeString("Member variable '@%s' not found at position %s.",
							memberName.c_str(), location.toString().c_str()));
					}
					
					return createMemberVarRef(context, selfValue, variableIterator->second);
				}
				case AST::Value::SIZEOF: {
					return SEM::Value::SizeOf(ConvertType(context, astValueNode->sizeOf.type), getBuiltInType(context.scopeStack(), "size_t")->selfType());
				}
				case AST::Value::BINARYOP: {
					const auto binaryOp = astValueNode->binaryOp.kind;
					const auto leftOperand = ConvertValue(context, astValueNode->binaryOp.leftOperand);
					const auto rightOperand = ConvertValue(context, astValueNode->binaryOp.rightOperand);
					
					const auto objectValue = tryDissolveValue(context, leftOperand, location);
					
					switch (binaryOp) {
						case AST::OP_ISEQUAL: {
							const auto opMethod = GetBinaryOp(context, objectValue, binaryOp, location);
							if (opMethod != nullptr) {
								return CallValue(context, opMethod, { rightOperand }, location);
							} else {
								// Fall back on 'compare' method.
								const auto compareMethod = GetMethod(context, objectValue, "compare", location);
								const auto compareResult = CallValue(context, compareMethod, { rightOperand }, location);
								const auto isEqualMethod = GetMethod(context, compareResult, "isEqual", location);
								return CallValue(context, isEqualMethod, {}, location);
							}
						}
						case AST::OP_NOTEQUAL: {
							const auto opMethod = GetBinaryOp(context, objectValue, binaryOp, location);
							if (opMethod != nullptr) {
								return CallValue(context, opMethod, { rightOperand }, location);
							} else {
								// Fall back on 'compare' method.
								const auto compareMethod = GetMethod(context, objectValue, "compare", location);
								const auto compareResult = CallValue(context, compareMethod, { rightOperand }, location);
								const auto isNotEqualMethod = GetMethod(context, compareResult, "isNotEqual", location);
								return CallValue(context, isNotEqualMethod, {}, location);
							}
						}
						default:
							throw std::runtime_error("Unknown binary op kind.");
					}
				}
				case AST::Value::TERNARY: {
					const auto cond = ConvertValue(context, astValueNode->ternary.condition);
					
					const auto boolType = getBuiltInType(context.scopeStack(), "bool");
					const auto boolValue = ImplicitCast(context, cond, boolType->selfType(), location);
					
					const auto ifTrue = ConvertValue(context, astValueNode->ternary.ifTrue);
					const auto ifFalse = ConvertValue(context, astValueNode->ternary.ifFalse);
					
					const auto targetType = UnifyTypes(context, ifTrue->type(), ifFalse->type(), location);
					
					const auto castIfTrue = ImplicitCast(context, ifTrue, targetType, location);
					const auto castIfFalse = ImplicitCast(context, ifFalse, targetType, location);
					
					return SEM::Value::Ternary(boolValue, castIfTrue, castIfFalse);
				}
				case AST::Value::CAST: {
					const auto sourceValue = ConvertValue(context, astValueNode->cast.value);
					const auto sourceType = ConvertType(context, astValueNode->cast.sourceType);
					const auto targetType = ConvertType(context, astValueNode->cast.targetType);
					
					switch(astValueNode->cast.castKind) {
						case AST::Value::CAST_STATIC: {
							throw ErrorException("static_cast not yet implemented.");
						}
						case AST::Value::CAST_CONST:
							throw ErrorException("const_cast not yet implemented.");
						case AST::Value::CAST_DYNAMIC:
							throw ErrorException("dynamic_cast not yet implemented.");
						case AST::Value::CAST_REINTERPRET:
							if (!sourceType->isPrimitive() || sourceType->getObjectType()->name().last() != "__ptr"
								|| !targetType->isPrimitive() || targetType->getObjectType()->name().last() != "__ptr") {
								throw ErrorException(makeString("reinterpret_cast currently only supports ptr<T>, "
									"in cast from value %s of type %s to type %s at position %s.",
									sourceValue->toString().c_str(), sourceType->toString().c_str(),
									targetType->toString().c_str(), location.toString().c_str()));
							}
							return SEM::Value::Reinterpret(ImplicitCast(context, sourceValue, sourceType, location), targetType);
						default:
							throw std::runtime_error("Unknown cast kind.");
					}
				}
				case AST::Value::LVAL: {
					const auto sourceValue = ConvertValue(context, astValueNode->makeLval.value);
					
					if (sourceValue->type()->isLval()) {
						throw ErrorException(makeString("Can't create lval of value that is already a lval, for value '%s' at position %s.",
							sourceValue->toString().c_str(), location.toString().c_str()));
					}
					
					if (sourceValue->type()->isRef()) {
						throw ErrorException(makeString("Can't create value that is both an lval and a ref, for value '%s' at position %s.",
							sourceValue->toString().c_str(), location.toString().c_str()));
					}
					
					const auto targetType = ConvertType(context, astValueNode->makeLval.targetType);
					return SEM::Value::Lval(targetType, sourceValue);
				}
				case AST::Value::NOLVAL: {
					const auto sourceValue = ConvertValue(context, astValueNode->makeNoLval.value);
					
					if (!sourceValue->type()->isLval()) {
						throw ErrorException(makeString("Can't use 'nolval' operator on non-lval value '%s' at position '%s'.",
							sourceValue->toString().c_str(), location.toString().c_str()));
					}
					
					return SEM::Value::NoLval(sourceValue);
				}
				case AST::Value::REF: {
					const auto sourceValue = ConvertValue(context, astValueNode->makeRef.value);
					
					if (sourceValue->type()->isLval()) {
						throw ErrorException(makeString("Can't create value that is both an lval and a ref, for value '%s' at position %s.",
							sourceValue->toString().c_str(), location.toString().c_str()));
					}
					
					if (sourceValue->type()->isRef()) {
						throw ErrorException(makeString("Can't create ref of value that is already a ref, for value '%s' at position %s.",
							sourceValue->toString().c_str(), location.toString().c_str()));
					}
					
					const auto targetType = ConvertType(context, astValueNode->makeRef.targetType);
					return SEM::Value::Ref(targetType, sourceValue);
				}
				case AST::Value::NOREF: {
					const auto sourceValue = ConvertValue(context, astValueNode->makeNoRef.value);
					
					if (!sourceValue->type()->isRef()) {
						throw ErrorException(makeString("Can't use 'noref' operator on non-ref value '%s' at position '%s'.",
							sourceValue->toString().c_str(), location.toString().c_str()));
					}
					
					return SEM::Value::NoRef(sourceValue);
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
					
					std::vector<SEM::Value*> semValues;
					
					for(size_t i = 0; i < thisTypeInstance->variables().size(); i++){
						const auto semVar = thisTypeInstance->variables().at(i);
						const auto semValue = ConvertValue(context, astParameterValueNodes->at(i));
						const auto semParam = ImplicitCast(context, semValue, semVar->constructType(), location);
						semValues.push_back(semParam);
					}
					
					return SEM::Value::InternalConstruct(thisTypeInstance, semValues);
				}
				case AST::Value::MEMBERACCESS: {
					const auto& memberName = astValueNode->memberAccess.memberName;
					
					auto object = ConvertValue(context, astValueNode->memberAccess.object);
					
					if (memberName != "address" && memberName != "assign" && memberName != "dissolve" && memberName != "move") {
						object = tryDissolveValue(context, object, location);
					}
					
					return MakeMemberAccess(context, object, memberName, astValueNode.location());
				}
				case AST::Value::FUNCTIONCALL: {
					const auto functionValue = ConvertValue(context, astValueNode->functionCall.functionValue);
					
					std::vector<SEM::Value*> argumentValues;
					
					for (const auto& astArgumentValueNode: *(astValueNode->functionCall.parameters)) {
						argumentValues.push_back(ConvertValue(context, astArgumentValueNode));
					}
					
					return CallValue(context, functionValue, argumentValues, location);
				}
				default:
					throw std::runtime_error("Unknown AST::Value kind.");
			}
		}
		
		Debug::ValueInfo makeValueInfo(const AST::Node<AST::Value>& astValueNode) {
			Debug::ValueInfo valueInfo;
			valueInfo.location = astValueNode.location();
			return valueInfo;
		}
		
		SEM::Value* ConvertValue(Context& context, const AST::Node<AST::Value>& astValueNode) {
			const auto semValue = ConvertValueData(context, astValueNode);
			context.debugModule().valueMap.insert(std::make_pair(semValue, makeValueInfo(astValueNode)));
			return semValue;
		}
		
	}
	
}


