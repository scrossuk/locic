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
		Locic::StringMap<NamespaceNode> children;
		
		inline Namespace(const Locic::Name& n)
			: name(n){ }
		
		NamespaceNode lookup(const Locic::Name& targetName);
		
	};

	struct NamespaceNode{
		enum TypeEnum{
			NONE = 0,
			FUNCTION,
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
		
		inline static NamespaceNode None(){
			return NamespaceNode(NONE);
		}
		
		inline static NamespaceNode Function(Function * function){
			NamespaceNode node(FUNCTION);
			node.function = function;
			return node;
		}
		
		inline static NamespaceNode Namespace(Namespace * nameSpace){
			NamespaceNode node(NAMESPACE);
			node.nameSpace = nameSpace;
			return node;
		}
		
		inline static NamespaceNode TypeInstance(TypeInstance * typeInstance){
			NamespaceNode node(TYPEINSTANCE);
			node.typeInstance = typeInstance;
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
		
		inline bool isNone() const{
			return typeEnum == NONE;
		}
		
		inline bool isNotNone() const{
			return typeEnum != NONE;
		}
		
		inline bool isFunction() const{
			return typeEnum == FUNCTION;
		}
		
		inline bool isNamespace() const{
			return typeEnum == NAMESPACE;
		}
		
		inline bool isTypeInstance() const{
			return typeEnum == TYPEINSTANCE;
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
		
		inline NamespaceNode lookup(const Locic::Name& name){
			if(typeEnum == NONE) return NamespaceNode::None();
			
			Locic::Name absoluteName = getName().makeAbsolute(name);
			
			NamespaceNode node = *this;
			
			while(!node.isNone()){
				if(node.getName() == absoluteName){
					return node;
				}
				switch(node.typeEnum){
					case NAMESPACE:
					{
						node = node.getNamespace()->lookup(absoluteName);
						break;
					}
					case TYPEINSTANCE:
					{
						node = node.getTypeInstance()->lookup(absoluteName);
						break;
					}
					case FUNCTION:
					{
						// Functions have no children.
						return NamespaceNode::None();
					}
					default:
					{
						assert(false);
						return NamespaceNode::None();
					}
				}
			}
			
			return NamespaceNode::None();
		}
	};

}

#endif
