#ifndef LOCIC_FLOATTREE_H
#define LOCIC_FLOATTREE_H

#include <sstream>
#include <string>
#include "Type.h"
#include "Tree.h"
#include "MethodDef.h"

namespace Locic{

	class FloatTree: public Tree{
		public:
			inline FloatTree(ProxyType * proxy, double val){
				mProxy = proxy;
				mVal = val;
			}

			inline TreeResult Generate(Resolver * resolver){
				std::ostringstream s;
				s << mVal;
				return TreeResult(mProxy->Get(), std::string("_Float_(") + s.str() + std::string("f)"));
			}

		private:
			double mVal;
			ProxyType * mProxy;

	};

}

#endif
