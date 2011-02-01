#ifndef LOCIC_INTERNALCLASS_H
#define LOCIC_INTERNALCLASS_H

#include <string>
#include <map>
#include <set>
#include <vector>
#include "Type.h"
#include "Class.h"

namespace Locic{

	class Method;

	class InternalClass: public Class{
		public:
			InternalClass(const std::string *);

			void Require(ProxyType *);

			void FillType(Type *);

			std::string Name();

			void Generate();

			void Add(Method *);

			void Add(ProxyType *, const std::string *);

			Type * GetVar(const std::string&);

		private:
			unsigned int mNextID;
		
			std::string mName;

			std::set<Method *> mMethods;

			std::map<std::string, ProxyType *> mVars;
			
			std::map<std::string, unsigned int> mVarIDs;

			std::vector<ProxyType *> mRequire;

	};

}

#endif
