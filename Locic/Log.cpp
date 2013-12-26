#include <assert.h>
#include <stdio.h>
#include <string>
#include <Locic/Log.hpp>

namespace Locic{
	
	static std::string levelToString(LogLevel level){
		switch(level){
			case LOG_NONE:
				return "NONE";
			case LOG_CRITICAL:
				return "CRITICAL";
			case LOG_ERROR:
				return "ERROR";
			case LOG_WARNING:
				return "WARNING";
			case LOG_NOTICE:
				return "NOTICE";
			case LOG_INFO:
				return "INFO";
			case LOG_EXCESSIVE:
				return "EXCESSIVE";
			case LOG_ALL:
				return "ALL";
			default:
				assert(false && "Unknown log level.");
				return "";
		}
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
	
	static LogLevel DISPLAY_LEVEL = LOG_NOTICE;
	
	void setLogDisplayLevel(LogLevel level) {
		DISPLAY_LEVEL = level;
	}
	
	void log(LogLevel level, const LogInfo& info, const std::string& message){
		assert(level != LOG_NONE && level != LOG_ALL);
		
		if (unsigned(level) > unsigned(DISPLAY_LEVEL)) {
			return;
		}
		
		fprintf(stderr, "[File %s] [Function %s] [Line %d] (%s):\n\t%s\n\n",
			info.fileName.c_str(), info.functionName.c_str(),
			info.lineNumber, levelToString(level).c_str(),
			formatMessage(message).c_str());
	}

}

