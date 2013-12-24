#include <assert.h>
#include <stdio.h>

#include <string>
#include <vector>

#include <Locic/Log.hpp>
#include <Locic/Map.hpp>
#include <Locic/CodeGen/VTable.hpp>

using namespace Locic::CodeGen;

int main(int argc, char * argv[]){
	if(argc < 2){
		printf("Must provide at least one name.\n");
		return -1;
	}
	assert(argc >= 2);
	
	std::vector<std::string> methodNames;
	
	for(int i = 1; i < argc; i++){
		methodNames.push_back(std::string(argv[i]));
	}
	
	printf("Given %llu method names.\n",
		(unsigned long long) methodNames.size());
	
	printf("\n");
	
	Locic::Map<MethodHash, std::string> namesMap;
	
	for (auto name: methodNames) {
		namesMap.insert(CreateMethodNameHash(name), name);
	}
	
	const auto vtable = Locic::CodeGen::VirtualTable::CalculateFromNames(methodNames);
	
	printf("Virtual table: %s.\n",
		Locic::formatMessage(vtable.toStringWithMapping(namesMap)).c_str());
	
	return 0;
}

