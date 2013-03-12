#ifndef LOCIC_SEM_NAMESPACE_HPP
#define LOCIC_SEM_NAMESPACE_HPP

#include <string>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/Optional.hpp>
#include <Locic/SEM/Function.hpp>
#include <Locic/SEM/TypeInstance.hpp>

namespace Locic {

	namespace SEM {
	
		struct NamespaceNode;
		
		class Namespace {
			public:
				inline Namespace(const Locic::Name& n)
					: name_(n) { }
					
				inline const Locic::Name& name() const {
					return name_;
				}
				
				inline Locic::StringMap<NamespaceNode>& children() {
					return children_;
				}
				
				inline const Locic::StringMap<NamespaceNode>& children() const {
					return children_;
				}
				
				NamespaceNode lookup(const Locic::Name& targetName) const;
				
			private:
				Locic::Name name_;
				Locic::StringMap<NamespaceNode> children_;
				
		};
		
		class NamespaceNode {
			public:
				enum Kind {
					NONE,
					FUNCTION,
					NAMESPACE,
					TYPEINSTANCE
				};
				
				inline static NamespaceNode None() {
					return NamespaceNode(NONE);
				}
				
				inline static NamespaceNode Function(Function* function) {
					NamespaceNode node(FUNCTION);
					node.function_ = function;
					return node;
				}
				
				inline static NamespaceNode Namespace(Namespace* nameSpace) {
					NamespaceNode node(NAMESPACE);
					node.nameSpace_ = nameSpace;
					return node;
				}
				
				inline static NamespaceNode TypeInstance(TypeInstance* typeInstance) {
					NamespaceNode node(TYPEINSTANCE);
					node.typeInstance_ = typeInstance;
					return node;
				}
				
				inline Kind kind() const {
					return kind_;
				}
				
				inline Locic::Name name() const {
					switch(kind()) {
						case FUNCTION:
							return function_->name();
						case NAMESPACE:
							return nameSpace_->name();
						case TYPEINSTANCE:
							return typeInstance_->name();
						default:
							assert(false && "Can't get name of 'NONE' namespace node.");
							return Locic::Name::Absolute();
					}
				}
				
				inline bool isNone() const {
					return kind() == NONE;
				}
				
				inline bool isNotNone() const {
					return kind() != NONE;
				}
				
				inline bool isFunction() const {
					return kind() == FUNCTION;
				}
				
				inline bool isNamespace() const {
					return kind() == NAMESPACE;
				}
				
				inline bool isTypeInstance() const {
					return kind() == TYPEINSTANCE;
				}
				
				inline class Function* getFunction() const {
						assert(kind() == FUNCTION);
						return function_;
					}
					
				inline class Namespace* getNamespace() const {
						assert(kind() == NAMESPACE);
						return nameSpace_;
					}
					
				inline class TypeInstance* getTypeInstance() const {
						assert(kind() == TYPEINSTANCE);
						return typeInstance_;
					}
					
				inline NamespaceNode lookup(const Locic::Name& targetName) const {
					if(kind() == NONE) return NamespaceNode::None();
					
					const Locic::Name absoluteName = name().makeAbsolute(targetName);
					
					NamespaceNode node = *this;
					
					while(!node.isNone()) {
						if(node.name() == absoluteName) {
							return node;
						}
						
						switch(node.kind()) {
							case NAMESPACE: {
								node = node.getNamespace()->lookup(absoluteName);
								break;
							}
							case TYPEINSTANCE: {
								node = node.getTypeInstance()->lookup(absoluteName);
								break;
							}
							case FUNCTION: {
								// Functions have no children.
								return NamespaceNode::None();
							}
							default: {
								assert(false);
								return NamespaceNode::None();
							}
						}
					}
					
					return NamespaceNode::None();
				}
				
			private:
				inline NamespaceNode(Kind k)
					: kind_(k), nullPtr_(NULL) { }
					
				Kind kind_;
				
				union {
					void* nullPtr_;
					class Function* function_;
					class Namespace* nameSpace_;
					class TypeInstance* typeInstance_;
				};
		};
		
	}
	
}

#endif
