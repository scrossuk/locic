#ifndef LOCIC_VARIABLE_H
#define LOCIC_VARIABLE_H

#include <sstream>
#include <string>
#include "Tree.h"

namespace Locic{

	class Variable: public Tree{
		public:
			inline Variable(const std::string * name) : mName(*name){ }

			inline TreeResult Generate(Resolver * resolver){
				Type * type = resolver->GetVar(mName);
				unsigned int id = resolver->GetVarID(mName);
				std::ostringstream s;
				s << "v" << id;
				return TreeResult(type, s.str());
			}

		private:
			std::string mName;

	};

}

#endif
