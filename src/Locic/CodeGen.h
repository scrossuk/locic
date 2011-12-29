#ifndef LOCIC_CODEGEN_H
#define LOCIC_CODEGEN_H

#ifdef __cplusplus
extern "C" {
#endif

void * Locic_CodeGenAlloc(const char * moduleName);
	
void Locic_CodeGenFree(void * context);
	
void Locic_CodeGen(void * context, AST_File * file);

void Locic_CodeGenDump(void * context);

#ifdef __cplusplus
}
#endif

#endif
