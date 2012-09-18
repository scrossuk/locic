#ifndef LOCIC_SEM_NAMESPACE_HPP
#define LOCIC_SEM_NAMESPACE_HPP

#include <string>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/Optional.hpp>
#include <Locic/SEM/Function.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace SEM{

	struct Namespace;

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
	
	struct Namespace{
		Locic::Name name;
		Locic::StringMap<NamespaceNode *> children;
		
		inline Namespace(const Locic::Name& n)
			: name(n){ }
		
		inline NamespaceNode * lookup(const Locic::Name& relativeName) const{
			Locic::Name absoluteName = name.makeAbsolute(relativeName);
		
			// If the target name matches this namespace, we still don't have the
			// namespace node to return (our parent namespace has that, or if we're
			// root, it doesn't exist).
			if(name.isPrefixOf(absoluteName)){
				// Get the part of the name that's of interest.
				const std::string namePart = absoluteName.at(name.size());
				
				Locic::Optional<NamespaceNode*> nodeResult = children.tryGet(namePart);
				if(nodeResult.hasValue()){
					NamespaceNode * node = nodeResult.getValue();
					if(node->typeEnum == NamespaceNode::NAMESPACE){
						// For namespaces, keep searching.
						return node->nameSpace->lookup(absoluteName);
					}else{
						return node;
					}
				}
			}
			return NULL;
		}
	};

}

#endif
