#ifndef LOCIC_PARSER_CONTEXT_HPP
#define LOCIC_PARSER_CONTEXT_HPP

#include <cstddef>
#include <string>
#include <vector>

#include <locic/AST.hpp>
#include <locic/Constant.hpp>
#include <locic/Debug/SourceLocation.hpp>

namespace locic{

	namespace Parser{
	
		struct Error{
			std::string message;
			Debug::SourceLocation location;
			
			Error(const std::string& m, const Debug::SourceLocation& l)
				: message(m), location(l) { }
		};
		
		class Context{
			public:
				Context(const StringHost& h, AST::NamespaceList& l, const std::string& n)
					: stringHost_(h), rootNamespaceList_(l), fileName_(n),
					nextAnonymousVariable_(0), column_(1),
					byteOffset_(0), columnByteOffset_(0) { }
				
				const std::string& fileName() const {
					return fileName_;
				}
				
				void error(const std::string& message, const Debug::SourceLocation& location) {
					errors_.push_back(Error(message, location));
				}
				
				void fileCompleted(const AST::Node<AST::Namespace>& namespaceNode) {
					rootNamespaceList_.push_back(namespaceNode);
				}
				
				std::string getAnonymousVariableName() {
					return makeString("__anon_var_%llu", (unsigned long long) nextAnonymousVariable_++);
				}
				
				const std::vector<Error>& errors() const {
					return errors_;
				}
				
				size_t columnPosition() const {
					return column_;
				}
				
				void advanceColumn(size_t columnIncrease) {
					column_ += columnIncrease;
				}
				
				void resetColumn() {
					column_ = 1;
					columnByteOffset_ = byteOffset_;
				}
				
				void addByteOffset(size_t pByteOffset) {
					byteOffset_ += pByteOffset;
				}
				
				size_t byteOffset() const {
					return byteOffset_;
				}
				
				size_t columnByteOffset() const {
					return columnByteOffset_;
				}
				
				String releaseStringConstant() {
					String result(stringHost_, std::move(stringConstant_));
					stringConstant_ = "";
					return result;
				}
				
				void appendStringConstant(const std::string& appendString) {
					stringConstant_ += appendString;
				}
				
				String getCString(const char* const cStringValue) const {
					return String(stringHost_, cStringValue);
				}
				
				String getString(std::string stringValue) const {
					return String(stringHost_, std::move(stringValue));
				}
				
			private:
				const StringHost& stringHost_;
				AST::NamespaceList& rootNamespaceList_;
				std::string fileName_;
				std::vector<Error> errors_;
				std::string stringConstant_;
				size_t nextAnonymousVariable_;
				size_t column_;
				size_t byteOffset_;
				size_t columnByteOffset_;
				
		};
		
	}

}

#endif
