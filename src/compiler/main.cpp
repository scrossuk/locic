#include <iostream>
#include <string>

#include "Context.h"
#include "Type.h"

using namespace Locic;

int main(){
	Context context;

	if(!context.AddFile("Main.loci")){
		std::cout << "Failed to parse Main.loci" << std::endl;
	}
	
	if(!context.AddFile("Other.loci")){
		std::cout << "Failed to parse Other.loci" << std::endl;
	}

	std::cout << "Generating output..." << std::endl;

	try{
		context.GenerateOutput();
	}catch(const std::string& error){
		std::cout << "Error: " << error << std::endl;
	}catch(const char * error){
		std::cout << "Error: " << error << std::endl;
	}

	return 0;
}

