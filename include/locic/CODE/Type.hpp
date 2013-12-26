#ifndef LOCIC_CODE_TYPE_HPP
#define LOCIC_CODE_TYPE_HPP

#include <string>
#include <vector>

#include <locic/String.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {
	
	namespace CODE {
		
		enum PrimitiveType{
			INT8,
			INT16,
			INT32,
			INT64,
			INT128,
			SIZET,
			CHAR,
			SHORT,
			INT,
			LONG,
			LONGLONG,
			FLOAT,
			DOUBLE
		};
	
		struct Type {
			enum TypeEnum {
				VOID,
				PRIMITIVE,
				STRUCT,
				POINTER,
				REFERENCE,
				FUNCTION,
				METHOD
			} typeEnum;
			
			bool isMutable;
			
			static const bool MUTABLE = true;
			static const bool CONST = false;
			
			bool isLValue;
			
			static const bool LVALUE = true;
			static const bool RVALUE = false;
			
			struct {
				TypeInstance* typeInstance;
			} namedType;
			
			struct {
				// Type that is being pointed to.
				Type* targetType;
			} pointerType;
			
			struct {
				// Type that is being referred to.
				Type* targetType;
			} referenceType;
			
			struct FunctionType {
				bool isVarArg;
				Type* returnType;
				std::vector<Type*> parameterTypes;
			} functionType;
			
			struct {
				Type* objectType;
				Type* functionType;
			} methodType;
			
			inline Type()
				: typeEnum(VOID),
				  isMutable(MUTABLE),
				  isLValue(RVALUE) { }
				  
			inline Type(TypeEnum e, bool m, bool l)
				: typeEnum(e),
				  isMutable(m),
				  isLValue(l) { }
				  
			inline static Type* Void(bool isMutable) {
				return new Type(VOID, isMutable, RVALUE);
			}
			
			inline static Type* Null(bool isMutable) {
				return new Type(NULLT, isMutable, RVALUE);
			}
			
			inline static Type* Named(bool isMutable, bool isLValue, TypeInstance* typeInstance) {
				Type* type = new Type(NAMED, isMutable, isLValue);
				type->namedType.typeInstance = typeInstance;
				return type;
			}
			
			inline static Type* Pointer(bool isMutable, bool isLValue, Type* targetType) {
				assert(targetType->isLValue);
				Type* type = new Type(POINTER, isMutable, isLValue);
				type->pointerType.targetType = targetType;
				return type;
			}
			
			inline static Type* Reference(bool isLValue, Type* targetType){
				assert(targetType->isLValue);
				
				// References are a 'const type', meaning they are always const.
				const bool isMutable = false;
				
				Type* type = new Type(REFERENCE, isMutable, isLValue);
				type->referenceType.targetType = targetType;
				return type;
			}
			
			inline static Type* Function(bool isMutable, bool isLValue, bool isVarArg, Type* returnType, const std::vector<Type*>& parameterTypes) {
				Type* type = new Type(FUNCTION, isMutable, isLValue);
				type->functionType.isVarArg = isVarArg;
				type->functionType.returnType = returnType;
				type->functionType.parameterTypes = parameterTypes;
				return type;
			}
			
			inline static Type* Method(bool isMutable, bool isLValue, Type* objectType, Type* functionType) {
				assert(objectType->isObjectType());
				Type* type = new Type(METHOD, isMutable, isLValue);
				type->methodType.objectType = objectType;
				type->methodType.functionType = functionType;
				return type;
			}
			
			inline Type* applyTransitiveConst() {
				Type* t = this;
				
				while(true) {
					t->isMutable = false;
					
					if(t->typeEnum == POINTER) {
						t = t->pointerType.targetType;
					} else {
						break;
					}
				}
				
				return this;
			}
			
			inline bool isVoid() const {
				return typeEnum == VOID;
			}
			
			inline bool isNull() const {
				return typeEnum == NULLT;
			}
			
			inline bool isPointer() const {
				return typeEnum == POINTER;
			}
			
			inline bool isReference() const {
				return typeEnum == REFERENCE;
			}
			
			inline bool isFunction() const {
				return typeEnum == FUNCTION;
			}
			
			inline bool isMethod() const {
				return typeEnum == METHOD;
			}
			
			inline SEM::Type* getPointerTarget() const {
				assert(isPointer() && "Cannot get target type of non-pointer type");
				return pointerType.targetType;
			}
			
			inline SEM::Type* getReferenceTarget() const {
				assert(isReference() && "Cannot get target type of non-reference type");
				return referenceType.targetType;
			}
			
			inline bool isObjectType() const {
				return typeEnum == NAMED;
			}
			
			inline SEM::TypeInstance* getObjectType() const {
				assert(isObjectType() && "Cannot get object type, since type is not an object type");
				return namedType.typeInstance;
			}
			
			inline bool isTypeInstance(const TypeInstance* typeInstance) const {
				if(typeEnum != NAMED) return false;
				
				return namedType.typeInstance == typeInstance;
			}
			
			inline bool isClass() const {
				if(typeEnum != NAMED) return false;
				
				return namedType.typeInstance->isClass();
			}
			
			inline bool isInterface() const {
				if(typeEnum != NAMED) return false;
				
				return namedType.typeInstance->isInterface();
			}
			
			inline bool supportsImplicitCopy() const {
				switch(typeEnum) {
					case VOID:
					case NULLT:
					case POINTER:
					case REFERENCE:
					case FUNCTION:
					case METHOD:
						// Pointer, function and method types can be copied implicitly.
						return true;
					case NAMED:
						// Named types must have a method for implicit copying.
						return namedType.typeInstance->supportsImplicitCopy();
					default:
						assert(false && "Unknown SEM type enum");
						return false;
				}
			}
			
			inline std::string basicToString() const {
				switch(typeEnum) {
					case VOID: {
						return "VoidType()";
					}
					case NULLT: {
						return "NullType()";
					}
					case NAMED:
						return makeString("ObjectType(%s)",
										  namedType.typeInstance->name.toString().c_str());
					case POINTER:
						return makeString("PointerType(%s)",
										  pointerType.targetType->toString().c_str());
					case REFERENCE:
						return makeString("ReferenceType(%s)",
										  referenceType.targetType->toString().c_str());
					case FUNCTION:
						return makeString("FunctionType(return: %s, args: %s, isVarArg: %s)",
										  functionType.returnType->toString().c_str(),
										  makeArrayString(functionType.parameterTypes).c_str(),
										  functionType.isVarArg ? "Yes" : "No");
					case METHOD:
						return makeString("MethodType(object: %s, function: %s)",
									  methodType.objectType->toString().c_str(),
									  methodType.functionType->toString().c_str());
					default:
						return "[UNKNOWN TYPE]";
				}
			}
			
			inline std::string constToString() const {
				if(isMutable) {
					return basicToString();
				} else {
					return makeString("Const(%s)",
									  basicToString().c_str());
				}
			}
			
			inline std::string toString() const {
				if(isLValue) {
					return makeString("LValue(%s)",
									  constToString().c_str());
				} else {
					return constToString();
				}
			}
			
			inline bool operator==(const Type& type) const{
				if(this == &type){
					return true;
				}
				
				if(typeEnum != type.typeEnum
					|| isMutable != type.isMutable
					|| isLValue != type.isLValue) {
					return false;
				}
			
				switch(typeEnum) {
					case SEM::Type::VOID:
					case SEM::Type::NULLT: {
						return true;
					}
					case SEM::Type::NAMED: {
						return getObjectType() == type.getObjectType();
					}
					case SEM::Type::POINTER: {
						return *(pointerType.targetType) == *(type.pointerType.targetType);
					}
					case SEM::Type::REFERENCE: {
						return *(referenceType.targetType) == *(type.referenceType.targetType);
					}
					case SEM::Type::FUNCTION: {
						const std::vector<SEM::Type*>& firstList = functionType.parameterTypes;
						const std::vector<SEM::Type*>& secondList = type.functionType.parameterTypes;
						
						if(firstList.size() != secondList.size()) {
							return false;
						}
						
						for(std::size_t i = 0; i < firstList.size(); i++){
							if(*(firstList.at(i)) != *(secondList.at(i))) {
								return false;
							}
						}
						
						return *(functionType.returnType) == *(type.functionType.returnType)
							&& functionType.isVarArg == type.functionType.isVarArg;
					}
					case SEM::Type::METHOD: {
						return methodType.objectType != type.methodType.objectType 
							&& *(methodType.functionType) == *(type.methodType.functionType);
					}
					default:
						return false;
				}
			}
			
			inline bool operator!=(const Type& type) const {
				return !(*this == type);
			}
			
		};
		
	}
	
}

#endif
