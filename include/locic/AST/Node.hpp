#ifndef LOCIC_AST_NODE_HPP
#define LOCIC_AST_NODE_HPP

#include <locic/Debug/SourceLocation.hpp>

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
				
				Debug::SourceLocation location() const {
					return location_;
				}
				
				void setLocation(const Debug::SourceLocation argLocation) {
					location_ = argLocation;
				}
				
				bool isNull() const {
					return contents_ == NULL;
				}
				
				NodeType* get() const {
					return contents_;
				}
				
				NodeType* operator->() const {
					assert(!isNull());
					return contents_;
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
				NodeType* contents_;
				
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
