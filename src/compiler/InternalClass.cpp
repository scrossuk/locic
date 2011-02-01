#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Type.h"
#include "InternalClass.h"
#include "Method.h"

namespace Locic{

	InternalClass::InternalClass(const std::string * name) : mName(*name){
		mNextID = 1;
	}

	void InternalClass::FillType(Type * type){
		std::set<Method *>::iterator iter;

		for(iter = mMethods.begin(); iter != mMethods.end(); ++iter){
			type->Add((*iter)->CreateDef());
		}
	}

	void InternalClass::Require(ProxyType * proxy){
		mRequire.push_back(proxy);
	}

	std::string InternalClass::Name(){
		return mName;
	}

	void InternalClass::Generate(){
		std::string fileName = mName + std::string(".loci.c");
		std::ofstream stream(fileName.c_str());
		stream << "#include \"runtime/loci.h\"" << std::endl;
		stream << "#include \"" << mName << ".loci.h\"" << std::endl;

		std::vector<ProxyType *>::iterator p;
		for(p = mRequire.begin(); p != mRequire.end(); ++p){
			Type * type = (*p)->Get();
			stream << "#include \"" << type->Name() << ".loci.h\"" << std::endl;
		}

		stream << "void * _" << mName << "_" << mName << "(";

		for(unsigned int i = 1; i < mNextID; ++i){
			if(i > 0){
				stream << ", ";
			}
			stream << "void * v" << i;
		}

		stream << "){ void ** object = (void **) Loci_Allocate(sizeof(void *) * " << mNextID << "); ";
		stream << "object[0] = _vtable_" << mName << "; ";

		for(unsigned int i = 1; i < mNextID; ++i){
			stream << "object[" << i << "] = v" << i << "; ";
		}

		stream << "return (void *) (object + 1); }" << std::endl;

		std::vector<std::string> vtable[32];

		std::set<Method *>::iterator iter;
		for(iter = mMethods.begin(); iter != mMethods.end(); ++iter){
			MethodDef * def = (*iter)->CreateDef();

			if(!def->IsConstructor()){
				vtable[def->Hash()].push_back(std::string("_") + mName + std::string("_") + def->Name());
			}

			stream << (*iter)->Generate(this) << std::endl;
		}

		stream << "static void * localvtable[] = { ";

		for(unsigned int i = 0; i < 32; ++i){
			if(i > 0){
				stream << ", ";
			}

			if(vtable[i].size() == 0){
				stream << "(void *) 0";
			}else if(vtable[i].size() == 1){
				stream << "(void *) " << vtable[i][0];
			}else{
				stream << "(void *) 0 /*collision*/";
			}
		}

		stream << " }; void ** _vtable_" << mName << " = (void **) localvtable;" << std::endl << std::endl;
	}

	void InternalClass::Add(Method * method){
		mMethods.insert(method);
	}

	void InternalClass::Add(ProxyType * proxy, const std::string * name){
		mVars[std::string(*name)] = proxy;
		mVarIDs[std::string(*name)] = mNextID++;
	}

	Type * InternalClass::GetVar(const std::string& name){
		return (mVars[name])->Get();
	}

}


