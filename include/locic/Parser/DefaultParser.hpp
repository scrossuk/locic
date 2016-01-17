#ifndef LOCIC_PARSER_DEFAULTPARSER_HPP
#define LOCIC_PARSER_DEFAULTPARSER_HPP

#include <cstdio>
#include <string>

#include <locic/AST.hpp>

namespace locic {
	
	class DiagnosticReceiver;
	class StringHost;
	
	namespace Parser {
		
		class DefaultParser {
			public:
				DefaultParser(const StringHost& stringHost, AST::NamespaceList& rootNamespaceList,
				              FILE * file, const std::string& fileName, DiagnosticReceiver& diagReceiver);
				~DefaultParser();
				
				void parseFile();
				
			private:
				std::unique_ptr<class DefaultParserImpl> impl_;
				
		};
		
	}
	
}

#endif
