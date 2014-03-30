#include <assert.h>
#include <stdio.h>

#include <string>
#include <vector>

#include <locic/Map.hpp>
#include <locic/String.hpp>
#include <locic/CodeGen/VTable.hpp>

using namespace locic::CodeGen;

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
	
	printf("Given %llu method names.\n\n", (unsigned long long) methodNames.size());
	
	locic::Map<MethodHash, std::string> namesMap;
	
	for (auto name: methodNames) {
		namesMap.insert(CreateMethodNameHash(name), name);
	}
	
	const auto vtable = locic::CodeGen::VirtualTable::CalculateFromNames(methodNames);
	
	printf("Virtual table: %s.\n",
		locic::formatMessage(vtable.toStringWithMapping(namesMap)).c_str());
	
	return 0;
}

