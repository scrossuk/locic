#ifndef LOCIC_TOKEN_H
#define LOCIC_TOKEN_H

typedef union Locic_Token{
	char * str;
	int boolValue;
	int intValue;
	float floatValue;
} Locic_Token;

#endif
