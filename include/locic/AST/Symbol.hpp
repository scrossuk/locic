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
				inline SymbolElement(const std::string& n, const Node<TypeList>& t)
					: name_(n), templateArguments_(t) { }
					
				inline const std::string& name() const {
					return name_;
				}
				
				inline const Node<TypeList>& templateArguments() const {
					return templateArguments_;
				}
				
			private:
				std::string name_;
				Node<TypeList> templateArguments_;
				
		};
		
		class Symbol {
			public:
				static inline Symbol Absolute() {
					return Symbol(true);
				}
				
				static inline Symbol Relative() {
					return Symbol(false);
				}
				
				inline Symbol()
					: isAbsolute_(false) { }
					
				inline Symbol operator+(const Node<SymbolElement>& symbolElement) const {
					return Symbol(*this, symbolElement);
				}
				
				inline bool empty() const {
					return list_.empty();
				}
				
				inline size_t size() const {
					return list_.size();
				}
				
				inline const Node<SymbolElement>& at(size_t i) const {
					return list_.at(i);
				}
				
				inline const Node<SymbolElement>& first() const {
					return list_.front();
				}
				
				inline const Node<SymbolElement>& last() const {
					return list_.back();
				}
				
				inline bool isAbsolute() const {
					return isAbsolute_;
				}
				
				inline bool isRelative() const {
					return !isAbsolute_;
				}
				
				std::string toString() const;
				
				Name createName() const;
				
			private:
				inline explicit Symbol(bool isAbs)
					: isAbsolute_(isAbs) { }
					
				Symbol(const Symbol& symbol, const Node<SymbolElement>& symbolElement);
				
				bool isAbsolute_;
				std::vector<Node<SymbolElement>> list_;
				
		};
		
	}
	
}

#endif
