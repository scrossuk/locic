#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string>

#include <locic/Frontend/DiagnosticReceiver.hpp>
#include <locic/Frontend/Diagnostics.hpp>
#include <locic/Parser/Context.hpp>
#include <locic/Parser/DefaultParser.hpp>
#include <locic/Parser/NamespaceParser.hpp>
#include <locic/Parser/TokenReader.hpp>
#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/StringHost.hpp>

#include "LexLexer.hpp"

namespace locic {
	
	namespace Parser {
		
		class DefaultParserImpl: public DiagnosticReceiver {
		public:
			DefaultParserImpl(const StringHost& stringHost, AST::NamespaceList& rootNamespaceList,
			                  FILE * file, const std::string& fileName)
			: context_(stringHost, rootNamespaceList, fileName),
			lexer_(file, context_.fileName()) { }
			
			Context& context() {
				return context_;
			}
			
			LexLexer& lexer() {
				return lexer_;
			}
			
			void issueDiag(std::unique_ptr<Diag> diag,
			               const Debug::SourceLocation& location) {
				context().error(std::move(diag), location);
			}
			
		private:
			Context context_;
			LexLexer lexer_;
			
		};
		
		DefaultParser::DefaultParser(const StringHost& stringHost, AST::NamespaceList& rootNamespaceList,
		                             FILE * file, const std::string& fileName)
		: impl_(new DefaultParserImpl(stringHost, rootNamespaceList, file, fileName)) { }
		
		DefaultParser::~DefaultParser() { }
		
		bool DefaultParser::parseFile() {
			TokenReader reader(impl_->lexer().getLexer(), *impl_);
			const auto namespaceDecl = NamespaceParser(reader).parseGlobalNamespace();
			impl_->context().fileCompleted(namespaceDecl);
			return impl_->context().errors().empty();
		}
		
		const std::vector<ParseError>& DefaultParser::getErrors() {
			return impl_->context().errors();
		}
		
	}
	
}

