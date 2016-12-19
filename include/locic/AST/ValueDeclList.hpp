#ifndef LOCIC_AST_VALUEDECLLIST_HPP
#define LOCIC_AST_VALUEDECLLIST_HPP

#include <vector>

#include <locic/AST/Node.hpp>

namespace locic {
	
	namespace AST {
		
		struct ValueDecl;
		
		typedef std::vector<Node<ValueDecl>> ValueDeclList;
		
	}
	
}

#endif
