#ifndef LOCIC_UINTTREE_H
#define LOCIC_UINTTREE_H

#include <string>
#include "Tree.h"

namespace Locic{

	class UintTree: public Tree{
		public:
			inline UintTree(ProxyType * proxy, unsigned int val){
				mProxy = proxy;
				mVal = val;
			}

			inline TreeResult Generate(Resolver * resolver){
				char buffer[33];
				sprintf(buffer, "%d", mVal);
				return TreeResult(mProxy->Get(), std::string("_Uint_(") + std::string(buffer) + std::string(")"));
			}

		private:
			ProxyType * mProxy;
			unsigned int mVal;
	};

}

#endif
