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

SEM::Value* ConvertValue(LocalContext& context, AST::Value* value) {
	assert(value != NULL && "Cannot convert NULL AST::Value");
	
	switch(value->typeEnum) {
		case AST::Value::CONSTANT: {
			if(value->constant->getType() == Locic::Constant::NULLVAL){
				return SEM::Value::Constant(value->constant, SEM::Type::Null(SEM::Type::CONST));
			}else if(value->constant->getType() == Locic::Constant::STRING && value->constant->getStringType() == Locic::Constant::CSTRING){
				// C strings have the type 'const char *', as opposed to just a
				// type name, so their type needs to be generated specially.
				SEM::TypeInstance * charType = context.getNode(Name::Absolute() + "char").getTypeInstance();
				assert(charType != NULL && "Couldn't find char constant type");
				SEM::Type * constCharPtrType = SEM::Type::Pointer(SEM::Type::MUTABLE, SEM::Type::RVALUE,
					SEM::Type::Named(SEM::Type::CONST, SEM::Type::LVALUE, charType));
				return SEM::Value::Constant(value->constant, constCharPtrType);
			}else{
				const std::string typeName = value->constant->getTypeName();
				SEM::TypeInstance * typeInstance = context.getNode(Name::Absolute() + typeName).getTypeInstance();
				if(typeInstance == NULL){
					printf("Couldn't find '::%s' constant type.\n", typeName.c_str());
				}
				assert(typeInstance != NULL && "Couldn't find constant type");
				return SEM::Value::Constant(value->constant, SEM::Type::Named(SEM::Type::CONST, SEM::Type::RVALUE, typeInstance));
			}
			
			assert(false && "Invalid if fallthrough in ConvertValue for constant");
			return NULL;
		}
		case AST::Value::NAMEREF: {
			const Locic::Name& name = value->nameRef.name;
			
			// Check if it could be a local variable.
			// Local variables must just be a single plain string,
			// and be a relative name (so no precending '::').
			if(name.size() == 1 && name.isRelative()){
				SEM::Var* semVar = context.findLocalVar(name.first());
				
				if(semVar != NULL) {
					return SEM::Value::VarValue(semVar);
				}
			}
			
			// Not a local variable => do a symbol lookup.
			SEM::NamespaceNode node = context.getNode(name);
			
			if(node.isNone()){
				printf("Semantic Analysis Error: Couldn't find symbol or value '%s'.\n", name.toString().c_str());
				return NULL;
			}
			
			if(node.isNamespace()){
				printf("Semantic Analysis Error: Namespace '%s' is not a valid value.\n", name.toString().c_str());
				return NULL;
			}
			
			if(node.isFunction()){
				SEM::Function * function = node.getFunction();
				assert(function != NULL && "Function pointer must not be NULL (as indicated by isFunction() being true)");
				return SEM::Value::FunctionRef(function, function->type);
			}else if(node.isTypeInstance()){
				SEM::TypeInstance * typeInstance = node.getTypeInstance();
				assert(typeInstance != NULL && "Type instance pointer must not be NULL (as indicated by isTypeInstance() being true)");
				
				SEM::Function * function = typeInstance->lookup(typeInstance->name + "Default").getFunction();
				if(function == NULL){
					printf("Semantic Analysis Error: Couldn't find default constructor for type '%s' (full name: %s).\n",
						name.toString().c_str(), typeInstance->name.toString().c_str());
					return NULL;
				}
				
				return SEM::Value::FunctionRef(function, function->type);
			}else{
				assert(false && "Unknown node for name reference");
				return NULL;
			}
			
			assert(false && "Invalid if-statement fallthrough in ConvertValue for name reference");
			return NULL;
		}
		case AST::Value::MEMBERREF: {
			const std::string& memberName = value->memberRef.name;
			SEM::Var* semVar = context.getThisVar(memberName);
			
			if(semVar == NULL){
				printf("Semantic Analysis Error: member variable '@%s' not found.\n", memberName.c_str());
				return NULL;
			}
			
			return SEM::Value::VarValue(semVar);
		}
		case AST::Value::ADDRESSOF: {
			SEM::Value* operand = ConvertValue(context, value->addressOf.value);
		
			if(operand->type->isLValue) {
				return SEM::Value::AddressOf(operand, SEM::Type::Pointer(SEM::Type::MUTABLE, SEM::Type::RVALUE, operand->type));
			}
			
			printf("Semantic Analysis Error: Attempting to take address of R-value.\n");
			return NULL;
		}
		case AST::Value::DEREFERENCE: {
			SEM::Value* operand = ConvertValue(context, value->dereference.value);
		
			if(operand->type->typeEnum == SEM::Type::POINTER) {
				return SEM::Value::Deref(operand, operand->type->pointerType.targetType);
			}
			
			printf("Semantic Analysis Error: Attempting to dereference non-pointer type.\n");
			return NULL;
		}
		case AST::Value::TERNARY: {
			SEM::Value* cond = ConvertValue(context, value->ternary.condition);
			
			SEM::TypeInstance * boolType = context.getNode(Name::Absolute() + "bool").getTypeInstance();
			assert(boolType != NULL && "Couldn't find bool type");
			SEM::Value* boolValue = CastValueToType(cond, SEM::Type::Named(SEM::Type::CONST, SEM::Type::RVALUE, boolType));
			
			if(boolValue == NULL) {
				printf("Semantic Analysis Error: Cannot cast or copy condition type (%s) to bool type in ternary operator.\n",
				       cond->type->toString().c_str());
				return NULL;
			}
			
			SEM::Value* ifTrue = ConvertValue(context, value->ternary.ifTrue);
			SEM::Value* ifFalse = ConvertValue(context, value->ternary.ifFalse);
			
			SEM::Type* ifTrueType = new SEM::Type(*(ifTrue->type));
			SEM::Type* ifFalseType = new SEM::Type(*(ifFalse->type));
			
			// Can only result in an lvalue if both possible results are lvalues.
			if(ifTrueType->isLValue == SEM::Type::RVALUE || ifFalseType->isLValue == SEM::Type::RVALUE) {
				ifTrueType->isLValue = SEM::Type::RVALUE;
				ifFalseType->isLValue = SEM::Type::RVALUE;
			}
			
			SEM::Value* castIfTrue = CastValueToType(ifTrue, ifFalseType);
				
			if(castIfTrue != NULL) {
				return SEM::Value::Ternary(boolValue, castIfTrue, ifFalse, ifFalseType);
			}
			
			SEM::Value* castIfFalse = CastValueToType(ifFalse, ifTrueType);
				
			if(castIfFalse != NULL) {
				return SEM::Value::Ternary(boolValue, ifTrue, castIfFalse, ifTrueType);
			}
			
			printf("Semantic Analysis Error: Can't cast result expressions to matching type in ternary operator.\n");
			return NULL;
		}
		case AST::Value::CAST: {
			SEM::Type* type = ConvertType(context, value->cast.targetType, SEM::Type::RVALUE);
			SEM::Value* val = ConvertValue(context, value->cast.value);
			
			if(type == NULL || val == NULL) {
				return NULL;
			}
			
			if(CanDoExplicitCast(val->type, type) == 0) {
				printf("Semantic Analysis Error: Can't perform explicit cast.\n");
				return NULL;
			}
			
			return SEM::Value::Cast(type, val);
		}
		case AST::Value::INTERNALCONSTRUCT: {
			SEM::TypeInstance * thisTypeInstance = context.getThisTypeInstance();
			assert(false && "Not yet implemented");
			return NULL;
		}
		case AST::Value::MEMBERACCESS: {
			const std::string memberName = value->memberAccess.memberName;
		
			SEM::Value * object = ConvertValue(context, value->memberAccess.object);
			if(object == NULL) return NULL;
			
			SEM::Type * objectType = object->type;
			if(objectType->typeEnum != SEM::Type::NAMED){
				printf("Semantic Analysis Error: Can't access member of non-object type.\n");
				return NULL;
			}
		
			SEM::TypeInstance * typeInstance = objectType->namedType.typeInstance;
			assert(typeInstance != NULL);
			
			if(typeInstance->typeEnum == SEM::TypeInstance::STRUCT){
				// Look for struct variables.
				Optional<SEM::Var *> varResult = typeInstance->variables.tryGet(memberName);
				if(varResult.hasValue()){
					SEM::Var * var = varResult.getValue();
					if(objectType->isLValue){
						return SEM::Value::MemberAccess(object, var->id, var->type);
					}else{
						// If the struct type is an R-value, then the member must
						// also be (preventing assignments to R-value members).
						SEM::Type * memberType = new SEM::Type(*(var->type));
						memberType->isLValue = false;
						return SEM::Value::MemberAccess(object, var->id, memberType);
					}
				}else{
					printf("Semantic Analysis Error: Can't access struct member '%s' in type '%s'.\n", memberName.c_str(), typeInstance->name.toString().c_str());
					return NULL;
				}
			}else{
				// Look for class methods.
				Optional<SEM::Function *> functionResult = typeInstance->functions.tryGet(memberName);
				
				if(functionResult.hasValue()){
					SEM::Function * function = functionResult.getValue();
					
					if(!function->isMethod){
						printf("Semantic Analysis Error: Cannot call static function '%s' in type '%s'.\n",
							function->name.last().c_str(), typeInstance->name.toString().c_str());
						return NULL;
					}
					
					SEM::Type * methodType = SEM::Type::Method(SEM::Type::MUTABLE, SEM::Type::RVALUE, typeInstance, function->type);
					
					return SEM::Value::MethodObject(function, object, methodType);
				}else{
					printf("Semantic Analysis Error: Can't find class method '%s' in type '%s'.\n", memberName.c_str(), typeInstance->name.toString().c_str());
					return NULL;
				}
			}
			
			assert(false && "Invalid switch fallthrough in ConvertValue for member access");
			return NULL;
		}
		case AST::Value::FUNCTIONCALL: {
			assert(value->functionCall.functionValue != NULL && "Cannot call NULL function value");
			SEM::Value* functionValue = ConvertValue(context, value->functionCall.functionValue);
			
			if(functionValue == NULL) {
				return NULL;
			}
			
			switch(functionValue->type->typeEnum){
				case SEM::Type::FUNCTION:
				{
					const std::vector<SEM::Type*>& typeList = functionValue->type->functionType.parameterTypes;
					const std::vector<AST::Value*>& astValueList = value->functionCall.parameters;
					
					if(functionValue->type->functionType.isVarArg){
						if(astValueList.size() < typeList.size()) {
							printf("Semantic Analysis Error: Var Arg Function [%s] called with %lu number of parameters; expected at least %lu.\n",
								functionValue->toString().c_str(), astValueList.size(), typeList.size());
							return NULL;
						}
					}else{
						if(astValueList.size() != typeList.size()) {
							printf("Semantic Analysis Error: Function [%s] called with %lu number of parameters; expected %lu.\n",
								functionValue->toString().c_str(), astValueList.size(), typeList.size());
							return NULL;
						}
					}
					
					assert(astValueList.size() >= typeList.size());
					
					std::vector<SEM::Value*> semValueList;
					
					for(std::size_t i = 0; i < astValueList.size(); i++){
						SEM::Value* value = ConvertValue(context, astValueList.at(i));
						
						if(value == NULL) return NULL;
						
						SEM::Value* param = (i < typeList.size()) ? CastValueToType(value, typeList.at(i)) : value;
						
						if(param == NULL) return NULL;
						
						semValueList.push_back(param);
					}
					
					return SEM::Value::FunctionCall(functionValue, semValueList, functionValue->type->functionType.returnType);
				}
				case SEM::Type::METHOD:
				{
					SEM::Type * functionType = functionValue->type->methodType.functionType;
					
					const std::vector<SEM::Type*>& typeList = functionType->functionType.parameterTypes;
					const std::vector<AST::Value*>& astValueList = value->functionCall.parameters;
					
					// First type must be the object type.
					assert(!typeList.empty() && "Method functions must have at least one argument for the 'this' pointer");
					
					assert(!functionType->functionType.isVarArg && "Methods cannot be var args");
					
					if(typeList.size() != (astValueList.size() + 1)) {
						printf("Semantic Analysis Error: Method called with %lu number of parameters; expected %lu.\n", astValueList.size(), typeList.size());
						return NULL;
					}
					
					std::vector<SEM::Value*> semValueList;
					
					for(std::size_t i = 0; i < astValueList.size(); i++){
						SEM::Value* value = ConvertValue(context, astValueList.at(i));
						
						if(value == NULL) return NULL;
						
						SEM::Value* param = CastValueToType(value, typeList.at(i + 1));
						
						if(param == NULL) return NULL;
						
						semValueList.push_back(param);
					}
					
					return SEM::Value::MethodCall(functionValue, semValueList, functionType->functionType.returnType);
				}
				default:
				{
					printf("Semantic Analysis Error: Can't call type that isn't a function or a method.\n");
					return NULL;
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


