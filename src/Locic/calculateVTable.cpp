#include <assert.h>
#include <stdio.h>
#include <string>
#include <vector>

#include <Locic/CodeGen/VTable.hpp>

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
	
	printf("Virtual table: %s.\n",
		Locic::CodeGen::VirtualTable::CalculateFromNames(methodNames).
		toString().c_str());
	
	return 0;
}

