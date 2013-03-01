#include <string>
#include <vector>

#include <Locic/String.hpp>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace Locic {

	namespace SEM {
	
		const std::vector<Type*> Type::NO_TEMPLATE_ARGS = std::vector<Type*>();
		
		Type* Type::createTransitiveConstType() const {
			if(isPointer()) {
				return Type::Pointer(CONST, isLValue(),
						getReferenceTarget()->createTransitiveConstType());
			} else if(isReference()) {
				return Type::Reference(isLValue(),
						getReferenceTarget()->createTransitiveConstType());
			} else if(isObject()) {
				std::vector<Type*> constArguments;
				const std::vector<Type*>& templateArgs = templateArguments();
				for(size_t i = 0; i < templateArgs.size(); i++) {
					constArguments.push_back(
						templateArgs.at(i)->createTransitiveConstType());
				}
				return Type::Object(CONST, isLValue(), getObjectType(), constArguments);
			} else {
				return createConstType();
			}
		}
		
		bool Type::supportsImplicitCopy() const {
			switch(kind()) {
				case VOID:
				case NULLT:
				case POINTER:
				case REFERENCE:
				case FUNCTION:
				case METHOD:
					// Pointer, function and method types can be copied implicitly.
					return true;
				case OBJECT:
					// Named types must have a method for implicit copying.
					return getObjectType()->supportsImplicitCopy();
				default:
					assert(false && "Unknown SEM type enum");
					return false;
			}
		}
		
		Type* Type::getImplicitCopyType() const {
			switch(kind()) {
				case VOID:
				case NULLT:
				case POINTER:
				case REFERENCE:
				case FUNCTION:
				case METHOD: {
					// Built in types retain their 'constness' in copying.
					// However, all except pointers are const types
					// anyway, so this essentially has no effect for them.
					return createRValueType();
				}
				case OBJECT:
					// Object types may or may not retain 'constness'.
					return getObjectType()->getImplicitCopyType();
				default:
					assert(false && "Unknown SEM type enum");
					return false;
			}
		}
		
		std::string Type::basicToString() const {
			switch(kind()) {
				case VOID: {
					return "VoidType()";
				}
				case NULLT: {
					return "NullType()";
				}
				case OBJECT:
					return makeString("ObjectType(%s)",
							getObjectType()->name().toString().c_str());
				case POINTER:
					return makeString("PointerType(%s)",
							getPointerTarget()->toString().c_str());
				case REFERENCE:
					return makeString("ReferenceType(%s)",
							getReferenceTarget()->toString().c_str());
				case FUNCTION:
					return makeString("FunctionType(return: %s, args: %s, isVarArg: %s)",
							getFunctionReturnType()->toString().c_str(),
							makeArrayString(getFunctionParameterTypes()).c_str(),
							isFunctionVarArg() ? "Yes" : "No");
				case METHOD:
					return makeString("MethodType(object: %s, function: %s)",
							getMethodObjectType()->toString().c_str(),
							getMethodFunctionType()->toString().c_str());
				default:
					return "[UNKNOWN TYPE]";
			}
		}
		
		std::string Type::constToString() const {
			if(isMutable()) {
				return basicToString();
			} else {
				return makeString("Const(%s)",
						basicToString().c_str());
			}
		}
		
		std::string Type::toString() const {
			if(isLValue()) {
				return makeString("LValue(%s)",
						constToString().c_str());
			} else {
				return constToString();
			}
		}
		
		bool Type::operator==(const Type& type) const {
			if(this == &type) {
				return true;
			}
			
			if(kind() != type.kind()
					|| isMutable() != type.isMutable()
					|| isLValue() != type.isLValue()) {
				return false;
			}
			
			switch(kind_) {
				case SEM::Type::VOID:
				case SEM::Type::NULLT: {
					return true;
				}
				case SEM::Type::OBJECT: {
					return getObjectType() == type.getObjectType();
				}
				case SEM::Type::POINTER: {
					return *(getPointerTarget()) == *(type.getPointerTarget());
				}
				case SEM::Type::REFERENCE: {
					return *(getReferenceTarget()) == *(type.getReferenceTarget());
				}
				case SEM::Type::FUNCTION: {
					const std::vector<Type*>& firstList = getFunctionParameterTypes();
					const std::vector<Type*>& secondList = type.getFunctionParameterTypes();
					
					if(firstList.size() != secondList.size()) {
						return false;
					}
					
					for(std::size_t i = 0; i < firstList.size(); i++) {
						if(*(firstList.at(i)) != *(secondList.at(i))) {
							return false;
						}
					}
					
					return *(getFunctionReturnType()) == *(type.getFunctionReturnType())
						   && isFunctionVarArg() == type.isFunctionVarArg();
				}
				case SEM::Type::METHOD: {
					return getMethodObjectType() != type.getMethodObjectType()
						   && *(getMethodFunctionType()) == *(type.getMethodFunctionType());
				}
				default:
					return false;
			}
		}
		
	}
	
}

