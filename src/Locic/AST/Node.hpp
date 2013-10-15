#ifndef LOCIC_AST_NODE_HPP
#define LOCIC_AST_NODE_HPP

#include <Locic/SourceLocation.hpp>

namespace AST {

	inline Locic::SourceLocation NullLocation() {
		const auto nullPosition = Locic::SourcePosition(0, 0);
		return Locic::SourceLocation("<NULL>", Locic::SourceRange(nullPosition, nullPosition));
	}
	
	template <typename NodeType>
	class Node {
		public:
			inline Node()
				: location_(NullLocation()), contents_(NULL) { }
			
			inline Node(const Locic::SourceLocation& pLocation, NodeType* pContents)
				: location_(pLocation), contents_(pContents) { }
			
			inline Locic::SourceLocation location() const {
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
		
		private:
			Locic::SourceLocation location_;
			NodeType* contents_;
			
	};
	
	template <typename T>
	inline Node<T> makeNode(const Locic::SourceLocation& pLocation, T* pContents) {
		return Node<T>(pLocation, pContents);
	}
	
	template <typename T>
	inline Node<T> makeDefaultNode() {
		return Node<T>(NullLocation(), new T());
	}
	
}

#endif
