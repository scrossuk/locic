#ifndef LOCIC_METHODCALL_H
#define LOCIC_METHODCALL_H

#include <vector>
#include <string>
#include "Tree.h"
#include "List.h"
#include "Type.h"
#include "MethodDef.h"

namespace Locic{

	class Object;

	class MethodCall: public Tree{
		public:
			inline MethodCall(Tree * tree, const std::string * name, List<Tree> * param) : mName(*name){
				mTree = tree;
				mParam = param;
			}

			inline TreeResult Generate(Resolver * resolver){
				std::vector<std::string> paramStrings;

				List<Type> * paramTypes = new List<Type>();
				for(unsigned int i = 0; i < mParam->Size(); ++i){
					TreeResult res = (mParam->Get(i))->Generate(resolver);

					paramTypes->Add(res.type);
					paramStrings.push_back(res.str);
				}

				TreeResult treeRes = mTree->Generate(resolver);

				MethodDef * def = (treeRes.type)->LookupMethod(mName);

				std::ostringstream s;

				s << "Loci_CallHashed" << mParam->Size() << "(" << treeRes.str << ", " << def->Hash();

				for(unsigned int i = 0; i < mParam->Size(); ++i){
					s << "," << paramStrings[i];
				}

				s << ")";

				return TreeResult(def->ReturnType(), s.str());
			}

		private:
			Tree * mTree;
			std::string mName;
			List<Tree> * mParam;

	};

}

#endif
