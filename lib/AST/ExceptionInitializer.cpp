#include <string>
#include <vector>

#include <locic/Support/String.hpp>

#include <locic/AST/ExceptionInitializer.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/Symbol.hpp>
#include <locic/AST/Value.hpp>

namespace locic {

	namespace AST {
	
		ExceptionInitializer* ExceptionInitializer::None() {
			return new ExceptionInitializer(NONE);
		}
		
		ExceptionInitializer* ExceptionInitializer::Initialize(Node<Symbol> symbol, Node<ValueList> valueList) {
			auto initializer = new ExceptionInitializer(INITIALIZE);
			initializer->symbol = std::move(symbol);
			initializer->valueList = std::move(valueList);
			return initializer;
		}
		
		ExceptionInitializer::~ExceptionInitializer() { }
		
		std::string ExceptionInitializer::toString() const {
			switch (kind) {
				case NONE:
					return "None";
				case INITIALIZE:
					return makeString("Initialize(symbol: %s, valueList: %s)",
						symbol.toString().c_str(),
						"[TODO]"
						);
				default:
					return "[UNKNOWN]";
			}
		}
		
	}
	
}

