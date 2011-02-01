#include <stdio.h>
#include <sstream>
#include <string>
#include <cstdlib>
#include "Type.h"
#include "Context.h"
#include "Class.h"
#include "Interface.h"

extern Locic::Context * globalContext;
extern FILE * yyin;
int yyparse();

namespace Locic{

	Context::Context(){ }

	bool Context::AddFile(const std::string& fileName){
		yyin = fopen(fileName.c_str(), "r");

		if (yyin != NULL){
			globalContext = this;
			yyparse();
			fclose(yyin);
			return true;
		}else{
			return false;
		}
	}

	void Context::GenerateOutput(){
		std::set<Class *>::iterator iter;
		for(iter = mClasses.begin(); iter != mClasses.end(); ++iter){
			Class * c = *iter;
			Type * type = Resolve(c->Name())->Get();
			c->FillType(type);
			type->Generate();
		}

		std::set<Interface *>::iterator interfaceIter;
		for(interfaceIter = mInterfaces.begin(); interfaceIter != mInterfaces.end(); ++interfaceIter){
			Interface * interface = *interfaceIter;
			Type * type = Resolve(interface->Name())->Get();
			interface->FillType(type);
		}

		for(iter = mClasses.begin(); iter != mClasses.end(); ++iter){
			Class * c = *iter;
			c->Generate();
		}

		std::ostringstream s;

		s << "gcc -o main ";

		for(iter = mClasses.begin(); iter != mClasses.end(); ++iter){
			Class * c = *iter;
			s << c->Name() << ".loci.c ";
		}

		s << "-L./runtime -lloci ";

		system(s.str().c_str());
	}

	void Context::AddClass(Class * c){
		mClasses.insert(c);
		AddType(new Type(TYPE_CLASS, c->Name()));
	}

	void Context::AddInterface(Interface * interface){
		mInterfaces.insert(interface);
		AddType(new Type(TYPE_INTERFACE, interface->Name()));
	}

	void Context::AddType(Type * type){
		ProxyType * proxy = Resolve(type->Name());
		proxy->Set(type);
	}

	ProxyType * Context::Resolve(const std::string * typeName){
		return Resolve(std::string(*typeName));
	}

	ProxyType * Context::Resolve(const std::string& typeName){
		ProxyType * proxy = mTypes[typeName];
		if(proxy == 0){
			proxy = new ProxyType(typeName);
			mTypes[typeName] = proxy;
		}
		return proxy;
	}

}



