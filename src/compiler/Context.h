#ifndef LOCIC_CONTEXT_H
#define LOCIC_CONTEXT_H

#include <string>
#include <map>
#include <set>

namespace Locic{

	class ProxyType;
	class Type;
	class Class;
	class Interface;

	class Context{
		public:
			Context();

			bool AddFile(const std::string&);

			void GenerateOutput();

			void AddClass(Class *);

			void AddInterface(Interface *);

			void AddType(Type *);

			ProxyType * Resolve(const std::string *);

			ProxyType * Resolve(const std::string&);

		private:
			std::set<Class *> mClasses;

			std::set<Interface *> mInterfaces;

			std::map<std::string, ProxyType *> mTypes;

	};

}

#endif
