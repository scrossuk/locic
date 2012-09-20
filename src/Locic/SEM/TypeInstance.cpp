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
			
			Locic::Optional<Function *> constructor = constructors.tryGet(nameEnd);
			if(constructor.hasValue()) return NamespaceNode::Function(constructor.getValue());
			
			Locic::Optional<Function *> method = methods.tryGet(nameEnd);
			if(method.hasValue()) return NamespaceNode::Function(method.getValue());
		}
			
		return NamespaceNode::None();
	}

}

