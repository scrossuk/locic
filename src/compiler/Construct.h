#ifndef LOCIC_CONSTRUCT_H
#define LOCIC_CONSTRUCT_H

#include <sstream>
#include "List.h"
#include "Tree.h"
#include "Type.h"

namespace Locic{

	class Construct: public Tree{
		public:
			inline Construct(ProxyType * proxy, std::string * name, List<Tree> * param) : mName(*name){
				mProxy = proxy;
				mParam = param;
			}

			inline TreeResult Generate(Resolver * resolver){
				Type * type = mProxy->Get();

				std::vector<std::string> paramStrings;
				List<Type> * paramTypes = new List<Type>();
				for(unsigned int i = 0; i < mParam->Size(); ++i){
					TreeResult res = (mParam->Get(i))->Generate(resolver);

					paramTypes->Add(res.type);
					paramStrings.push_back(res.str);
				}

				std::ostringstream s;

				s << "_" << type->Name() << "_" << mName << "((void *) 0";

				for(unsigned int i = 0; i < mParam->Size(); ++i){
					s << ", " << paramStrings[i];
				}

				s << ")";

				return TreeResult(type, s.str());
			}

		private:
			ProxyType * mProxy;
			List<Tree> * mParam;
			std::string mName;

	};

}

#endif
