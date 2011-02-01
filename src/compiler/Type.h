#ifndef LOCIC_TYPE_H
#define LOCIC_TYPE_H

#include "List.h"
#include <string>
#include <map>

namespace Locic{

	enum TypeGroup{
		TYPE_CLASS,
		TYPE_INTERFACE
	};

	class MethodDef;

	class Type{
		public:
			Type(TypeGroup, const std::string&);

			TypeGroup Group();

			std::string Name();

			int Size();
			
			void Generate();

			MethodDef * LookupMethod(const std::string&);

			bool HasMethod(MethodDef *);

			void Add(MethodDef *);

			int Distance(Type *);

		private:
			std::string mName;
			TypeGroup mGroup;

			std::map<std::string, MethodDef *> mMethodMap;

	};

	inline Type * VoidType(){
		return new Type(TYPE_CLASS, "");
	}

	class ProxyType{
		public:
			inline ProxyType(const std::string& name) : mName(name){
				mType = 0;
			}

			inline ProxyType(Type * type) : mName("?"){
				mType = type;
			}

			inline void Set(Type * type){
				mType = type;
			}

			inline Type * Get(){
				if(mType == 0){
					throw std::string("Couldn't find type: ") + mName;
				}
				return mType;
			}

		private:
			std::string mName;
			Type * mType;

	};

}

#endif
