#include <string>
#include <vector>
#include <locic/AST/Node.hpp>
#include <locic/AST/Symbol.hpp>
#include <locic/AST/Type.hpp>

namespace locic {

	namespace AST {
	
		Type::Type() : typeEnum(static_cast<TypeEnum>(-1)) { }
		
		Type::Type(TypeEnum e) : typeEnum(e) { }
		
		Type* Type::Auto() {
			return new Type(AUTO);
		}
		
		Type* Type::Void() {
			return new Type(VOID);
		}
		
		Type* Type::Bool() {
			return new Type(BOOL);
		}
		
		Type* Type::Const(Node<Type> targetType) {
			Type* type = new Type(CONST);
			type->constType.targetType = std::move(targetType);
			return type;
		}
		
		Type* Type::ConstPredicate(Node<Predicate> predicate, Node<Type> targetType) {
			Type* type = new Type(CONSTPREDICATE);
			type->constPredicateType.predicate = std::move(predicate);
			type->constPredicateType.targetType = std::move(targetType);
			return type;
		}
		
		Type* Type::NoTag(Node<Type> targetType) {
			Type* type = new Type(NOTAG);
			type->noTagType.targetType = std::move(targetType);
			return type;
		}
		
		Type* Type::Lval(Node<Type> targetType, Node<Type> lvalType) {
			Type* type = new Type(LVAL);
			type->lvalType.targetType = std::move(targetType);
			type->lvalType.lvalType = std::move(lvalType);
			return type;
		}
		
		Type* Type::Ref(Node<Type> targetType, Node<Type> refType) {
			Type* type = new Type(REF);
			type->refType.targetType = std::move(targetType);
			type->refType.refType = std::move(refType);
			return type;
		}
		
		Type* Type::StaticRef(Node<Type> targetType, Node<Type> refType) {
			Type* type = new Type(STATICREF);
			type->staticRefType.targetType = std::move(targetType);
			type->staticRefType.refType = std::move(refType);
			return type;
		}
		
		Type* Type::Integer(SignedModifier signedModifier, const String& name) {
			Type* type = new Type(INTEGER);
			type->integerType.signedModifier = signedModifier;
			type->integerType.name = name;
			return type;
		}
		
		Type* Type::Float(const String& name) {
			Type* type = new Type(FLOAT);
			type->floatType.name = name;
			return type;
		}
		
		Type* Type::Primitive(const PrimitiveID primitiveID) {
			Type* type = new Type(PRIMITIVE);
			type->primitiveType.primitiveID = primitiveID;
			return type;
		}
		
		Type* Type::Object(Node<Symbol> symbol) {
			Type* type = new Type(OBJECT);
			type->objectType.symbol = std::move(symbol);
			return type;
		}
		
		Type* Type::Reference(Node<Type> targetType) {
			Type* type = new Type(REFERENCE);
			type->referenceType.targetType = std::move(targetType);
			return type;
		}
		
		Type* Type::Pointer(Node<Type> targetType) {
			Type* type = new Type(POINTER);
			type->pointerType.targetType = std::move(targetType);
			return type;
		}
		
		Type* Type::StaticArray(Node<Type> targetType, Node<Value> arraySize) {
			Type* type = new Type(STATICARRAY);
			type->staticArrayType.targetType = std::move(targetType);
			type->staticArrayType.arraySize = std::move(arraySize);
			return type;
		}
		
		Type* Type::Function(Node<Type> returnType, Node<TypeList> parameterTypes) {
			Type* type = new Type(FUNCTION);
			type->functionType.isVarArg = false;
			type->functionType.returnType = std::move(returnType);
			type->functionType.parameterTypes = std::move(parameterTypes);
			return type;
		}
		
		Type* Type::VarArgFunction(Node<Type> returnType, Node<TypeList> parameterTypes) {
			Type* type = new Type(FUNCTION);
			type->functionType.isVarArg = true;
			type->functionType.returnType = std::move(returnType);
			type->functionType.parameterTypes = std::move(parameterTypes);
			return type;
		}
		
		Type::~Type() { }
		
		std::string Type::toString() const {
			switch(typeEnum) {
				case AUTO:
					return "auto";
					
				case CONST:
					return std::string("const ") + getConstTarget()->toString();
					
				case CONSTPREDICATE:
					return std::string("const<[TODO]> ") + getConstPredicateTarget()->toString();
					
				case NOTAG:
					return std::string("notag(") + getNoTagTarget()->toString() + ")";
					
				case LVAL:
					return std::string("lval <") + getLvalTarget()->toString() + "> " + getLvalType()->toString();
					
				case REF:
					return std::string("ref <") + getRefTarget()->toString() + "> " + getRefType()->toString();
					
				case STATICREF:
					return std::string("staticref <") + getStaticRefTarget()->toString() + "> " + getStaticRefType()->toString();
					
				case VOID:
					return "void";
					
				case BOOL:
					return "bool";
				
				case INTEGER: {
					const auto signedString = (integerSignedModifier() == SIGNED ?
						"signed" :
							(integerSignedModifier() == UNSIGNED ?
								"unsigned" :
								""
							)
						);
					return std::string("[integer type: ") + signedString + " " + integerName().asStdString() + std::string("]");	
				}
				
				case FLOAT: {
					return std::string("[float type: ") + floatName().asStdString() + std::string("]");	
				}
				
				case PRIMITIVE: {
					return std::string("[primitive type: ") + primitiveID().toString() + std::string("]");
				}
				
				case OBJECT:
					return std::string("[object type: ") + objectType.symbol->toString() + std::string("]");
					
				case REFERENCE:
					return getReferenceTarget()->toString() + "&";
					
				case POINTER:
					return getPointerTarget()->toString() + "*";
					
				case STATICARRAY:
					return getStaticArrayTarget()->toString() + "[" + getArraySize()->toString() + "]";
					
				case FUNCTION: {
					std::string str;
					str += "(";
					str += functionType.returnType->toString();
					str += ")(";
					
					for(size_t i = 0; i < functionType.parameterTypes->size(); i++) {
						if(i != 0) {
							str += ", ";
						}
						
						str += functionType.parameterTypes->at(i)->toString();
					}
					
					str += ")";
					return str;
				}
			}
			
			std::terminate();
		}
		
	}
	
}

