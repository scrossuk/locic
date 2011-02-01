#ifndef LOCIC_TREE_H
#define LOCIC_TREE_H

#include <string>

namespace Locic{

	class ProxyType;
	class Resolver;

	struct TreeResult{
		Type * type;
		std::string str;

		TreeResult(Type * t, const std::string& s) : str(s){
			type = t;
		}
	};

	class Tree{
		public:
			virtual TreeResult Generate(Resolver *) = 0;

	};

}

#endif
