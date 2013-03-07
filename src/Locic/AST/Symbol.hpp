#ifndef LOCIC_AST_SYMBOL_HPP
#define LOCIC_AST_SYMBOL_HPP

#include <string>
#include <vector>

#include <Locic/Constant.hpp>

#include <Locic/AST/Type.hpp>

namespace AST {
	
	class SymbolElement{
		public:
			inline SymbolElement(const std::string& name, const std::vector<Type*> templateArguments)
				: name_(name), templateArguments_(templateArguments){ }
			
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

	class Symbol{
		public:
			inline Symbol Absolute(){
				return Symbol(true);
			}
			
			inline Symbol Relative(){
				return Symbol(false);
			}
			
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
				return list_.first();
			}
			
			inline const SymbolElement& last() const {
				return list_.last();
			}
			
			inline bool isAbsolute() const {
				return isAbsolute_;
			}
			
			inline bool isRelative() const {
				return !isAbsolute_;
			}
			
			inline std::string toString() const {
				std::string str;
				
				if(isAbsolute()){
					str += "::";
				}
				
				for(size_t i = 0; i < symbol.size(); i++){
					if(i > 0){
						str += "::";
					}
					
					str += list_.at(i).name();
					
					const std::vector<Type*>& templateArgs = list_.at(i).templateArguments();
					
					if(!templateArgs.empty()){
						str += "<";
						for(size_t j = 0; j < templateArgs.size(); j++){
							str += templateArgs.at(j)->toString();
						}
						str += ">";
					}
				}
			}
			
		private:
			inline explicit Symbol(bool isAbsolute)
				: isAbsolute_(isAbsolute){ }
			
			inline Symbol(const Symbol& symbol, const SymbolElement& symbolElement)
				: isAbsolute_(symbol.isAbsolute()){
					for(size_t i = 0; i < symbol.size(); i++){
						list_.push_back(symbol.at(i));
					}
					list_.push_back(symbolElement);
				}
			
			bool isAbsolute_;
			std::vector<SymbolElement> list_;
		
	};
	
}

#endif
