#include <cstdio>
#include <string>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/SEM/Namespace.hpp>
#include <Locic/SEM/TypeInstance.hpp>
#include <Locic/SEM/Var.hpp>

namespace SEM{

	NamespaceNode TypeInstance::lookup(const Locic::Name& targetName){
		assert(targetName.isAbsolute() && !targetName.empty());
		
		if(name.isPrefixOf(targetName) &&
			targetName.size() == (name.size() + 1)){
			
			const std::string nameEnd = targetName.last();
			
			Locic::Optional<Function *> function = functions.tryGet(nameEnd);
			if(function.hasValue()) return NamespaceNode::Function(function.getValue());
		}
			
		return NamespaceNode::None();
	}
	
	bool TypeInstance::supportsImplicitCopy() const{
		// All primitives can be implicitly copied.
		if(typeEnum == TypeInstance::PRIMITIVE) return true;
	
		const std::string functionName = "implicitCopy";
		Locic::Optional<Function *> result = functions.tryGet(functionName);
		if(!result.hasValue()) return false;
		
		Function * function = result.getValue();
		assert(function != NULL);
		if(!function->isMethod) return false;
		
		Type * type = function->type;
		assert(type->typeEnum == Type::FUNCTION);
		if(type->functionType.isVarArg) return false;
		
		// One argument for the 'this' pointer.
		if(type->functionType.parameterTypes.size() != 1) return false;
		
		return type->functionType.returnType->isTypeInstance(this);
	}

}

