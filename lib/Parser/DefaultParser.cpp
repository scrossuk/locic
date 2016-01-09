#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <locic/Parser/Context.hpp>
#include <locic/Parser/DefaultParser.hpp>
#include <locic/Parser/DiagnosticReceiver.hpp>
#include <locic/Parser/Diagnostics.hpp>
#include <locic/Parser/NamespaceParser.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Support/StringHost.hpp>

#include "LexerAPI.hpp"
#include "LexLexer.hpp"
#include "Token.hpp"

namespace locic {
	
	namespace Parser {
		
		static std::string formatLocation(const Debug::SourceLocation& location) {
			return makeString("%s:%u:%u", location.fileName().c_str(),
			                  (unsigned) location.range().start().lineNumber(),
			                  (unsigned) location.range().start().column());
		}
		
		static std::string formatLevel(const DiagLevel level) {
			switch (level) {
				case DiagLevel::Error:
					return "error";
				case DiagLevel::Warning:
					return "warning";
				case DiagLevel::Notice:
					return "notice";
			}
		}
		
		class DefaultParserImpl: public DiagnosticReceiver {
		public:
			DefaultParserImpl(const StringHost& stringHost, AST::NamespaceList& rootNamespaceList,
			                  FILE * file, const std::string& fileName)
			: context_(stringHost, rootNamespaceList, fileName),
			lexer_(file, context_.fileName()) { }
			
			Context& context() {
				return context_;
			}
			
			LexerAPI& lexer() {
				return lexer_;
			}
			
			void issueDiag(std::unique_ptr<Diag> diag,
			               const Debug::SourceLocation& location) {
				const auto error = makeString("%s: %s: %s",
				                              formatLocation(location).c_str(),
				                              formatLevel(diag->level()).c_str(),
				                              diag->toString().c_str());
				context().error(error, location);
			}
			
		private:
			Context context_;
			LexLexer lexer_;
			
		};
		
		DefaultParser::DefaultParser(const StringHost& stringHost, AST::NamespaceList& rootNamespaceList,
		                             FILE * file, const std::string& fileName, bool /*useNewParser*/)
		: impl_(new DefaultParserImpl(stringHost, rootNamespaceList, file, fileName)) { }
		
		DefaultParser::~DefaultParser() { }
		
		bool DefaultParser::parseFile() {
			TokenReader reader(impl_->lexer().getLexer(), *impl_);
			const auto namespaceDecl = NamespaceParser(reader).parseGlobalNamespace();
			impl_->context().fileCompleted(namespaceDecl);
			return impl_->context().errors().empty();
		}
		
		std::vector<ParseError> DefaultParser::getErrors() {
			return impl_->context().errors();
		}
		
	}
	
}

