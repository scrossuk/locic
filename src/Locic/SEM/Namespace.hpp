#ifndef LOCIC_SEM_NAMESPACE_HPP
#define LOCIC_SEM_NAMESPACE_HPP

#include <string>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/Optional.hpp>
#include <Locic/SEM/Function.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace SEM{

	struct NamespaceNode;

	struct Namespace{
		Locic::Name name;
		Locic::StringMap<NamespaceNode *> children;
		
		inline Namespace(const Locic::Name& n)
			: name(n){ }
		
		inline NamespaceNode * lookup(const Locic::Name& targetName) const{
			assert(targetName.isAbsolute() && !targetName.empty());
			if(name.isPrefixOf(targetName)){
				// Get the part of the name that's of interest.
				const std::string namePart = targetName.at(name.size());
				
				Locic::Optional<NamespaceNode*> nodeResult = children.tryGet(namePart);
				if(nodeResult.hasValue()){
					return nodeResult.getValue();
				}
			}
			return NULL;
		}
	};

	struct NamespaceNode{
		enum TypeEnum{
			FUNCTION = 0,
			NAMESPACE,
			TYPEINSTANCE
		} typeEnum;
		
		union{
			Function * function;
			Namespace * nameSpace;
			TypeInstance * typeInstance;
		};
		
		inline NamespaceNode(TypeEnum e)
			: typeEnum(e){ }
		
		inline static NamespaceNode * Function(Function * function){
			NamespaceNode * node = new NamespaceNode(FUNCTION);
			node->function = function;
			return node;
		}
		
		inline static NamespaceNode * Namespace(Namespace * nameSpace){
			NamespaceNode * node = new NamespaceNode(NAMESPACE);
			node->nameSpace = nameSpace;
			return node;
		}
		
		inline static NamespaceNode * TypeInstance(TypeInstance * typeInstance){
			NamespaceNode * node = new NamespaceNode(TYPEINSTANCE);
			node->typeInstance = typeInstance;
			return node;
		}
		
		inline Locic::Name getName() const{
			switch(typeEnum){
				case FUNCTION:
					return function->name;
				case NAMESPACE:
					return nameSpace->name;
				case TYPEINSTANCE:
					return typeInstance->name;
				default:
					assert(false);
					return Locic::Name::Absolute();
			}
		}
		
		inline struct Function * getFunction(){
			return (typeEnum == FUNCTION) ? function : NULL;
		}
		
		inline struct Namespace * getNamespace(){
			return (typeEnum == NAMESPACE) ? nameSpace : NULL;
		}
		
		inline struct TypeInstance * getTypeInstance(){
			return (typeEnum == TYPEINSTANCE) ? typeInstance : NULL;
		}
	};

}

#endif
