#ifndef LOCIC_METHODDEF_H
#define LOCIC_METHODDEF_H

#include <cctype>
#include <sstream>
#include <string>
#include "List.h"
#include "Type.h"

namespace Locic{

	class MethodDef{
		public:
			inline MethodDef(Type * returnType, const std::string& name, List<Type> * paramTypes) : mName(name){
				mReturnType = returnType;
				mParamTypes = paramTypes;
			}

			inline Type * ReturnType(){
				return mReturnType;
			}

			inline std::string Name(){
				return mName;
			}

			inline bool IsConstructor(){
				return isupper(mName[0]);
			}

			inline std::string Generate(std::string typeName){
				std::ostringstream s;
				s << "void * _" << typeName << "_" << mName << "(void *";

				for(unsigned int i = 0; i < mParamTypes->Size(); ++i){
					s << ", void *";
				}

				s << ");" << std::endl;

				return s.str();
			}

			inline int Distance(List<Type> * paramTypes){
				if(paramTypes->Size() != mParamTypes->Size()){
					return -1;
				}

				int distance = 0;

				for(unsigned int i = 0; i < paramTypes->Size(); ++i){
					Type * myType = mParamTypes->Get(i);
					Type * otherType = paramTypes->Get(i);

					int typeDistance = myType->Distance(otherType);

					if(typeDistance < 0){
						return -1;
					}else{
						distance += typeDistance;
					}
				}

				return distance;
			}

			inline unsigned int Hash(){
				std::string s = Name();

				unsigned int v = 0;

				for(unsigned int i = 0; i < s.size(); ++i){
					v += ((unsigned int) s[i]) * (i + 1);
				}

				return v % 32;
			}

			inline List<Type> * ParameterTypes(){
				return mParamTypes;
			}

		private:
			Type * mReturnType;
			std::string mName;
			List<Type> * mParamTypes;

	};

}

#endif
