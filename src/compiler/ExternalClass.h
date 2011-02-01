#ifndef LOCIC_EXTERNALCLASS_H
#define LOCIC_EXTERNALCLASS_H

#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include "Type.h"
#include "Class.h"
#include "ProxyMethodDef.h"

namespace Locic{

	class ProxyMethodDef;

	class ExternalClass: public Class{
		public:
			inline ExternalClass(const std::string * name) : mName(*name){ }

			inline void FillType(Type * type){
				std::set<ProxyMethodDef *>::iterator iter;

				for(iter = mMethods.begin(); iter != mMethods.end(); ++iter){
					type->Add((*iter)->CreateDef());
				}
			}

			inline std::string Name(){
				return mName;
			}

			inline void Generate(){
				std::string sourceFileName = mName + std::string(".loci.c");
				std::ofstream sourceStream(sourceFileName.c_str());
				std::vector<std::string> vtable[32];
				
				std::set<ProxyMethodDef *>::iterator iter;
				for(iter = mMethods.begin(); iter != mMethods.end(); ++iter){
					MethodDef * def = (*iter)->CreateDef();
					
					if(def->Name() != "Construct"){
						vtable[def->Hash()].push_back(std::string("_") + mName + std::string("_") + def->Name());
					}
				}
				
				sourceStream << "#include \"" << mName << ".loci.h\"" << std::endl;

				sourceStream << "static void * localvtable[] = { ";

				for(unsigned int i = 0; i < 32; ++i){
					if(i > 0){
						sourceStream << ", ";
					}

					if(vtable[i].size() == 0){
						sourceStream << "(void *) 0";
					}else if(vtable[i].size() == 1){
						sourceStream << "(void *) " << vtable[i][0];
					}else{
						sourceStream << "(void *) 0 /*collision*/";
					}
				}

				sourceStream << " }; void ** _vtable_" << mName << " = (void **) localvtable;" << std::endl << std::endl;
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
