#ifndef LOCIC_DIRECTCONSTRUCT_H
#define LOCIC_DIRECTCONSTRUCT_H

#include <sstream>
#include "List.h"
#include "Tree.h"
#include "Type.h"

namespace Locic{

	class DirectConstruct: public Tree{
		public:
			inline DirectConstruct(ProxyType * proxy, List<Tree> * param){
				mProxy = proxy;
				mParam = param;
			}

			inline TreeResult Generate(Resolver * resolver){
				Type * type = mProxy->Get();

				std::ostringstream s;
				
				std::string name = type->Name();

				s << "_" << name << "_" << name << "((void *) 0";

				for(unsigned int i = 0; i < mParam->Size(); ++i){
					TreeResult res = (mParam->Get(i))->Generate(resolver);
					s << ", " << res.str;
				}

				s << ")";

				return TreeResult(type, s.str());
			}

		private:
			ProxyType * mProxy;
			List<Tree> * mParam;

	};

}

#endif
