#include <string>
#include <vector>

#include <Locic/Map.hpp>
#include <Locic/String.hpp>
#include <Locic/SEM/TemplateVar.hpp>
#include <Locic/SEM/Type.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace Locic {

	namespace SEM {
	
		const std::vector<Type*> Type::NO_TEMPLATE_ARGS = std::vector<Type*>();
		
		Map<TemplateVar*, Type*> Type::generateTemplateVarMap() const {
			assert(isObject());
			const std::vector<TemplateVar*>& templateVars = getObjectType()->templateVariables();
			const std::vector<Type*>& templateArgs = templateArguments();
			
			assert(templateVars.size() == templateArgs.size());
			
			Map<TemplateVar*, Type*> templateVarMap;
			for(size_t i = 0; i < templateVars.size(); i++){
				templateVarMap.insert(templateVars.at(i), templateArgs.at(i));
			}
			
			return templateVarMap;
		}
		
		Type* Type::substitute(const Map<TemplateVar*, Type*>& templateVarMap) const {
			switch(kind()) {
				case VOID: {
					return Void();
				}
				case NULLT: {
					return Null();
				}
				case OBJECT: {
					std::vector<Type*> templateArgs;
					for(size_t i = 0; i < templateArguments().size(); i++){
						templateArgs.push_back(templateArguments().at(i)->substitute(templateVarMap));
					}
					return Object(isMutable(), isLValue(), getObjectType(), templateArgs);
				}
				case POINTER: {
					return Pointer(isMutable(), isLValue(), getPointerTarget()->substitute(templateVarMap));
				}
				case REFERENCE: {
					return Reference(isLValue(), getReferenceTarget()->substitute(templateVarMap));
				}
				case FUNCTION: {
					std::vector<Type*> args;
					for(size_t i = 0; i < getFunctionParameterTypes().size(); i++){
						args.push_back(getFunctionParameterTypes().at(i)->substitute(templateVarMap));
					}
					
					Type* returnType = getFunctionReturnType()->substitute(templateVarMap);
					
					return Function(isLValue(), isFunctionVarArg(), returnType, args);
				}
				case METHOD: {
					Type* functionType = getMethodFunctionType()->substitute(templateVarMap);
					
					return Method(isLValue(), functionType);
				}
				case INTERFACEMETHOD: {
					Type* functionType = getInterfaceMethodFunctionType()->substitute(templateVarMap);
					
					return InterfaceMethod(isLValue(), functionType);
				}
				case TEMPLATEVAR: {
					Optional<Type*> substituteType = templateVarMap.tryGet(getTemplateVar());
					if(substituteType.hasValue()){
						return substituteType.getValue()->copyType(isMutable(), isLValue());
					}else{
						return TemplateVarRef(isMutable(), isLValue(), getTemplateVar());
					}
				}
				default:
					assert(false && "Unknown type enum for template var substitution.");
					return NULL;
			}
		}
		
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
				case INTERFACEMETHOD:
					// Pointer, function and method types can be copied implicitly.
					return true;
				case OBJECT:
					// Named types must have a method for implicit copying.
					return getObjectType()->supportsImplicitCopy();
				case TEMPLATEVAR:
					return getTemplateVar()->specType()->supportsImplicitCopy();
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
					return getObjectType()->getImplicitCopyType()->substitute(generateTemplateVarMap());
				case TEMPLATEVAR:
					return getTemplateVar()->specType()->getImplicitCopyType();
				default:
					assert(false && "Unknown SEM type enum");
					return NULL;
			}
		}
		
		std::string Type::nameToString() const {
			switch(kind()) {
				case VOID: {
					return "VoidType()";
				}
				case NULLT: {
					return "NullType()";
				}
				case OBJECT:
					return makeString("ObjectType(typeInstance: %s, templateArguments: %s)",
							getObjectType()->name().toString().c_str(),
							makeNameArrayString(templateArguments()).c_str());
				case POINTER:
					return makeString("PointerType(%s)",
							getPointerTarget()->nameToString().c_str());
				case REFERENCE:
					return makeString("ReferenceType(%s)",
							getReferenceTarget()->nameToString().c_str());
				case FUNCTION:
					return makeString("FunctionType(return: %s, args: %s, isVarArg: %s)",
							getFunctionReturnType()->nameToString().c_str(),
							makeNameArrayString(getFunctionParameterTypes()).c_str(),
							isFunctionVarArg() ? "Yes" : "No");
				case METHOD:
					return makeString("MethodType(functionType: %s)",
							getMethodFunctionType()->nameToString().c_str());
				case TEMPLATEVAR:
					return "TemplateVarType(templateVar: [possible loop])";
				default:
					return "[UNKNOWN TYPE]";
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
					return makeString("ObjectType(typeInstance: %s, templateArguments: %s)",
							getObjectType()->name().toString().c_str(),
							makeArrayString(templateArguments()).c_str());
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
					return makeString("MethodType(functionType: %s)",
							getMethodFunctionType()->toString().c_str());
				case TEMPLATEVAR:
					return makeString("TemplateVarType(templateVar: %s)",
						getTemplateVar()->toString().c_str());
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
					return *(getMethodFunctionType()) == *(type.getMethodFunctionType());
				}
				case SEM::Type::TEMPLATEVAR: {
					return getTemplateVar() == type.getTemplateVar();
				}
				default:
					return false;
			}
		}
		
	}
	
}

