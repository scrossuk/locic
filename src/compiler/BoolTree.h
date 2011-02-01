#ifndef LOCIC_BOOLTREE_H
#define LOCIC_BOOLTREE_H

#include <string>
#include "Tree.h"

namespace Locic{

	class BoolTree: public Tree{
		public:
			inline BoolTree(ProxyType * proxy, bool val){
				mVal = val;
				mProxy = proxy;
			}

			inline TreeResult Generate(Resolver * resolver){
				return TreeResult(mProxy->Get(), std::string("_Bool_(") + std::string(mVal ? "1" : "0") + std::string(")"));
			}

		private:
			bool mVal;
			ProxyType * mProxy;

	};

}

#endif
