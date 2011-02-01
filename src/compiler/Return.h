#ifndef LOCIC_RETURN_H
#define LOCIC_RETURN_H

#include <string>
#include "Tree.h"

namespace Locic{

	class Return: public Tree{
		public:
			inline Return(Tree * tree){
				mTree = tree;
			}

			inline TreeResult Generate(Resolver * resolver){
				TreeResult res = mTree->Generate(resolver);
				return TreeResult(res.type, std::string("return ") + res.str);
			}

		private:
			Tree * mTree;

	};

}

#endif
