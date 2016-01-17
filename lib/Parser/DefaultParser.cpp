#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <locic/Frontend/DiagnosticReceiver.hpp>
#include <locic/Parser/DefaultParser.hpp>
#include <locic/Parser/NamespaceParser.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/StringHost.hpp>

#include "LexLexer.hpp"

namespace locic {
	
	namespace Parser {
		
		class DefaultParserImpl {
		public:
			DefaultParserImpl(const StringHost& stringHost,
			                  AST::NamespaceList& argRootNamespaceList,
			                  FILE * file, const std::string& fileName,
			                  DiagnosticReceiver& argDiagReceiver)
			: rootNamespaceList_(argRootNamespaceList),
			lexer_(file, String(stringHost, fileName), argDiagReceiver),
			diagReceiver_(argDiagReceiver) { }
			
			AST::NamespaceList& rootNamespaceList() {
				return rootNamespaceList_;
			}
			
			LexLexer& lexer() {
				return lexer_;
			}
			
			DiagnosticReceiver& diagReceiver() {
				return diagReceiver_;
			}
			
		private:
			AST::NamespaceList& rootNamespaceList_;
			LexLexer lexer_;
			DiagnosticReceiver& diagReceiver_;
			
		};
		
		DefaultParser::DefaultParser(const StringHost& stringHost, AST::NamespaceList& rootNamespaceList,
		                             FILE * file, const std::string& fileName, DiagnosticReceiver& diagReceiver)
		: impl_(new DefaultParserImpl(stringHost, rootNamespaceList, file, fileName, diagReceiver)) { }
		
		DefaultParser::~DefaultParser() { }
		
		void DefaultParser::parseFile() {
			TokenReader reader(impl_->lexer().getLexer(), impl_->diagReceiver());
			const auto namespaceDecl = NamespaceParser(reader).parseGlobalNamespace();
			impl_->rootNamespaceList().push_back(namespaceDecl);
		}
		
	}
	
}

