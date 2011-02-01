#ifndef LOCIRUNTIME_CALL_H
#define LOCIRUNTIME_CALL_H

void * Loci_CallHashed0(void *, unsigned int);
void * Loci_CallHashed1(void *, unsigned int, void *);
void * Loci_CallHashed2(void *, unsigned int, void *, void *);

void * Loci_Call0(void *, const char *);
void * Loci_Call1(void *, const char *, void *);
void * Loci_Call2(void *, const char *, void *, void *);

#endif
