#ifndef LOCIC_SCOPE_H
#define LOCIC_SCOPE_H

#include "List.h"
#include "Tree.h"
#include "Resolver.h"

namespace Locic{

	class Scope: public Tree{
		public:
			inline Scope(List<Tree> * treeList){
				mTreeList = treeList;
			}

			inline TreeResult Generate(Resolver * resolver){
				resolver->PushScope();

				std::string s = "(";
				
				Type * lastType;

				for(unsigned int i = 0; i < mTreeList->Size(); ++i){
					if(i > 0){
						s += ", ";
					}
				
					TreeResult res = (mTreeList->Get(i))->Generate(resolver);

					s += res.str;
					
					lastType = res.type;
				}

				s += ")";

				resolver->PopScope();

				return TreeResult(lastType, s);
			}

		private:
			List<Tree> * mTreeList;

	};

}

#endif
