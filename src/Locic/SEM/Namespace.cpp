#include <string>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/Optional.hpp>
#include <Locic/SEM/Function.hpp>
#include <Locic/SEM/Namespace.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace SEM{

	NamespaceNode Namespace::lookup(const Locic::Name& targetName){
		assert(targetName.isAbsolute() && !targetName.empty());
		if(name.isPrefixOf(targetName)){
			// Get the part of the name that's of interest.
			const std::string namePart = targetName.at(name.size());
			
			Locic::Optional<NamespaceNode> nodeResult = children.tryGet(namePart);
			if(nodeResult.hasValue()){
				assert(!nodeResult.getValue().isNone());
				return nodeResult.getValue();
			}
		}
		return NamespaceNode::None();
	}

}

