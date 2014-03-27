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
#include <locic/SemanticAnalysis/Ref.hpp>
#include <locic/SemanticAnalysis/VarArgCast.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		SEM::Value* MakeMemberAccess(Context& context, SEM::Value* accessObject, const std::string& memberName, const Debug::SourceLocation& location) {
			const auto object = derefValue(accessObject);
			const auto objectType = getDerefType(accessObject->type());
			
			if (!objectType->isObject() && !objectType->isTemplateVar()) {
				throw ErrorException(makeString("Can't access member of non-object value '%s' of type '%s'.",
					object->toString().c_str(), objectType->toString().c_str()));
			}
			
			const auto typeInstance =
				objectType->isTemplateVar() ?
					objectType->getTemplateVar()->specTypeInstance() :
					objectType->getObjectType();
			assert(typeInstance != nullptr);
			
			const Node typeNode = context.reverseLookup(typeInstance);
			assert(typeNode.isNotNone());
			
			// Look for methods.
			const Node childNode = typeNode.getChild(memberName);
			
			if (childNode.isFunction()) {
				const auto function = childNode.getSEMFunction();
				
				assert(function->isMethod());
				
				if (function->isStaticMethod()) {
					throw ErrorException(makeString("Cannot call static function '%s' in type '%s' at location %s.",
						function->name().toString().c_str(), typeInstance->name().toString().c_str(),
						location.toString().c_str()));
				}
				
				if (objectType->isConst() && !function->isConstMethod()) {
					throw ErrorException(makeString("Cannot refer to mutator method '%s' from const object '%s' of type '%s' at location %s.",
						function->name().toString().c_str(), accessObject->toString().c_str(),
						objectType->toString().c_str(), location.toString().c_str()));
				}
				
				const auto functionRef = SEM::Value::FunctionRef(objectType, function, objectType->generateTemplateVarMap());
				
				if (typeInstance->isInterface()) {
					return SEM::Value::InterfaceMethodObject(functionRef, object);
				} else {
					return SEM::Value::MethodObject(functionRef, object);
				}
			}
			
			// TODO: this should be replaced by falling back on 'property' methods.
			
			// Look for variables.
			const Node varNode = typeNode.getChild("#__ivar_" + memberName);
			
			if (varNode.isNotNone()) {
				assert(varNode.isVariable());
				const auto var = varNode.getSEMVar();
				auto memberType = var->type();
				
				if (objectType->isConst()) {
					// If the type instance is const, then
					// the members must also be.
					memberType = memberType->createConstType();
				}
				
				return SEM::Value::MemberAccess(object, var, SEM::Type::Reference(memberType)->createRefType(memberType));
			}
			
			throw ErrorException(makeString("Can't access member '%s' in type '%s' at location %s.",
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
					if (specifier.empty()) {
						throw ErrorException("Loci strings are not currently supported; add 'C' specifier to use C strings (e.g. C\"example\").");
					} else if (specifier == "C") {
						// C strings have the type 'const char * const', as opposed to just a
						// type name, so their type needs to be generated specially.
						const auto charTypeInstance = getBuiltInType(context, "char_t");
						const auto ptrTypeInstance = getBuiltInType(context, "ptr");
						
						// Generate type 'const char'.
						const auto constCharType = SEM::Type::Object(charTypeInstance, SEM::Type::NO_TEMPLATE_ARGS)->createConstType();
						
						// Generate type 'const ptr<const char>'.
						return SEM::Type::Object(ptrTypeInstance, { constCharType })->createConstType();
					} else {
						throw ErrorException(makeString("Invalid string literal specifier '%s'.",
							specifier.c_str()));
					}
				}
				default: {
					const auto typeName = getLiteralTypeName(specifier, constant);
					const auto typeInstance = getBuiltInType(context, typeName);
					if (typeInstance == nullptr) {
						throw ErrorException(makeString("Couldn't find constant type '%s' when generating value constant.",
							typeName.c_str()));
					}
					
					return SEM::Type::Object(typeInstance, SEM::Type::NO_TEMPLATE_ARGS)->createConstType();
				}
			}
		}
		
		SEM::Value* ConvertValueData(Context& context, const AST::Node<AST::Value>& astValueNode) {
			assert(astValueNode.get() != nullptr);
			
			switch (astValueNode->typeEnum) {
				case AST::Value::BRACKET: {
					return ConvertValue(context, astValueNode->bracket.value);
				}
				case AST::Value::SELF: {
					const Node thisTypeNode = lookupParentType(context);
					
					assert(thisTypeNode.isNone() || thisTypeNode.isTypeInstance());
					
					if (thisTypeNode.isNone()) {
						throw ErrorException(makeString("Cannot access 'self' in non-method at %s.",
							astValueNode.location().toString().c_str()));
					}
					
					// TODO: make const type when in const methods.
					const auto selfType = thisTypeNode.getSEMTypeInstance()->selfType();
					return SEM::Value::Self(SEM::Type::Reference(selfType)->createRefType(selfType));
				}
				case AST::Value::THIS: {
					const Node thisTypeNode = lookupParentType(context);
					
					assert(thisTypeNode.isNone() || thisTypeNode.isTypeInstance());
					
					if (thisTypeNode.isNone()) {
						throw ErrorException(makeString("Cannot access 'this' in non-method at %s.",
							astValueNode.location().toString().c_str()));
					}
					
					// TODO: make const type when in const methods.
					const auto selfType = thisTypeNode.getSEMTypeInstance()->selfType();
					const auto ptrTypeInstance = getBuiltInType(context, "ptr");
					return SEM::Value::This(SEM::Type::Object(ptrTypeInstance, { selfType })->createConstType());
				}
				case AST::Value::LITERAL: {
					const auto& specifier = astValueNode->literal.specifier;
					auto& constant = *(astValueNode->literal.constant);
					return SEM::Value::Constant(&constant, getLiteralType(context, specifier, constant));
				}
				case AST::Value::SYMBOLREF: {
					const auto& astSymbolNode = astValueNode->symbolRef.symbol;
					const Name name = astSymbolNode->createName();
					
					// Not a local variable => do a symbol lookup.
					const Node node = context.lookupName(name);
					
					// Get a map from template variables to their values (i.e. types).
					const auto templateVarMap = GenerateTemplateVarMap(context, astSymbolNode);
					
					if (node.isNone()) {
						throw ErrorException(makeString("Couldn't find symbol or value '%s' at %s.",
							name.toString().c_str(), astSymbolNode.location().toString().c_str()));
					} else if (node.isNamespace()) {
						throw ErrorException(makeString("Namespace '%s' is not a valid value at %s.",
							name.toString().c_str(), astSymbolNode.location().toString().c_str()));
					} else if (node.isFunction()) {
						const auto function = node.getSEMFunction();
						assert(function != nullptr && "Function pointer must not be NULL (as indicated by isFunction() being true)");
						
						if (function->isMethod()) {
							if (!function->isStaticMethod()) {
								throw ErrorException(makeString("Cannot refer directly to non-static class method '%s' at %s.",
									name.toString().c_str(), astSymbolNode.location().toString().c_str()));
							}
							
							const Node typeNode = context.lookupName(name.getPrefix());
							assert(typeNode.isTypeInstance());
							
							const auto typeInstance = typeNode.getSEMTypeInstance();
							
							const auto parentType = SEM::Type::Object(typeInstance, GetTemplateValues(context, astSymbolNode));
							
							return SEM::Value::FunctionRef(parentType, function, templateVarMap);
						} else {
							return SEM::Value::FunctionRef(nullptr, function, templateVarMap);
						}
					} else if (node.isTypeInstance()) {
						const auto typeInstance = node.getSEMTypeInstance();
						
						if (typeInstance->isInterface()) {
							throw ErrorException(makeString("Can't construct interface type '%s' at %s.",
								name.toString().c_str(), astSymbolNode.location().toString().c_str()));
						}
						
						if (!typeInstance->hasProperty("Create")) {
							throw ErrorException(makeString("Couldn't find default constructor for type '%s' at %s.",
								name.toString().c_str(), astSymbolNode.location().toString().c_str()));
						}
						
						const auto parentType = SEM::Type::Object(typeInstance, GetTemplateValues(context, astSymbolNode));
						
						const auto defaultConstructor = typeInstance->getProperty("Create");
						
						return SEM::Value::FunctionRef(parentType, defaultConstructor, templateVarMap);
					} else if (node.isVariable()) {
						// Variables must just be a single plain string,
						// and be a relative name (so no precending '::').
						// TODO: make these throw exceptions.
						assert(astSymbolNode->size() == 1);
						assert(astSymbolNode->isRelative());
						assert(astSymbolNode->first()->templateArguments()->empty());
						return SEM::Value::LocalVar(node.getSEMVar());
					} else if (node.isTemplateVar()) {
						assert(templateVarMap.empty() && "Template vars cannot have template arguments.");
						const auto templateVar = node.getSEMTemplateVar();
						
						const auto specTypeInstance = templateVar->specTypeInstance();
						if (!specTypeInstance->hasProperty("Create")) {
							throw ErrorException(makeString("Couldn't find default constructor for type '%s' at %s.",
								name.toString().c_str(), astSymbolNode.location().toString().c_str()));
						}
						
						const auto defaultConstructor = specTypeInstance->getProperty("Create");
						
						return SEM::Value::FunctionRef(SEM::Type::TemplateVarRef(templateVar),
							defaultConstructor, Map<SEM::TemplateVar*, SEM::Type*>());
					} else {
						throw std::runtime_error("Unknown node for name reference.");
					}
					
					throw std::runtime_error("Invalid if-statement fallthrough in ConvertValue for name reference.");
				}
				case AST::Value::MEMBERREF: {
					const auto& memberName = astValueNode->memberRef.name;
					const auto semVar = getParentMemberVariable(context, memberName).getSEMVar();
					
					if (semVar == nullptr) {
						throw ErrorException(makeString("Member variable '@%s' not found.",
								memberName.c_str()));
					}
					
					return SEM::Value::MemberVar(semVar);
				}
				case AST::Value::TERNARY: {
					const auto cond = ConvertValue(context, astValueNode->ternary.condition);
					
					const auto boolType = getBuiltInType(context, "bool");
					const auto boolValue = ImplicitCast(context, cond, boolType->selfType());
					
					const auto ifTrue = ConvertValue(context, astValueNode->ternary.ifTrue);
					const auto ifFalse = ConvertValue(context, astValueNode->ternary.ifFalse);
					
					const auto targetType = UnifyTypes(context, ifTrue->type(), ifFalse->type());
					
					const auto castIfTrue = ImplicitCast(context, ifTrue, targetType);
					const auto castIfFalse = ImplicitCast(context, ifFalse, targetType);
					
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
							if (!sourceType->isPrimitive() || sourceType->getObjectType()->name().last() != "ptr"
								|| !targetType->isPrimitive() || targetType->getObjectType()->name().last() != "ptr") {
								throw ErrorException(makeString("reinterpret_cast currently only supports ptr<T>, "
									"in cast from value %s of type %s to type %s.", sourceValue->toString().c_str(),
									sourceType->toString().c_str(), targetType->toString().c_str()));
							}
							return SEM::Value::Reinterpret(ImplicitCast(context, sourceValue, sourceType), targetType);
						default:
							throw std::runtime_error("Unknown cast kind.");
					}
				}
				case AST::Value::LVAL: {
					const auto sourceValue = ConvertValue(context, astValueNode->makeLval.value);
					
					if (sourceValue->type()->isLval()) {
						throw ErrorException(makeString("Can't create lval of value that is already a lval, for value '%s'.",
							sourceValue->toString().c_str()));
					}
					
					if (sourceValue->type()->isRef()) {
						throw ErrorException(makeString("Can't create value that is both an lval and a ref, for value '%s'.",
							sourceValue->toString().c_str()));
					}
					
					const auto targetType = ConvertType(context, astValueNode->makeLval.targetType);
					return SEM::Value::Lval(targetType, sourceValue);
				}
				case AST::Value::REF: {
					const auto sourceValue = ConvertValue(context, astValueNode->makeRef.value);
					
					if (sourceValue->type()->isLval()) {
						throw ErrorException(makeString("Can't create value that is both an lval and a ref, for value '%s'.",
							sourceValue->toString().c_str()));
					}
					
					if (sourceValue->type()->isRef()) {
						throw ErrorException(makeString("Can't create ref of value that is already a ref, for value '%s'.",
							sourceValue->toString().c_str()));
					}
					
					const auto targetType = ConvertType(context, astValueNode->makeRef.targetType);
					return SEM::Value::Ref(targetType, sourceValue);
				}
				case AST::Value::INTERNALCONSTRUCT: {
					const auto& astParameterValueNodes = astValueNode->internalConstruct.parameters;
					
					const Node thisTypeNode = lookupParentType(context);
					
					assert(thisTypeNode.isNone() || thisTypeNode.isTypeInstance());
					
					if (thisTypeNode.isNone()) {
						throw ErrorException(makeString("Cannot call internal constructor in non-method at %s.",
							astValueNode.location().toString().c_str()));
					}
					
					const auto thisTypeInstance = thisTypeNode.getSEMTypeInstance();
					
					if (astParameterValueNodes->size() != thisTypeInstance->variables().size()) {
						throw ErrorException(makeString("Internal constructor called "
							   "with wrong number of arguments; received %llu, expected %llu.",
							(unsigned long long) astParameterValueNodes->size(),
							(unsigned long long) thisTypeInstance->variables().size()));
					}
					
					std::vector<SEM::Value*> semValues;
					
					for(size_t i = 0; i < thisTypeInstance->variables().size(); i++){
						SEM::Type* constructType = thisTypeInstance->constructTypes().at(i);
						SEM::Value* semValue = ConvertValue(context, astParameterValueNodes->at(i));
						SEM::Value* semParam = ImplicitCast(context, semValue, constructType);
						semValues.push_back(semParam);
					}
					
					return SEM::Value::InternalConstruct(thisTypeInstance, semValues);
				}
				case AST::Value::MEMBERACCESS: {
					const auto& memberName = astValueNode->memberAccess.memberName;
					
					SEM::Value* object = ConvertValue(context, astValueNode->memberAccess.object);
					
					if (memberName != "address" && memberName != "assign" && memberName != "dissolve" && memberName != "move") {
						object = tryDissolveValue(object);
					}
					
					return MakeMemberAccess(context, object, memberName, astValueNode.location());
				}
				case AST::Value::FUNCTIONCALL: {
					assert(astValueNode->functionCall.functionValue.get() != NULL && "Cannot call NULL function value");
					const auto functionValue = ConvertValue(context, astValueNode->functionCall.functionValue);
					
					switch (functionValue->type()->kind()) {
						case SEM::Type::FUNCTION: {
							const auto& typeList = functionValue->type()->getFunctionParameterTypes();
							const auto& astValueList = astValueNode->functionCall.parameters;
							
							if (functionValue->type()->isFunctionVarArg()) {
								if(astValueList->size() < typeList.size()) {
									throw ErrorException(makeString("Var Arg Function [%s] called with %llu number of parameters; expected at least %llu.",
										functionValue->toString().c_str(),
										(unsigned long long) astValueList->size(),
										(unsigned long long) typeList.size()));
								}
							} else {
								if(astValueList->size() != typeList.size()) {
									throw ErrorException(makeString("Function [%s] called with %llu number of parameters; expected %llu.",
										functionValue->toString().c_str(),
										(unsigned long long) astValueList->size(),
										(unsigned long long) typeList.size()));
								}
							}
							
							assert(astValueList->size() >= typeList.size());
							
							std::vector<SEM::Value*> semValueList;
							
							for(std::size_t i = 0; i < astValueList->size(); i++) {
								const auto semArgValue = ConvertValue(context, astValueList->at(i));
								
								// Cast arguments to the function type's corresponding
								// argument type; var-arg arguments should be cast to
								// one of the allowed types (since there's no specific
								// destination type).
								const auto param = (i < typeList.size()) ?
										ImplicitCast(context, semArgValue, typeList.at(i)) :
										VarArgCast(semArgValue);
										
								semValueList.push_back(param);
							}
							
							return SEM::Value::FunctionCall(functionValue, semValueList);
						}
						case SEM::Type::METHOD: {
							const auto functionType = functionValue->type()->getMethodFunctionType();
							
							const auto& typeList = functionType->getFunctionParameterTypes();
							const auto& astValueList = astValueNode->functionCall.parameters;
							
							assert(!functionType->isFunctionVarArg() && "Methods cannot be var args");
							
							if (typeList.size() != astValueList->size()) {
								throw ErrorException(makeString("Method [%s] called with %lu number of parameters; expected %lu.",
									functionValue->toString().c_str(),
									astValueList->size(), typeList.size()));
							}
							
							std::vector<SEM::Value*> semValueList;
							
							for (size_t i = 0; i < astValueList->size(); i++) {
								const auto semArgValue = ConvertValue(context, astValueList->at(i));
								const auto param = ImplicitCast(context, semArgValue, typeList.at(i));
								semValueList.push_back(param);
							}
							
							return SEM::Value::MethodCall(functionValue, semValueList);
						}
						case SEM::Type::INTERFACEMETHOD: {
							const auto functionType = functionValue->type()->getInterfaceMethodFunctionType();
							
							const auto& typeList = functionType->getFunctionParameterTypes();
							const auto& astValueList = astValueNode->functionCall.parameters;
							
							assert(!functionType->isFunctionVarArg() && "Methods cannot be var args");
							
							if (typeList.size() != astValueList->size()) {
								throw ErrorException(makeString("Method [%s] called with %lu number of parameters; expected %lu.",
									functionValue->toString().c_str(),
									astValueList->size(), typeList.size()));
							}
							
							std::vector<SEM::Value*> semValueList;
							
							for (size_t i = 0; i < astValueList->size(); i++) {
								const auto semArgValue = ConvertValue(context, astValueList->at(i));
								const auto param = ImplicitCast(context, semArgValue, typeList.at(i));
								semValueList.push_back(param);
							}
							
							return SEM::Value::InterfaceMethodCall(functionValue, semValueList);
						}
						default: {
							throw ErrorException(makeString("Can't call value '%s' that isn't a function or a method.",
								functionValue->toString().c_str()));
						}
					}
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


