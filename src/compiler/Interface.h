#ifndef LOCIC_INTERFACE_H
#define LOCIC_INTERFACE_H

#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include "Type.h"
#include "ProxyMethodDef.h"

namespace Locic{

	class ProxyMethodDef;

	class Interface{
		public:
			inline Interface(const std::string * name) : mName(*name){ }

			inline void FillType(Type * type){
				std::set<ProxyMethodDef *>::iterator iter;

				for(iter = mMethods.begin(); iter != mMethods.end(); ++iter){
					type->Add((*iter)->CreateDef());
				}
			}

			inline std::string Name(){
				return mName;
			}

			inline void Add(ProxyMethodDef * method){
				mMethods.insert(method);
			}

		private:
			std::string mName;

			std::set<ProxyMethodDef *> mMethods;

	};

}

#endif
