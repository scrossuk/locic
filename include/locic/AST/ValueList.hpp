#ifndef LOCIC_AST_VALUELIST_HPP
#define LOCIC_AST_VALUELIST_HPP

#include <vector>

#include <locic/AST/Node.hpp>

namespace locic {
	
	namespace AST {
		
		struct Value;
		
		typedef std::vector<Node<Value>> ValueList;
		
	}
	
}

#endif
