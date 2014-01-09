#include <cassert>
#include <cstdio>
#include <list>
#include <map>
#include <string>
#include <locic/AST.hpp>
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
		
		SEM::Value* MakeMemberAccess(Context& context, SEM::Value* accessObject, const std::string& memberName) {
			const auto object = derefValue(accessObject);
			const auto objectType = getDerefType(accessObject->type());
			
			if (!objectType->isObject() && !objectType->isTemplateVar()) {
				throw TodoException(makeString("Can't access member of non-object value '%s' of type '%s'.",
					object->toString().c_str(), objectType->toString().c_str()));
			}
			
			const auto typeInstance =
				objectType->isTemplateVar() ?
					objectType->getTemplateVar()->specType() :
					objectType->getObjectType();
			assert(typeInstance != NULL);
			
			const Node typeNode = context.reverseLookup(typeInstance);
			assert(typeNode.isNotNone());
			
			if (typeInstance->isStruct() || typeInstance->isDatatype()) {
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
				} else {
					throw TodoException(makeString("Can't access struct member '%s' in type '%s'.",
						memberName.c_str(), typeInstance->name().toString().c_str()));
				}
			} else if (typeInstance->isClass() || typeInstance->isTemplateType() || typeInstance->isPrimitive() || typeInstance->isInterface()) {
				// Look for methods.
				const Node childNode = typeNode.getChild(memberName);
				
				if (childNode.isFunction()) {
					const auto function = childNode.getSEMFunction();
					
					assert(function->isMethod());
					
					if (function->isStaticMethod()) {
						throw TodoException(makeString("Cannot call static function '%s' in type '%s'.",
							function->name().toString().c_str(), typeInstance->name().toString().c_str()));
					}
					
					if (objectType->isConst() && !function->isConstMethod()) {
						throw TodoException(makeString("Cannot refer to mutator method '%s' from const object '%s' of type '%s'.",
							function->name().toString().c_str(), accessObject->toString().c_str(),
							objectType->toString().c_str()));
					}
					
					const auto functionRef = SEM::Value::FunctionRef(objectType, function, objectType->generateTemplateVarMap());
					
					if (typeInstance->isInterface()) {
						return SEM::Value::InterfaceMethodObject(functionRef, object);
					} else {
						return SEM::Value::MethodObject(functionRef, object);
					}
				} else {
					throw TodoException(makeString("Can't find method '%s' in type '%s' with value %s.",
						memberName.c_str(), typeInstance->name().toString().c_str(),
						object->toString().c_str()));
				}
			} else {
				assert(false && "Invalid fall through in MakeMemberAccess.");
				return NULL;
			}
		}
	
		SEM::Value* ConvertValue(Context& context, AST::Node<AST::Value> astValueNode) {
			assert(astValueNode.get() != NULL);
			
			switch (astValueNode->typeEnum) {
				case AST::Value::BRACKET: {
					return ConvertValue(context, astValueNode->bracket.value);
				}
				case AST::Value::CONSTANT: {
					if (astValueNode->constant->getType() == locic::Constant::NULLVAL) {
						return SEM::Value::Constant(astValueNode->constant.get(), SEM::Type::Null());
					} else if(astValueNode->constant->getType() == locic::Constant::STRING
						&& astValueNode->constant->getStringType() == locic::Constant::CSTRING) {
						// C strings have the type 'const char * const', as opposed to just a
						// type name, so their type needs to be generated specially.
						const auto charTypeInstance = context.getBuiltInType("char");
						const auto ptrTypeInstance = context.getBuiltInType("ptr");
						
						// Generate type 'const char'.
						auto constCharType = SEM::Type::Object(charTypeInstance, SEM::Type::NO_TEMPLATE_ARGS)->createConstType();
						
						// Generate type 'const ptr<const char>'.
						auto constCharPtrType = SEM::Type::Object(ptrTypeInstance, std::vector<SEM::Type*>(1, constCharType))->createConstType();
						
						return SEM::Value::Constant(astValueNode->constant.get(), constCharPtrType);
					} else {
						const std::string typeName = astValueNode->constant->getTypeName();
						if (typeName == "string") {
							throw TodoException("Loci string constants not yet implemented (use 'C' suffix to get C strings for now, e.g. \"someString\"C).");
						}
						
						const auto typeInstance = context.getBuiltInType(typeName);
						if (typeInstance == NULL) {
							throw TodoException(makeString("Couldn't find constant type '%s' when generating value constant.",
								typeName.c_str()));
						}
						
						return SEM::Value::Constant(astValueNode->constant.get(),
								SEM::Type::Object(typeInstance, SEM::Type::NO_TEMPLATE_ARGS)->createConstType());
					}
					
					assert(false && "Invalid if fallthrough in ConvertValue for constant");
					return NULL;
				}
				case AST::Value::SYMBOLREF: {
					const auto& astSymbolNode = astValueNode->symbolRef.symbol;
					const Name name = astSymbolNode->createName();
					
					// Not a local variable => do a symbol lookup.
					const Node node = context.lookupName(name);
					
					// Get a map from template variables to their values (i.e. types).
					const auto templateVarMap = GenerateTemplateVarMap(context, astSymbolNode);
					
					if (node.isNone()) {
						throw TodoException(makeString("Couldn't find symbol or value '%s' at %s.",
							name.toString().c_str(), astSymbolNode.location().toString().c_str()));
					} else if (node.isNamespace()) {
						throw TodoException(makeString("Namespace '%s' is not a valid value at %s.",
							name.toString().c_str(), astSymbolNode.location().toString().c_str()));
					} else if (node.isFunction()) {
						SEM::Function* function = node.getSEMFunction();
						assert(function != NULL && "Function pointer must not be NULL (as indicated by isFunction() being true)");
						
						if (function->isMethod()) {
							if (!function->isStaticMethod()) {
								throw TodoException(makeString("Cannot refer directly to non-static class method '%s' at %s.",
									name.toString().c_str(), astSymbolNode.location().toString().c_str()));
							}
							
							const Node typeNode = context.lookupName(name.getPrefix());
							assert(typeNode.isTypeInstance());
							
							const auto typeInstance = typeNode.getSEMTypeInstance();
							
							const auto parentType = SEM::Type::Object(typeInstance, GetTemplateValues(context, astSymbolNode));
							
							return SEM::Value::FunctionRef(parentType, function, templateVarMap);
						} else {
							return SEM::Value::FunctionRef(NULL, function, templateVarMap);
						}
					} else if (node.isTypeInstance()) {
						auto typeInstance = node.getSEMTypeInstance();
						
						if (typeInstance->isInterface()) {
							throw TodoException(makeString("Can't construct interface type '%s' at %s.",
								name.toString().c_str(), astSymbolNode.location().toString().c_str()));
						}
						
						if (!typeInstance->hasProperty("Create")) {
							throw TodoException(makeString("Couldn't find default constructor for type '%s' at %s.",
								name.toString().c_str(), astSymbolNode.location().toString().c_str()));
						}
						
						auto parentType = SEM::Type::Object(typeInstance, GetTemplateValues(context, astSymbolNode));
						
						auto defaultConstructor = typeInstance->getProperty("Create");
						
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
						
						const auto specTypeInstance = templateVar->specType();
						const auto defaultConstructor = specTypeInstance->getProperty("Create");
						
						return SEM::Value::FunctionRef(SEM::Type::TemplateVarRef(templateVar),
							defaultConstructor, Map<SEM::TemplateVar*, SEM::Type*>());
					} else {
						assert(false && "Unknown node for name reference");
						return NULL;
					}
					
					assert(false && "Invalid if-statement fallthrough in ConvertValue for name reference");
					return NULL;
				}
				case AST::Value::MEMBERREF: {
					const std::string& memberName = astValueNode->memberRef.name;
					SEM::Var* semVar = context.getParentMemberVariable(memberName).getSEMVar();
					
					if(semVar == NULL) {
						throw TodoException(makeString("Member variable '@%s' not found.",
								memberName.c_str()));
					}
					
					return SEM::Value::MemberVar(semVar);
				}
				case AST::Value::TERNARY: {
					SEM::Value* cond = ConvertValue(context, astValueNode->ternary.condition);
					
					SEM::TypeInstance* boolType = context.getBuiltInType("bool");
					
					SEM::Value* boolValue = ImplicitCast(cond,
							SEM::Type::Object(boolType, SEM::Type::NO_TEMPLATE_ARGS));
							
					SEM::Value* ifTrue = ConvertValue(context, astValueNode->ternary.ifTrue);
					SEM::Value* ifFalse = ConvertValue(context, astValueNode->ternary.ifFalse);
					
					SEM::Type* targetType = UnifyTypes(ifTrue->type(), ifFalse->type());
					
					SEM::Value* castIfTrue = ImplicitCast(ifTrue, targetType);
					SEM::Value* castIfFalse = ImplicitCast(ifFalse, targetType);
					
					return SEM::Value::Ternary(boolValue, castIfTrue, castIfFalse);
				}
				case AST::Value::CAST: {
					SEM::Value* sourceValue = ConvertValue(context, astValueNode->cast.value);
					SEM::Type* sourceType = ConvertType(context, astValueNode->cast.sourceType);
					SEM::Type* targetType = ConvertType(context, astValueNode->cast.targetType);
					
					switch(astValueNode->cast.castKind) {
						case AST::Value::CAST_STATIC:
							throw TodoException("static_cast not yet implemented.");
						case AST::Value::CAST_CONST:
							throw TodoException("const_cast not yet implemented.");
						case AST::Value::CAST_DYNAMIC:
							throw TodoException("dynamic_cast not yet implemented.");
						case AST::Value::CAST_REINTERPRET:
							if (!sourceType->isPrimitive() || sourceType->getObjectType()->name().last() != "ptr"
								|| !targetType->isPrimitive() || targetType->getObjectType()->name().last() != "ptr") {
								throw TodoException(makeString("reinterpret_cast currently only supports ptr<T>, "
									"in cast from value %s of type %s to type %s.", sourceValue->toString().c_str(),
									sourceType->toString().c_str(), targetType->toString().c_str()));
							}
							return SEM::Value::Reinterpret(ImplicitCast(sourceValue, sourceType), targetType);
						default:
							assert(false && "Unknown cast kind");
							return NULL;
					}
				}
				case AST::Value::LVAL: {
					SEM::Value* sourceValue = ConvertValue(context, astValueNode->makeLval.value);
					
					if (sourceValue->type()->isLval()) {
						throw TodoException(makeString("Can't create lval of value that is already a lval, for value '%s'.",
							sourceValue->toString().c_str()));
					}
					
					if (sourceValue->type()->isRef()) {
						throw TodoException(makeString("Can't create value that is both an lval and a ref, for value '%s'.",
							sourceValue->toString().c_str()));
					}
					
					SEM::Type* targetType = ConvertType(context, astValueNode->makeLval.targetType);
					return SEM::Value::Lval(targetType, sourceValue);
				}
				case AST::Value::REF: {
					SEM::Value* sourceValue = ConvertValue(context, astValueNode->makeRef.value);
					
					if (sourceValue->type()->isLval()) {
						throw TodoException(makeString("Can't create value that is both an lval and a ref, for value '%s'.",
							sourceValue->toString().c_str()));
					}
					
					if (sourceValue->type()->isRef()) {
						throw TodoException(makeString("Can't create ref of value that is already a ref, for value '%s'.",
							sourceValue->toString().c_str()));
					}
					
					SEM::Type* targetType = ConvertType(context, astValueNode->makeRef.targetType);
					return SEM::Value::Ref(targetType, sourceValue);
				}
				case AST::Value::INTERNALCONSTRUCT: {
					const auto& astParameterValueNodes = astValueNode->internalConstruct.parameters;
					
					const Node thisTypeNode = context.lookupParentType();
					
					// TODO: throw an exception.
					assert(thisTypeNode.isTypeInstance());
					
					SEM::TypeInstance* thisTypeInstance = thisTypeNode.getSEMTypeInstance();
					
					if (astParameterValueNodes->size() != thisTypeInstance->variables().size()) {
						throw TodoException(makeString("Internal constructor called "
							   "with wrong number of arguments; received %llu, expected %llu.",
							(unsigned long long) astParameterValueNodes->size(),
							(unsigned long long) thisTypeInstance->variables().size()));
					}
					
					std::vector<SEM::Value*> semValues;
					
					for(size_t i = 0; i < thisTypeInstance->variables().size(); i++){
						SEM::Type* constructType = thisTypeInstance->constructTypes().at(i);
						SEM::Value* semValue = ConvertValue(context, astParameterValueNodes->at(i));
						SEM::Value* semParam = ImplicitCast(semValue, constructType);
						semValues.push_back(semParam);
					}
					
					return SEM::Value::InternalConstruct(thisTypeInstance, semValues);
				}
				case AST::Value::MEMBERACCESS: {
					const std::string memberName = astValueNode->memberAccess.memberName;
					
					SEM::Value* object = ConvertValue(context, astValueNode->memberAccess.object);
					
					if (memberName != "address" && memberName != "assign" && memberName != "dissolve" && memberName != "move") {
						object = tryDissolveValue(object);
					}
					
					return MakeMemberAccess(context, object, memberName);
				}
				case AST::Value::FUNCTIONCALL: {
					assert(astValueNode->functionCall.functionValue.get() != NULL && "Cannot call NULL function value");
					SEM::Value* functionValue = ConvertValue(context, astValueNode->functionCall.functionValue);
					
					switch(functionValue->type()->kind()) {
						case SEM::Type::FUNCTION: {
							const std::vector<SEM::Type*>& typeList = functionValue->type()->getFunctionParameterTypes();
							const auto& astValueList = astValueNode->functionCall.parameters;
							
							if (functionValue->type()->isFunctionVarArg()) {
								if(astValueList->size() < typeList.size()) {
									throw TodoException(makeString("Var Arg Function [%s] called with %llu number of parameters; expected at least %llu.",
										functionValue->toString().c_str(),
										(unsigned long long) astValueList->size(),
										(unsigned long long) typeList.size()));
								}
							} else {
								if(astValueList->size() != typeList.size()) {
									throw TodoException(makeString("Function [%s] called with %llu number of parameters; expected %llu.",
										functionValue->toString().c_str(),
										(unsigned long long) astValueList->size(),
										(unsigned long long) typeList.size()));
								}
							}
							
							assert(astValueList->size() >= typeList.size());
							
							std::vector<SEM::Value*> semValueList;
							
							for(std::size_t i = 0; i < astValueList->size(); i++) {
								SEM::Value* semArgValue = ConvertValue(context, astValueList->at(i));
								
								// Cast arguments to the function type's corresponding
								// argument type; var-arg arguments should be cast to
								// one of the allowed types (since there's no specific
								// destination type).
								SEM::Value* param = (i < typeList.size()) ?
										ImplicitCast(semArgValue, typeList.at(i)) :
										VarArgCast(semArgValue);
										
								semValueList.push_back(param);
							}
							
							return SEM::Value::FunctionCall(functionValue, semValueList);
						}
						case SEM::Type::METHOD: {
							SEM::Type* functionType = functionValue->type()->getMethodFunctionType();
							
							const std::vector<SEM::Type*>& typeList = functionType->getFunctionParameterTypes();
							const auto& astValueList = astValueNode->functionCall.parameters;
							
							assert(!functionType->isFunctionVarArg() && "Methods cannot be var args");
							
							if (typeList.size() != astValueList->size()) {
								throw TodoException(makeString("Method [%s] called with %lu number of parameters; expected %lu.",
									functionValue->toString().c_str(),
									astValueList->size(), typeList.size()));
							}
							
							std::vector<SEM::Value*> semValueList;
							
							for(std::size_t i = 0; i < astValueList->size(); i++) {
								SEM::Value* semArgValue = ConvertValue(context, astValueList->at(i));
								
								SEM::Value* param = ImplicitCast(semArgValue, typeList.at(i));
								
								semValueList.push_back(param);
							}
							
							return SEM::Value::MethodCall(functionValue, semValueList);
						}
						case SEM::Type::INTERFACEMETHOD: {
							SEM::Type* functionType = functionValue->type()->getInterfaceMethodFunctionType();
							
							const std::vector<SEM::Type*>& typeList = functionType->getFunctionParameterTypes();
							const auto& astValueList = astValueNode->functionCall.parameters;
							
							assert(!functionType->isFunctionVarArg() && "Methods cannot be var args");
							
							if (typeList.size() != astValueList->size()) {
								throw TodoException(makeString("Method [%s] called with %lu number of parameters; expected %lu.",
									functionValue->toString().c_str(),
									astValueList->size(), typeList.size()));
							}
							
							std::vector<SEM::Value*> semValueList;
							
							for (std::size_t i = 0; i < astValueList->size(); i++) {
								SEM::Value* semArgValue = ConvertValue(context, astValueList->at(i));
								
								SEM::Value* param = ImplicitCast(semArgValue, typeList.at(i));
								
								semValueList.push_back(param);
							}
							
							return SEM::Value::InterfaceMethodCall(functionValue, semValueList);
						}
						default: {
							throw TodoException(makeString("Can't call value '%s' that isn't a function or a method.",
								functionValue->toString().c_str()));
						}
					}
				}
				default:
					assert(false && "Unknown AST::Value kind enum");
					return NULL;
			}
		}
		
	}
	
}


