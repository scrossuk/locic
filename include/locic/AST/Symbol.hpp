#ifndef LOCIC_AST_SYMBOL_HPP
#define LOCIC_AST_SYMBOL_HPP

#include <string>
#include <vector>

#include <locic/Name.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/Type.hpp>

namespace locic {

	namespace AST {
	
		class SymbolElement {
			public:
				SymbolElement(const std::string& n, const Node<TypeList>& t)
					: name_(n), templateArguments_(t) { }
					
				const std::string& name() const {
					return name_;
				}
				
				const Node<TypeList>& templateArguments() const {
					return templateArguments_;
				}
				
			private:
				std::string name_;
				Node<TypeList> templateArguments_;
				
		};
		
		class Symbol {
			public:
				static Symbol Absolute() {
					return Symbol(true);
				}
				
				static Symbol Relative() {
					return Symbol(false);
				}
				
				Symbol()
					: isAbsolute_(false) { }
					
				Symbol operator+(const Node<SymbolElement>& symbolElement) const {
					return Symbol(*this, symbolElement);
				}
				
				bool empty() const {
					return list_.empty();
				}
				
				size_t size() const {
					return list_.size();
				}
				
				const Node<SymbolElement>& at(size_t i) const {
					return list_.at(i);
				}
				
				const Node<SymbolElement>& first() const {
					return list_.front();
				}
				
				const Node<SymbolElement>& last() const {
					return list_.back();
				}
				
				bool isAbsolute() const {
					return isAbsolute_;
				}
				
				bool isRelative() const {
					return !isAbsolute_;
				}
				
				bool isTrivial() const {
					return size() == 1 && first()->templateArguments()->empty();
				}
				
				std::string trivialString() const {
					assert(isTrivial());
					return createName().first();
				}
				
				std::string toString() const;
				
				Name createName() const;
				
			private:
				explicit Symbol(bool isAbs)
					: isAbsolute_(isAbs) { }
					
				Symbol(const Symbol& symbol, const Node<SymbolElement>& symbolElement);
				
				bool isAbsolute_;
				std::vector<Node<SymbolElement>> list_;
				
		};
		
	}
	
}

#endif
