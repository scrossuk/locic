#include <assert.h>
#include <stdio.h>

#include <string>
#include <vector>

#include <locic/CodeGen/VTable.hpp>
#include <locic/Support/MakeString.hpp>
#include <locic/Support/Map.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/StringHost.hpp>

using namespace locic::CodeGen;

int main(int argc, char * argv[]){
	if(argc < 2){
		printf("Must provide at least one name.\n");
		return -1;
	}
	assert(argc >= 2);
	
	locic::StringHost stringHost;
	
	std::vector<locic::String> methodNames;
	
	for(int i = 1; i < argc; i++){
		methodNames.push_back(locic::String(stringHost, std::string(argv[i])));
	}
	
	printf("Given %llu method names.\n\n", (unsigned long long) methodNames.size());
	
	locic::Map<MethodHash, locic::String> namesMap;
	
	for (const auto& name: methodNames) {
		namesMap.insert(CreateMethodNameHash(name), name);
	}
	
	const auto vtable = locic::CodeGen::VirtualTable::CalculateFromNames(methodNames);
	
	printf("Virtual table: %s.\n",
		locic::formatMessage(vtable.toStringWithMapping(namesMap)).c_str());
	
	return 0;
}

