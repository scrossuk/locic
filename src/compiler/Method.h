#ifndef LOCIC_METHOD_H
#define LOCIC_METHOD_H

#include <string>
#include <sstream>
#include "List.h"
#include "ParamSpec.h"
#include "Tree.h"
#include "Resolver.h"
#include "MethodDef.h"
#include "InternalClass.h"

namespace Locic{

	class Method{
		public:
			inline Method(const std::string * name, ParamSpec * param, Tree * tree) : mName(*name){
				mReturnType = 0;
				mParamNames = param->Names();
				mParamTypes = param->Types();
				mTree = tree;
			}

			inline Method(ProxyType * returnType, const std::string * name, ParamSpec * param, Tree * tree) : mName(*name){
				mReturnType = returnType;
				mParamNames = param->Names();
				mParamTypes = param->Types();
				mTree = tree;
			}

			inline std::string Generate(InternalClass * thisClass){
				Resolver resolver(thisClass);

				std::stringstream out;

				out << "void * _" << thisClass->Name() << "_" << mName << "(void * This";

				for(unsigned int i = 0; i < mParamTypes->Size(); ++i){
					ProxyType * proxy = mParamTypes->Get(i);
					Type * type = proxy->Get();

					resolver.DeclareVar(std::string(*(mParamNames->Get(i))), type);
					
					out << ", void * v" << (i + 1);
				}

				out << "){ ";

				TreeResult res = mTree->Generate(&resolver);
				
				for(unsigned int i = mParamTypes->Size(); i < resolver.GetNumVars(); ++i){
					out << "void * v" << (i + 1) << "; ";
				}

				out << " return " << res.str << "; }";

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
			ProxyType * mReturnType;
			std::string mName;
			List<ProxyType> * mParamTypes;
			List<std::string> * mParamNames;
			Tree * mTree;

	};

}


#endif
