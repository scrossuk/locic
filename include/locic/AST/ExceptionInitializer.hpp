#ifndef LOCIC_AST_EXCEPTIONINITIALIZER_HPP
#define LOCIC_AST_EXCEPTIONINITIALIZER_HPP

#include <string>
#include <vector>

#include <locic/AST/Node.hpp>

namespace locic {

	namespace AST {
	
		struct Symbol;
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
			
			static ExceptionInitializer* Initialize(const Node<Symbol>& symbol, const Node<ValueList>& valueList);
			
			std::string toString() const;
			
			inline ExceptionInitializer(Kind pKind)
				: kind(pKind) { }
		};
		
	}
	
}

#endif
