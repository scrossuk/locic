#ifndef LOCIC_CODEGEN_HPP
#define LOCIC_CODEGEN_HPP

#include <string>
#include <Locic/SEM.hpp>

void * Locic_CodeGenAlloc(const std::string& moduleName);
	
void Locic_CodeGenFree(void * context);
	
void Locic_CodeGen(void * context, SEM::Module * module);

void Locic_CodeGenWriteToFile(void * context, const std::string& fileName);

void Locic_CodeGenDump(void * context);

#endif
