#ifndef LOCIC_AST_NODE_HPP
#define LOCIC_AST_NODE_HPP

#include <locic/SourceLocation.hpp>

namespace locic {

	namespace AST {
	
		inline SourceLocation NullLocation() {
			const auto nullPosition = SourcePosition(0, 0);
			return SourceLocation("<NULL>", SourceRange(nullPosition, nullPosition));
		}
		
		template <typename NodeType>
		class Node {
			public:
				inline Node()
					: location_(NullLocation()), contents_(NULL) { }
					
				inline Node(const SourceLocation& pLocation, NodeType* pContents)
					: location_(pLocation), contents_(pContents) { }
					
				inline SourceLocation location() const {
					return location_;
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
				SourceLocation location_;
				NodeType* contents_;
				
		};
		
		template <typename T>
		inline Node<T> makeNode(const SourceLocation& pLocation, T* pContents) {
			return Node<T>(pLocation, pContents);
		}
		
		template <typename T>
		inline Node<T> makeDefaultNode() {
			return Node<T>(NullLocation(), new T());
		}
		
	}
	
}

#endif
