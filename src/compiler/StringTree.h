#ifndef LOCIC_STRINGTREE_H
#define LOCIC_STRINGTREE_H

#include <sstream>
#include <string>
#include "Tree.h"

namespace Locic{

	std::string escape(std::string string){
		std::string res;
		for(unsigned int i = 0; i < string.size(); ++i){
			if(string[i] == '"'){
				res += "\\\"";
			}else if(string[i] == '\\'){
				res += "\\\\";
			}else if(string[i] == '\n'){
				res += "\\n";
			}else{
				res += string[i];
			}
		}
		return res;
	}

	class StringTree: public Tree{
		public:
			inline StringTree(ProxyType * proxy, const std::string& val) : mVal(val){
				mProxy = proxy;
			}

			inline TreeResult Generate(Resolver * resolver){
				std::ostringstream s;
				s << mVal.size();
				return TreeResult(mProxy->Get(), std::string("_String_(") + s.str() + std::string(", \"") + escape(mVal) + std::string("\")"));
			}

		private:
			std::string mVal;
			ProxyType * mProxy;

	};

}

#endif
