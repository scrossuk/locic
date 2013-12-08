#include <string>
#include <vector>

#include <Locic/Name.hpp>

#include <Locic/AST/Symbol.hpp>
#include <Locic/AST/Type.hpp>

namespace Locic {

	namespace AST {
	
		std::string Symbol::toString() const {
			std::string str;
			
			if(isAbsolute()) {
				str += "::";
			}
			
			for(size_t i = 0; i < size(); i++) {
				if(i > 0) {
					str += "::";
				}
				
				str += at(i)->name();
				
				const auto& templateArgs = at(i)->templateArguments();
				
				if(!templateArgs->empty()) {
					str += "<";
					
					for(size_t j = 0; j < templateArgs->size(); j++) {
						str += templateArgs->at(j)->toString();
					}
					
					str += ">";
				}
			}
			
			return str;
		}
		
		Name Symbol::createName() const {
			Name name = isAbsolute()
							   ? Name::Absolute()
							   : Name::Relative();
							   
			for(size_t i = 0; i < list_.size(); i++) {
				name = name + list_.at(i)->name();
			}
			
			return name;
		}
		
		Symbol::Symbol(const Symbol& symbol, const Node<SymbolElement>& symbolElement)
			: isAbsolute_(symbol.isAbsolute()) {
			for(size_t i = 0; i < symbol.size(); i++) {
				list_.push_back(symbol.at(i));
			}
			
			list_.push_back(symbolElement);
		}
		
	}
	
}

