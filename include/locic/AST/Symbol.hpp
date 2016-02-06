#ifndef LOCIC_AST_SYMBOL_HPP
#define LOCIC_AST_SYMBOL_HPP

#include <string>
#include <vector>

#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/ValueList.hpp>

namespace locic {
	
	namespace AST {
		
		class SymbolElement {
			public:
				SymbolElement(String n, Node<ValueList> t);
				~SymbolElement();
				
				const String& name() const;
				
				const Node<ValueList>& templateArguments() const;
				
			private:
				String name_;
				Node<ValueList> templateArguments_;
				
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
				
				void push_back(Node<SymbolElement> symbolElement) {
					list_.push_back(std::move(symbolElement));
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
				
				const String& trivialString() const {
					assert(isTrivial());
					return createName().first();
				}
				
				std::string toString() const;
				
				Name createName() const;
				
			private:
				explicit Symbol(bool isAbs)
				: isAbsolute_(isAbs) { }
				
				bool isAbsolute_;
				std::vector<Node<SymbolElement>> list_;
				
		};
		
	}
	
}

#endif
