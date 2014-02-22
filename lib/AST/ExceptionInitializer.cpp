#include <string>
#include <vector>

#include <locic/String.hpp>

#include <locic/AST/ExceptionInitializer.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/Symbol.hpp>
#include <locic/AST/Value.hpp>

namespace locic {

	namespace AST {
	
		ExceptionInitializer* ExceptionInitializer::None() {
			return new ExceptionInitializer(NONE);
		}
		
		ExceptionInitializer* ExceptionInitializer::Initialize(const Node<Symbol>& symbol, const Node<ValueList>& valueList) {
			auto initializer = new ExceptionInitializer(NONE);
			initializer->symbol = symbol;
			initializer->valueList = valueList;
			return initializer;
		}
		
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

