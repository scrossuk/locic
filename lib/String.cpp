#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <locic/String.hpp>

namespace locic {

	std::string makeString(const char* format, ...) {
		va_list varArgList;
		
		size_t bufferSize = 1024;
		char stackBuffer[1024];
		std::vector<char> dynamicBuffer;
		char* buffer = &stackBuffer[0];
		
		while(true) {
			va_start(varArgList, format);
			const int needed = vsnprintf(buffer, bufferSize, format, varArgList);
			va_end(varArgList);
			
			// In case the buffer provided is too small, some
			// platforms return the needed buffer size, whereas
			// some simply return -1.
			if(needed <= (int)bufferSize && needed >= 0) {
				return std::string(buffer, (size_t) needed);
			}
			
			// Need to increase buffer size; use needed size if available.
			bufferSize = (needed > 0) ? (needed + 1) : (bufferSize * 2);
			dynamicBuffer.resize(bufferSize);
			buffer = &dynamicBuffer[0];
		}
	}
	
	std::string escapeString(const std::string& string) {
		std::string resultString;
		
		for(size_t i = 0; i < string.size(); i++) {
			const char c = string.at(i);
			
			switch(c) {
				case '\a':
					resultString += "\\a";
					break;
				case '\b':
					resultString += "\\b";
					break;
				case '\t':
					resultString += "\\t";
					break;
				case '\n':
					resultString += "\\n";
					break;
				case '\v':
					resultString += "\\v";
					break;
				case '\f':
					resultString += "\\f";
					break;
				case '\r':
					resultString += "\\r";
					break;
				case '\\':
					resultString += "\\\\";
					break;
				case '\"':
					resultString += "\\\"";
					break;
				default:
					resultString += c;
					break;
			}
		}
		
		return resultString;
	}
	
	static std::string createTabs(size_t numTabs){
		std::string s;
		for(size_t i = 0; i < numTabs; i++){
			s += '\t';
		}
		return s;
	}
	
	std::string formatMessage(const std::string& message){
		size_t tabLevel = 1;
		std::string resultString;
		char prevChar = 0x00;
		
		for(size_t i = 0; i < message.size(); i++){
			const char c = message.at(i);
			switch(c){
				case '{':
				case '(':
					tabLevel++;
					resultString += c;
					resultString += '\n';
					resultString += createTabs(tabLevel);
					break;
				case '}':
				case ')':
					if(tabLevel >= 2) tabLevel--;
					resultString += '\n';
					resultString += createTabs(tabLevel);
					resultString += c;
					break;
				case ',':
					resultString += ",\n";
					resultString += createTabs(tabLevel);
					break;
				case ' ':
					if(prevChar != ','){
						resultString += ' ';
					}
					break;
				default:
					resultString += c;
					break;
			}
			prevChar = c;
		}
		
		return resultString;
	}
	
}
	
