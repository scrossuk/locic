#ifndef LOCIC_AST_SYMBOL_HPP
#define LOCIC_AST_SYMBOL_HPP

#include <string>
#include <vector>

#include <Locic/Name.hpp>

namespace AST {

	struct Type;
	
	class SymbolElement {
		public:
			inline SymbolElement(const std::string& n, const std::vector<Type*> t)
				: name_(n), templateArguments_(t) { }
				
			inline const std::string& name() const {
				return name_;
			}
			
			inline const std::vector<Type*>& templateArguments() const {
				return templateArguments_;
			}
			
		private:
			std::string name_;
			std::vector<Type*> templateArguments_;
			
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
				: isAbsolute_(false){ }
			
			inline Symbol operator+(const SymbolElement& symbolElement) const {
				return Symbol(*this, symbolElement);
			}
			
			inline bool empty() const {
				return list_.empty();
			}
			
			inline size_t size() const {
				return list_.size();
			}
			
			inline const SymbolElement& at(size_t i) const {
				return list_.at(i);
			}
			
			inline const SymbolElement& first() const {
				return list_.front();
			}
			
			inline const SymbolElement& last() const {
				return list_.back();
			}
			
			inline bool isAbsolute() const {
				return isAbsolute_;
			}
			
			inline bool isRelative() const {
				return !isAbsolute_;
			}
			
			std::string toString() const;
			
			Locic::Name createName() const;
			
		private:
			inline explicit Symbol(bool isAbs)
				: isAbsolute_(isAbs) { }
				
			Symbol(const Symbol& symbol, const SymbolElement& symbolElement);
			
			bool isAbsolute_;
			std::vector<SymbolElement> list_;
			
	};
	
}

#endif
