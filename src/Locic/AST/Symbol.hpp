#ifndef LOCIC_AST_SYMBOL_HPP
#define LOCIC_AST_SYMBOL_HPP

#include <string>
#include <vector>

#include <Locic/Constant.hpp>
#include <Locic/Name.hpp>

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
			
		
	};
	
}

#endif
