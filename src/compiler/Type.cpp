#include <string>
#include <fstream>
#include "MethodDef.h"
#include "Type.h"

namespace Locic{

	Type::Type(TypeGroup group, const std::string& name) : mName(name){
		mGroup = group;
	}

	TypeGroup Type::Group(){
		return mGroup;
	}

	std::string Type::Name(){
		return mName;
	}

	int Type::Size(){
		return mMethodMap.size();
	}
	
	void Type::Generate(){
		if(mGroup == TYPE_CLASS){
			std::string headerFileName = mName + std::string(".loci.h");
			std::ofstream stream(headerFileName.c_str());
			
			stream << "extern void ** _vtable_" << mName << ";" << std::endl;
		
			std::map<std::string, MethodDef *>::iterator iter;
			for(iter = mMethodMap.begin(); iter != mMethodMap.end(); ++iter){
				stream << (iter->second)->Generate(mName) << std::endl;
			}
		}
	}

	MethodDef * Type::LookupMethod(const std::string& name){
		MethodDef * method = mMethodMap[name];

		if(method == 0){
			throw std::string("No method found: ") + name;
		}

		return method;
	}

	bool Type::HasMethod(MethodDef * otherDef){
		MethodDef * myDef = mMethodMap[otherDef->Name()];

		if(myDef != 0){
			Type * myReturnType = myDef->ReturnType();
			Type * otherReturnType = otherDef->ReturnType();

			if(otherReturnType->Distance(myReturnType) >= 0 && myDef->Distance(otherDef->ParameterTypes()) >= 0){
				return true;
			}
		}

		return false;
	}

	void Type::Add(MethodDef * def){
		mMethodMap[def->Name()] = def;
	}

	int Type::Distance(Type * type){
		if(this == type){
			return 0;
		}

		std::map<std::string, MethodDef *>::iterator iter;

		switch(mGroup){
			case TYPE_CLASS:
				if(type->Group() == TYPE_CLASS && mName == type->Name()){
					return 0;
				}else{
					return -1;
				}
			case TYPE_INTERFACE:
				if(type->Group() != TYPE_CLASS && type->Group() != TYPE_INTERFACE){
					return -1;
				}

				for(iter = mMethodMap.begin(); iter != mMethodMap.end(); ++iter){
					if(!type->HasMethod(iter->second)){
						return -1;
					}
				}

				return type->Size() - Size();

			default:
				return -1;
		};
	}
}
