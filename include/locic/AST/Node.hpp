#ifndef LOCIC_AST_NODE_HPP
#define LOCIC_AST_NODE_HPP

#include <memory>

#include <locic/Debug/SourceLocation.hpp>
#include <locic/Support/Copy.hpp>

namespace locic {

	namespace AST {
	
		template <typename NodeType>
		class Node {
			public:
				Node()
				: location_(Debug::SourceLocation::Null()),
				contents_(nullptr) { }
				
				Node(const Debug::SourceLocation& pLocation, NodeType* pContents)
				: location_(pLocation), contents_(pContents) { }
				
				explicit Node(const Node& other)
				: location_(other.location_),
				contents_(other.isNull() ? nullptr : new NodeType(copyObject(*other.contents_))) { }
				
				Node copy() const {
					return Node(*this);
				}
				
				Node(Node&&) = default;
				Node& operator=(Node&&) = default;
				
				Debug::SourceLocation location() const {
					return location_;
				}
				
				void setLocation(const Debug::SourceLocation argLocation) {
					location_ = argLocation;
				}
				
				bool isNull() const {
					return get() == nullptr;
				}
				
				NodeType* get() const {
					return contents_.get();
				}
				
				NodeType* operator->() const {
					assert(!isNull());
					return get();
				}
				
				NodeType& operator*() const {
					assert(!isNull());
					return *contents_;
				}
				
				std::string toString() const {
					return makeString("Node[location = %s](%s)",
						location_.toString().c_str(),
						isNull() ? "[NULL]" : contents_->toString().c_str());
				}
				
			private:
				Debug::SourceLocation location_;
				std::unique_ptr<NodeType> contents_;
				//NodeType* contents_;
				
		};
		
		template <typename T>
		Node<T> makeNode(const Debug::SourceLocation& pLocation, T* pContents) {
			return Node<T>(pLocation, pContents);
		}
		
		template <typename T>
		Node<T> makeDefaultNode() {
			return Node<T>(Debug::SourceLocation::Null(), new T());
		}
		
	}
	
}

#endif
