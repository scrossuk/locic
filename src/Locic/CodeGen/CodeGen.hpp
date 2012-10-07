#ifndef LOCIC_CODEGEN_HPP
#define LOCIC_CODEGEN_HPP

#include <cstddef>
#include <string>
#include <Locic/SEM.hpp>

void * Locic_CodeGenAlloc(const std::string& moduleName, std::size_t optLevel);
	
void Locic_CodeGenFree(void * context);
	
void Locic_CodeGen(void * context, SEM::Module * module);

void Locic_CodeGenWriteToFile(void * context, const std::string& fileName);

void Locic_CodeGenDumpToFile(void * context, const std::string& fileName);

void Locic_CodeGenDump(void * context);

#endif
