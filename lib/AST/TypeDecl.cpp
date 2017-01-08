#include <string>
#include <vector>
#include <locic/AST/Node.hpp>
#include <locic/AST/Symbol.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/ValueDecl.hpp>

namespace locic {
	
	namespace AST {
		
		TypeDecl::TypeDecl() : typeEnum(static_cast<TypeEnum>(-1)),
		resolvedType_(nullptr) { }
		
		TypeDecl::TypeDecl(TypeEnum e) : typeEnum(e),
		resolvedType_(nullptr) { }
		
		TypeDecl* TypeDecl::Auto() {
			return new TypeDecl(AUTO);
		}
		
		TypeDecl* TypeDecl::Void() {
			return new TypeDecl(VOID);
		}
		
		TypeDecl* TypeDecl::Bool() {
			return new TypeDecl(BOOL);
		}
		
		TypeDecl* TypeDecl::Const(Node<TypeDecl> targetType) {
			TypeDecl* type = new TypeDecl(CONST);
			type->constType.targetType = std::move(targetType);
			return type;
		}
		
		TypeDecl* TypeDecl::ConstPredicate(Node<PredicateDecl> predicate, Node<TypeDecl> targetType) {
			TypeDecl* type = new TypeDecl(CONSTPREDICATE);
			type->constPredicateType.predicate = std::move(predicate);
			type->constPredicateType.targetType = std::move(targetType);
			return type;
		}
		
		TypeDecl* TypeDecl::NoTag(Node<TypeDecl> targetType) {
			TypeDecl* type = new TypeDecl(NOTAG);
			type->noTagType.targetType = std::move(targetType);
			return type;
		}
		
		TypeDecl* TypeDecl::Integer(SignedModifier signedModifier, const String& name) {
			TypeDecl* type = new TypeDecl(INTEGER);
			type->integerType.signedModifier = signedModifier;
			type->integerType.name = name;
			return type;
		}
		
		TypeDecl* TypeDecl::Float(const String& name) {
			TypeDecl* type = new TypeDecl(FLOAT);
			type->floatType.name = name;
			return type;
		}
		
		TypeDecl* TypeDecl::Primitive(const PrimitiveID primitiveID) {
			TypeDecl* type = new TypeDecl(PRIMITIVE);
			type->primitiveType.primitiveID = primitiveID;
			return type;
		}
		
		TypeDecl* TypeDecl::Object(Node<Symbol> symbol) {
			TypeDecl* type = new TypeDecl(OBJECT);
			type->objectType.symbol = std::move(symbol);
			return type;
		}
		
		TypeDecl* TypeDecl::Reference(Node<TypeDecl> targetType) {
			TypeDecl* type = new TypeDecl(REFERENCE);
			type->referenceType.targetType = std::move(targetType);
			return type;
		}
		
		TypeDecl* TypeDecl::Pointer(Node<TypeDecl> targetType) {
			TypeDecl* type = new TypeDecl(POINTER);
			type->pointerType.targetType = std::move(targetType);
			return type;
		}
		
		TypeDecl* TypeDecl::StaticArray(Node<TypeDecl> targetType, Node<ValueDecl> arraySize) {
			TypeDecl* type = new TypeDecl(STATICARRAY);
			type->staticArrayType.targetType = std::move(targetType);
			type->staticArrayType.arraySize = std::move(arraySize);
			return type;
		}
		
		TypeDecl* TypeDecl::Function(Node<TypeDecl> returnType, Node<TypeDeclList> parameterTypes) {
			TypeDecl* type = new TypeDecl(FUNCTION);
			type->functionType.isVarArg = false;
			type->functionType.returnType = std::move(returnType);
			type->functionType.parameterTypes = std::move(parameterTypes);
			return type;
		}
		
		TypeDecl* TypeDecl::VarArgFunction(Node<TypeDecl> returnType, Node<TypeDeclList> parameterTypes) {
			TypeDecl* type = new TypeDecl(FUNCTION);
			type->functionType.isVarArg = true;
			type->functionType.returnType = std::move(returnType);
			type->functionType.parameterTypes = std::move(parameterTypes);
			return type;
		}
		
		TypeDecl::~TypeDecl() { }
		
		const Type* TypeDecl::resolvedType() const {
			return resolvedType_;
		}
		
		void TypeDecl::setResolvedType(const Type* type) {
			resolvedType_ = type;
		}
		
		std::string TypeDecl::toString() const {
			switch(typeEnum) {
				case AUTO:
					return "auto";
					
				case CONST:
					return std::string("const ") + getConstTarget()->toString();
					
				case CONSTPREDICATE:
					return std::string("const<[TODO]> ") + getConstPredicateTarget()->toString();
					
				case NOTAG:
					return std::string("notag(") + getNoTagTarget()->toString() + ")";
					
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
			
			locic_unreachable("Unknown type kind.");
		}
		
	}
	
}

