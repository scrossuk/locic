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
			
			inline Error(const std::string& m, const Debug::SourceLocation& l)
				: message(m), location(l) { }
		};
		
		class Context{
			public:
				inline Context(AST::NamespaceList& l, const std::string& n)
					: rootNamespaceList_(l), fileName_(n),
					stringKind_(locic::Constant::C_STRING),
					nextAnonymousVariable_(0), column_(1) { }
				
				inline const std::string& fileName() const {
					return fileName_;
				}
				
				inline void error(const std::string& message, const Debug::SourceLocation& location) {
					errors_.push_back(Error(message, location));
				}
				
				inline void fileCompleted(const AST::Node<AST::Namespace>& namespaceNode) {
					rootNamespaceList_.push_back(namespaceNode);
				}
				
				inline std::string getAnonymousVariableName() {
					return makeString("__anon_var_%llu", (unsigned long long) nextAnonymousVariable_++);
				}
				
				inline const std::vector<Error>& errors() const {
					return errors_;
				}
				
				inline size_t columnPosition() const {
					return column_;
				}
				
				inline void advanceColumn(size_t columnIncrease) {
					column_ += columnIncrease;
				}
				
				inline void resetColumn() {
					column_ = 1;
				}
				
				inline locic::Constant::StringKind getStringKind() const {
					return stringKind_;
				}
				
				inline const std::string& getStringConstant() const {
					return stringConstant_;
				}
				
				inline void appendStringConstant(const std::string& appendString) {
					stringConstant_ += appendString;
				}
				
				inline void resetStringConstant(locic::Constant::StringKind stringKind) {
					stringKind_ = stringKind;
					stringConstant_ = "";
				}
				
			private:
				AST::NamespaceList& rootNamespaceList_;
				std::string fileName_;
				std::vector<Error> errors_;
				locic::Constant::StringKind stringKind_;
				std::string stringConstant_;
				size_t nextAnonymousVariable_;
				size_t column_;
				
		};
		
	}

}

#endif
