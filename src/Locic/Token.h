#ifndef LOCIC_TOKEN_H
#define LOCIC_TOKEN_H

typedef union Locic_TokenValue{
	char * str;
	int boolValue;
	int intValue;
	float floatValue;
} Locic_TokenValue;

typedef struct Locic_Token{
	int id;
	Locic_TokenValue value;
} Locic_Token;

#endif
