#ifndef LOCIC_AST_NODE_HPP
#define LOCIC_AST_NODE_HPP

#include <locic/Debug/SourceLocation.hpp>

namespace locic {

	namespace AST {
	
		template <typename NodeType>
		class Node {
			public:
				inline Node()
					: location_(Debug::SourceLocation::Null()), contents_(NULL) { }
					
				inline Node(const Debug::SourceLocation& pLocation, NodeType* pContents)
					: location_(pLocation), contents_(pContents) { }
					
				inline Debug::SourceLocation location() const {
					return location_;
				}
				
				void setLocation(const Debug::SourceLocation argLocation) {
					location_ = argLocation;
				}
				
				inline bool isNull() const {
					return contents_ == NULL;
				}
				
				inline NodeType* get() const {
					return contents_;
				}
				
				inline NodeType* operator->() const {
					assert(!isNull());
					return contents_;
				}
				
				inline NodeType& operator*() const {
					assert(!isNull());
					return *contents_;
				}
				
				inline std::string toString() const {
					return makeString("Node[location = %s](%s)",
						location_.toString().c_str(),
						isNull() ? "[NULL]" : contents_->toString().c_str());
				}
				
			private:
				Debug::SourceLocation location_;
				NodeType* contents_;
				
		};
		
		template <typename T>
		inline Node<T> makeNode(const Debug::SourceLocation& pLocation, T* pContents) {
			return Node<T>(pLocation, pContents);
		}
		
		template <typename T>
		inline Node<T> makeDefaultNode() {
			return Node<T>(Debug::SourceLocation::Null(), new T());
		}
		
	}
	
}

#endif
