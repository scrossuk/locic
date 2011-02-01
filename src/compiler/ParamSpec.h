#ifndef LOCIC_PARAMSPEC_H
#define LOCIC_PARAMSPEC_H

#include <string>
#include "Type.h"

namespace Locic{

	class ParamSpec{
		public:
			inline ParamSpec(){
				mTypes = new List<ProxyType>();
				mNames = new List<std::string>();
			}

			inline ParamSpec(ProxyType * proxy, std::string * name){
				mTypes = new List<ProxyType>(proxy);
				mNames = new List<std::string>(name);
			}

			inline ParamSpec * Add(ProxyType * proxy, std::string * name){
				mTypes->Add(proxy);
				mNames->Add(name);
				return this;
			}

			inline List<ProxyType> * Types(){
				return mTypes;
			}

			inline List<std::string> * Names(){
				return mNames;
			}

		private:
			List<ProxyType> * mTypes;
			List<std::string> * mNames;

	};

}

#endif
