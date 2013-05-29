#include <cassert>
#include <cstdio>
#include <list>
#include <map>
#include <string>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/CanCast.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertType.hpp>
#include <Locic/SemanticAnalysis/ConvertValue.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Value* ConvertValue(Context& context, AST::Value* value) {
			assert(value != NULL && "Cannot convert NULL AST::Value");
			
			switch(value->typeEnum) {
				case AST::Value::CONSTANT: {
					if(value->constant->getType() == Locic::Constant::NULLVAL) {
						return SEM::Value::Constant(value->constant, SEM::Type::Null());
					} else if(value->constant->getType() == Locic::Constant::STRING
						&& value->constant->getStringType() == Locic::Constant::CSTRING) {
						// C strings have the type 'const char * const', as opposed to just a
						// type name, so their type needs to be generated specially.
						SEM::TypeInstance* charType = context.getBuiltInType("char");
						
						SEM::Type* constCharPtrType = SEM::Type::Pointer(SEM::Type::CONST, SEM::Type::RVALUE,
								SEM::Type::Object(SEM::Type::CONST, SEM::Type::LVALUE, charType,
									SEM::Type::NO_TEMPLATE_ARGS));
						return SEM::Value::Constant(value->constant, constCharPtrType);
					} else {
						const std::string typeName = value->constant->getTypeName();
						SEM::TypeInstance* typeInstance = context.getBuiltInType(typeName);
						if(typeInstance == NULL) {
							printf("Couldn't find '::%s' constant type.\n", typeName.c_str());
						}
						assert(typeInstance != NULL && "Couldn't find constant type");
						return SEM::Value::Constant(value->constant,
								SEM::Type::Object(SEM::Type::CONST, SEM::Type::RVALUE,
								typeInstance, SEM::Type::NO_TEMPLATE_ARGS));
					}
					
					assert(false && "Invalid if fallthrough in ConvertValue for constant");
					return NULL;
				}
				case AST::Value::SYMBOLREF: {
					const AST::Symbol& symbol = value->symbolRef.symbol;
					const Name name = symbol.createName();
					
					// Not a local variable => do a symbol lookup.
					const Node node = context.lookupName(name);
					
					// Get a map from template variables to their values (i.e. types).
					const Map<SEM::TemplateVar*, SEM::Type*> templateVarMap = GenerateTemplateVarMap(context, symbol);
					
					if(node.isNone()) {
						throw TodoException(makeString("Couldn't find symbol or value '%s'.", name.toString().c_str()));
					} else if(node.isNamespace()) {
						throw TodoException(makeString("Namespace '%s' is not a valid value.", name.toString().c_str()));
					} else if(node.isFunction()) {
						SEM::Function* function = node.getSEMFunction();
						assert(function != NULL && "Function pointer must not be NULL (as indicated by isFunction() being true)");
						
						if (function->isMethod()) {
							if (!function->isStatic()) {
								throw TodoException(makeString("Cannot refer directly to non-static class method '%s'.",
									name.toString().c_str()));
							}
							
							const Node typeNode = context.lookupName(name.getPrefix());
							assert(node.isTypeInstance());
							
							SEM::Type* parentType = SEM::Type::Object(SEM::Type::MUTABLE, SEM::Type::LVALUE,
								node.getSEMTypeInstance(), GetTemplateValues(context, symbol));
							
							return SEM::Value::FunctionRef(parentType, function, templateVarMap);
						} else {
							return SEM::Value::FunctionRef(NULL, function, templateVarMap);
						}
					} else if(node.isTypeInstance()) {
						SEM::TypeInstance* typeInstance = node.getSEMTypeInstance();
						
						if(typeInstance->isInterface()) {
							throw TodoException(makeString("Can't construct interface type '%s'.", name.toString().c_str()));
						}
						
						const Node defaultConstructorNode = node.getChild("Default");
						if(!defaultConstructorNode.isFunction()) {
							throw TodoException(makeString("Couldn't find default constructor for type '%s'.",
								name.toString().c_str()));
						}
						
						SEM::Type* parentType = SEM::Type::Object(SEM::Type::MUTABLE, SEM::Type::LVALUE, typeInstance, GetTemplateValues(context, symbol));
						
						return SEM::Value::FunctionRef(parentType,
							defaultConstructorNode.getSEMFunction(), templateVarMap);
					} else if(node.isVariable()) {
						// Variables must just be a single plain string,
						// and be a relative name (so no precending '::').
						// TODO: make these throw exceptions.
						assert(symbol.size() == 1);
						assert(symbol.isRelative());
						assert(symbol.first().templateArguments().empty());
						return SEM::Value::VarValue(node.getSEMVar());
					} else if(node.isTemplateVar()) {
						assert(templateVarMap.empty() && "Template vars cannot have template arguments.");
						SEM::TemplateVar* templateVar = node.getSEMTemplateVar();
						
						SEM::Type* specType = templateVar->specType();
						assert(specType != NULL && "Can't find default constructor in template type (without spec type).");
						
						SEM::TypeInstance* specTypeInstance = specType->getObjectType();
						
						SEM::Function* defaultConstructor = specTypeInstance->getDefaultConstructor();
						
						return SEM::Value::FunctionRef(SEM::Type::TemplateVarRef(SEM::Type::MUTABLE, SEM::Type::LVALUE, templateVar),
							defaultConstructor, specType->generateTemplateVarMap());
					} else {
						assert(false && "Unknown node for name reference");
						return NULL;
					}
					
					assert(false && "Invalid if-statement fallthrough in ConvertValue for name reference");
					return NULL;
				}
				case AST::Value::MEMBERREF: {
					const std::string& memberName = value->memberRef.name;
					SEM::Var* semVar = context.getParentMemberVariable(memberName).getSEMVar();
					
					if(semVar == NULL) {
						throw TodoException(makeString("Member variable '@%s' not found.",
								memberName.c_str()));
					}
					
					return SEM::Value::VarValue(semVar);
				}
				case AST::Value::ADDRESSOF: {
					SEM::Value* operand = ConvertValue(context, value->addressOf.value);
					
					if(operand->type()->isLValue()) {
						return SEM::Value::AddressOf(operand);
					}
					
					throw TodoException(makeString("Attempted to take address of R-value '%s'.",
						operand->toString().c_str()));
				}
				case AST::Value::DEREFERENCE: {
					SEM::Value* operand = ConvertValue(context, value->dereference.value);
					
					if(operand->type()->isPointer()) {
						return SEM::Value::DerefPointer(operand);
					}
					
					throw TodoException(makeString("Attempted to dereference non-pointer value '%s'.",
						operand->toString().c_str()));
				}
				case AST::Value::TERNARY: {
					SEM::Value* cond = ConvertValue(context, value->ternary.condition);
					
					SEM::TypeInstance* boolType = context.getBuiltInType("bool");
					
					SEM::Value* boolValue = ImplicitCast(cond,
							SEM::Type::Object(SEM::Type::CONST, SEM::Type::RVALUE, boolType, SEM::Type::NO_TEMPLATE_ARGS));
							
					SEM::Value* ifTrue = ConvertValue(context, value->ternary.ifTrue);
					SEM::Value* ifFalse = ConvertValue(context, value->ternary.ifFalse);
					
					SEM::Type* targetType = UnifyTypes(ifTrue->type(), ifFalse->type());
					
					// Can only result in an lvalue if both possible results are lvalues.
					if(ifTrue->type()->isRValue() || ifFalse->type()->isRValue()) {
						targetType = targetType->createRValueType();
					}
					
					SEM::Value* castIfTrue = ImplicitCast(ifTrue, targetType);
					SEM::Value* castIfFalse = ImplicitCast(ifFalse, targetType);
					
					return SEM::Value::Ternary(boolValue, castIfTrue, castIfFalse);
				}
				case AST::Value::CAST: {
					SEM::Type* type = ConvertType(context, value->cast.targetType, SEM::Type::RVALUE);
					SEM::Value* val = ConvertValue(context, value->cast.value);
					
					(void) type;
					(void) val;
					
					std::string s;
					switch(value->cast.castKind) {
						case AST::Value::CAST_STATIC:
							s = "STATIC";
							break;
						case AST::Value::CAST_CONST:
							s = "CONST";
							break;
						case AST::Value::CAST_DYNAMIC:
							s = "DYNAMIC";
							break;
						case AST::Value::CAST_REINTERPRET:
							s = "REINTERPRET";
							break;
						default:
							assert(false && "Unknown cast kind");
							return NULL;
					}
					
					throw TodoException(makeString("Casts of kind '%s' not yet implemented.",
						s.c_str()));
				}
				case AST::Value::INTERNALCONSTRUCT: {
					const std::vector<AST::Value*>& astValues = value->internalConstruct.parameters;
					
					const Node thisTypeNode = context.lookupParentType();
					
					// TODO: throw an exception.
					assert(thisTypeNode.isTypeInstance());
					
					SEM::TypeInstance* thisTypeInstance = thisTypeNode.getSEMTypeInstance();
					
					if(astValues.size() != thisTypeInstance->variables().size()) {
						throw TodoException(makeString("Internal constructor called "
							   "with wrong number of arguments; received %llu, expected %llu.",
							(unsigned long long) astValues.size(),
							(unsigned long long) thisTypeInstance->variables().size()));
					}
					
					std::vector<SEM::Value*> semValues;
					
					for(size_t i = 0; i < thisTypeInstance->variables().size(); i++){
						SEM::Var* var = thisTypeInstance->variables().at(i);
						SEM::Value* semValue = ConvertValue(context, astValues.at(i));
						SEM::Value* semParam = ImplicitCast(semValue, var->type());
						semValues.push_back(semParam);
					}
					
					return SEM::Value::InternalConstruct(thisTypeInstance, semValues);
				}
				case AST::Value::MEMBERACCESS: {
					const std::string memberName = value->memberAccess.memberName;
					
					SEM::Value* object = ConvertValue(context, value->memberAccess.object);
					
					// Any number of levels of references are automatically dereferenced.
					while(object->type()->isReference()) {
						object = SEM::Value::DerefReference(object);
					}
					
					if(!object->type()->isObject() && !object->type()->isTemplateVar()) {
						throw TodoException(makeString("Can't access member of non-object value '%s'.",
							object->toString().c_str()));
					}
					
					SEM::TypeInstance* typeInstance =
						object->type()->isTemplateVar() ?
							object->type()->getTemplateVar()->specType()->getObjectType() :
							object->type()->getObjectType();
					assert(typeInstance != NULL);
					
					const Node typeNode = context.reverseLookup(typeInstance);
					assert(typeNode.isNotNone());
					
					if(typeInstance->isStructDef()) {
						// Look for struct variables.
						const Node varNode = typeNode.getChild(memberName);
						
						if(varNode.isNotNone()) {
							assert(varNode.isVariable());
							SEM::Var* var = varNode.getSEMVar();
							SEM::Type* memberType = var->type();
							
							if(object->type()->isConst()) {
								// If the struct type is const, then the members must
								// also be.
								memberType = memberType->createConstType();
							}
							
							if(object->type()->isRValue()) {
								// If the struct type is an R-value, then the member must
								// also be (preventing assignments to R-value members).
								memberType = memberType->createRValueType();
							}
							
							return SEM::Value::MemberAccess(object, var, memberType);
						} else {
							throw TodoException(makeString("Can't access struct member '%s' in type '%s'.",
								memberName.c_str(), typeInstance->name().toString().c_str()));
						}
					} else if(typeInstance->isClass() || typeInstance->isPrimitive() || typeInstance->isInterface()) {
						// Look for class methods.
						const Node childNode = typeNode.getChild(memberName);
						
						if(childNode.isFunction()) {
							SEM::Function* function = childNode.getSEMFunction();
							
							assert(function->isMethod());
							
							if(function->isStatic()) {
								throw TodoException(makeString("Cannot call static function '%s' in type '%s'.",
									function->name().toString().c_str(), typeInstance->name().toString().c_str()));
							}
							
							SEM::Value* functionRef = SEM::Value::FunctionRef(object->type(), function, object->type()->generateTemplateVarMap());
							
							if (typeInstance->isInterface() && !object->type()->isTemplateVar()) {
								return SEM::Value::InterfaceMethodObject(functionRef, object);
							} else {
								return SEM::Value::MethodObject(functionRef, object);
							}
						} else {
							throw TodoException(makeString("Can't find method '%s' in type '%s'.",
								memberName.c_str(), typeInstance->name().toString().c_str()));
						}
					} else if(typeInstance->isStructDecl()) {
						throw TodoException(makeString("Can't access member '%s' in unspecified struct type '%s'.",
							memberName.c_str(), typeInstance->name().toString().c_str()));
					}
					
					assert(false && "Invalid switch fallthrough in ConvertValue for member access");
					return NULL;
				}
				case AST::Value::FUNCTIONCALL: {
					assert(value->functionCall.functionValue != NULL && "Cannot call NULL function value");
					SEM::Value* functionValue = ConvertValue(context, value->functionCall.functionValue);
					
					switch(functionValue->type()->kind()) {
						case SEM::Type::FUNCTION: {
							const std::vector<SEM::Type*>& typeList = functionValue->type()->getFunctionParameterTypes();
							const std::vector<AST::Value*>& astValueList = value->functionCall.parameters;
							
							if(functionValue->type()->isFunctionVarArg()) {
								if(astValueList.size() < typeList.size()) {
									throw TodoException(makeString("Var Arg Function [%s] called with %llu number of parameters; expected at least %llu.",
										functionValue->toString().c_str(),
										(unsigned long long) astValueList.size(),
										(unsigned long long) typeList.size()));
								}
							} else {
								if(astValueList.size() != typeList.size()) {
									throw TodoException(makeString("Function [%s] called with %llu number of parameters; expected %llu.",
										functionValue->toString().c_str(),
										(unsigned long long) astValueList.size(),
										(unsigned long long) typeList.size()));
								}
							}
							
							assert(astValueList.size() >= typeList.size());
							
							std::vector<SEM::Value*> semValueList;
							
							for(std::size_t i = 0; i < astValueList.size(); i++) {
								SEM::Value* semArgValue = ConvertValue(context, astValueList.at(i));
								
								SEM::Value* param = (i < typeList.size()) ?
										ImplicitCast(semArgValue, typeList.at(i)->createRValueType()) :
										semArgValue;
										
								semValueList.push_back(param);
							}
							
							return SEM::Value::FunctionCall(functionValue, semValueList);
						}
						case SEM::Type::METHOD: {
							SEM::Type* functionType = functionValue->type()->getMethodFunctionType();
							
							const std::vector<SEM::Type*>& typeList = functionType->getFunctionParameterTypes();
							const std::vector<AST::Value*>& astValueList = value->functionCall.parameters;
							
							assert(!functionType->isFunctionVarArg() && "Methods cannot be var args");
							
							if(typeList.size() != astValueList.size()) {
								throw TodoException(makeString("Method [%s] called with %lu number of parameters; expected %lu.",
									functionValue->toString().c_str(),
									astValueList.size(), typeList.size()));
							}
							
							std::vector<SEM::Value*> semValueList;
							
							for(std::size_t i = 0; i < astValueList.size(); i++) {
								SEM::Value* semArgValue = ConvertValue(context, astValueList.at(i));
								
								SEM::Value* param = ImplicitCast(semArgValue, typeList.at(i));
								
								semValueList.push_back(param);
							}
							
							return SEM::Value::MethodCall(functionValue, semValueList);
						}
						case SEM::Type::INTERFACEMETHOD: {
							SEM::Type* functionType = functionValue->type()->getInterfaceMethodFunctionType();
							
							const std::vector<SEM::Type*>& typeList = functionType->getFunctionParameterTypes();
							const std::vector<AST::Value*>& astValueList = value->functionCall.parameters;
							
							assert(!functionType->isFunctionVarArg() && "Methods cannot be var args");
							
							if(typeList.size() != astValueList.size()) {
								throw TodoException(makeString("Method [%s] called with %lu number of parameters; expected %lu.",
									functionValue->toString().c_str(),
									astValueList.size(), typeList.size()));
							}
							
							std::vector<SEM::Value*> semValueList;
							
							for(std::size_t i = 0; i < astValueList.size(); i++) {
								SEM::Value* semArgValue = ConvertValue(context, astValueList.at(i));
								
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
					assert(false && "Unknown AST::Value type enum");
					return NULL;
			}
		}
		
	}
	
}


