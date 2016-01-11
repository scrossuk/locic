#ifndef LOCIC_PARSER_CONTEXT_HPP
#define LOCIC_PARSER_CONTEXT_HPP

#include <cstddef>
#include <string>
#include <vector>

#include <locic/AST.hpp>
#include <locic/Constant.hpp>
#include <locic/Debug/SourceLocation.hpp>
#include <locic/Support/String.hpp>

namespace locic{

	namespace Parser{
	
		struct ParseError {
			std::string message;
			Debug::SourceLocation location;
			
			ParseError(const std::string& m, const Debug::SourceLocation& l)
				: message(m), location(l) { }
		};
		
		class Context {
			public:
				Context(const StringHost& h, AST::NamespaceList& l, const std::string& n)
					: stringHost_(h), rootNamespaceList_(l), fileName_(h, n) { }
				
				const StringHost& stringHost() const {
					return stringHost_;
				}
				
				String fileName() const {
					return fileName_;
				}
				
				void error(const std::string& message, const Debug::SourceLocation& location) {
					errors_.push_back(ParseError(message, location));
				}
				
				void fileCompleted(const AST::Node<AST::NamespaceDecl>& namespaceNode) {
					rootNamespaceList_.push_back(namespaceNode);
				}
				
				const std::vector<ParseError>& errors() const {
					return errors_;
				}
				
			private:
				const StringHost& stringHost_;
				AST::NamespaceList& rootNamespaceList_;
				String fileName_;
				std::vector<ParseError> errors_;
				
		};
		
	}

}

#endif
