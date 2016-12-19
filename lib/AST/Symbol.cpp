#include <string>
#include <vector>

#include <locic/Support/Name.hpp>

#include <locic/AST/Symbol.hpp>
#include <locic/AST/TypeDecl.hpp>
#include <locic/AST/ValueDecl.hpp>

namespace locic {
	
	namespace AST {
		
		SymbolElement::SymbolElement(String n, Node<ValueDeclList> t)
		: name_(std::move(n)), templateArguments_(std::move(t)) { }
		
		SymbolElement::~SymbolElement() { }
		
		SymbolElement SymbolElement::copy() const {
			return SymbolElement(*this);
		}
		
		const String& SymbolElement::name() const {
			return name_;
		}
		
		const Node<ValueDeclList>& SymbolElement::templateArguments() const {
			return templateArguments_;
		}
		
		std::string Symbol::toString() const {
			std::string str;
			
			if(isAbsolute()) {
				str += "::";
			}
			
			for (size_t i = 0; i < size(); i++) {
				if (i > 0) {
					str += "::";
				}
				
				str += at(i)->name().toString();
				
				const auto& templateArgs = at(i)->templateArguments();
				
				if (!templateArgs->empty()) {
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
			Name name = isAbsolute() ? Name::Absolute() : Name::Relative();
			
			for(size_t i = 0; i < list_.size(); i++) {
				name = name + list_.at(i)->name();
			}
			
			return name;
		}
		
	}
	
}

