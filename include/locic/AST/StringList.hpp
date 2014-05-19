#ifndef LOCIC_AST_STRINGLIST_HPP
#define LOCIC_AST_STRINGLIST_HPP

#include <string>
#include <vector>

#include <locic/AST/Node.hpp>

namespace locic {

	namespace AST {
	
		typedef std::vector<Node<std::string>> StringList;
		
	}
	
}

#endif
