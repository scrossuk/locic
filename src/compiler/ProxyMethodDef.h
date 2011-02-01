#ifndef LOCIC_PROXYMETHODDEF_H
#define LOCIC_PROXYMETHODDEF_H

#include <sstream>
#include <string>
#include "List.h"
#include "Type.h"
#include "MethodDef.h"

namespace Locic{

	class ProxyMethodDef{
		public:
			inline ProxyMethodDef(ProxyType * returnType, const std::string * name, List<ProxyType> * paramTypes) : mName(*name){
				mParamTypes = paramTypes;
				mReturnType = returnType;
			}

			inline std::string Generate(const std::string className){
				std::stringstream out;

				out << "void * _" << className << "_" << mName << "(void *";

				for(unsigned int i = 0; i < mParamTypes->Size(); ++i){
					out << ", void *";
				}

				out << ");";

				return out.str();
			}

			inline MethodDef * CreateDef(){
				List<Type> * paramTypes = new List<Type>();

				for(unsigned int i = 0; i < mParamTypes->Size(); ++i){
					ProxyType * proxy = mParamTypes->Get(i);
					paramTypes->Add(proxy->Get());
				}

				return new MethodDef(mReturnType == 0 ? VoidType() : mReturnType->Get(), mName, paramTypes);
			}

		private:
			std::string mName;
			ProxyType * mReturnType;
			List<ProxyType> * mParamTypes;

	};

}

#endif
