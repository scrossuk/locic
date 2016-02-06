#ifndef LOCIC_AST_EXCEPTIONINITIALIZER_HPP
#define LOCIC_AST_EXCEPTIONINITIALIZER_HPP

#include <string>
#include <vector>

#include <locic/AST/Node.hpp>

namespace locic {

	namespace AST {
	
		class Symbol;
		struct Value;
		typedef std::vector<Node<Value>> ValueList;
		
		struct ExceptionInitializer {
			enum Kind {
				NONE,
				INITIALIZE
			} kind;
			
			Node<Symbol> symbol;
			Node<ValueList> valueList;
			
			static ExceptionInitializer* None();
			
			static ExceptionInitializer*
			Initialize(Node<Symbol> symbol, Node<ValueList> valueList);
			
			std::string toString() const;
			
			ExceptionInitializer(Kind pKind) : kind(pKind) { }
			~ExceptionInitializer();
		};
		
	}
	
}

#endif
