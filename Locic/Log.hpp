#ifndef LOCIC_LOG_HPP
#define LOCIC_LOG_HPP

#include <string>
#include <Locic/String.hpp>

#define LOG(level, message, ...) do{ ::Locic::log(level, LogInfo(__LINE__, __PRETTY_FUNCTION__, __FILE__, __DATE__, __TIME__), ::Locic::makeString(message, ##__VA_ARGS__)); } while(false);

namespace Locic{
	
	struct LogInfo{
		int lineNumber;
		std::string functionName, fileName,
			compileDate, compileTime;
		
		inline LogInfo(int line, const std::string& func,
			const std::string& file, const std::string& date,
			const std::string& time)
			: lineNumber(line), functionName(func),
			fileName(file), compileDate(date), compileTime(time){ }
	};

	enum LogLevel{
		LOG_NONE = 0,
		LOG_CRITICAL,
		LOG_ERROR,
		LOG_WARNING,
		LOG_NOTICE,
		LOG_INFO,
		LOG_EXCESSIVE,
		LOG_ALL
	};
	
	std::string formatMessage(const std::string& message);
	
	void log(LogLevel level, const LogInfo& info, const std::string& message);

}

#endif
