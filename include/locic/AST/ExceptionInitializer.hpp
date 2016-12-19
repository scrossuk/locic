#ifndef LOCIC_AST_EXCEPTIONINITIALIZER_HPP
#define LOCIC_AST_EXCEPTIONINITIALIZER_HPP

#include <string>
#include <vector>

#include <locic/AST/Node.hpp>

namespace locic {

	namespace AST {
	
		class Symbol;
		struct ValueDecl;
		typedef std::vector<Node<ValueDecl>> ValueDeclList;
		
		struct ExceptionInitializer {
			enum Kind {
				NONE,
				INITIALIZE
			} kind;
			
			Node<Symbol> symbol;
			Node<ValueDeclList> valueList;
			
			static ExceptionInitializer* None();
			
			static ExceptionInitializer*
			Initialize(Node<Symbol> symbol, Node<ValueDeclList> valueList);
			
			std::string toString() const;
			
			ExceptionInitializer(Kind pKind) : kind(pKind) { }
			~ExceptionInitializer();
		};
		
	}
	
}

#endif
