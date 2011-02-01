#ifndef LOCIC_VARIABLEDECLARE_H
#define LOCIC_VARIABLEDECLARE_H

#include <sstream>
#include <string>
#include "Tree.h"
#include "Type.h"

namespace Locic{

	class VariableDeclare: public Tree{
		public:
			inline VariableDeclare(const std::string * name, Tree * tree) : mName(*name){
				mTree = tree;
			}

			inline TreeResult Generate(Resolver * resolver){
				TreeResult res = mTree->Generate(resolver);

				unsigned int id = resolver->DeclareVar(mName, res.type);
				
				std::ostringstream s;

				s << "v" << id << " = ";
				
				return TreeResult(res.type, s.str() + res.str);
			}

		private:
			ProxyType * mType;
			std::string mName;
			Tree * mTree;

	};

}

#endif
