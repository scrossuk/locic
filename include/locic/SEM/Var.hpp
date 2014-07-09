#ifndef LOCIC_SEM_VAR_HPP
#define LOCIC_SEM_VAR_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include <locic/Map.hpp>

namespace locic {

	namespace SEM {
	
		class TemplateVar;
		class Type;
		
		typedef std::unordered_map<TemplateVar*, Type*> TemplateVarMap;
		
		class Var {
			public:
				static Var* Any(Type* constructType);
				
				static Var* Basic(Type* constructType, Type* type);
				
				static Var* Composite(Type* type, const std::vector<Var*>& children);
				
				enum Kind {
					ANY,
					BASIC,
					COMPOSITE
				};
				
				Kind kind() const;
				
				bool isAny() const;
				bool isBasic() const;
				bool isComposite() const;
				
				Type* constructType() const;
				Type* type() const;
				const std::vector<Var*>& children() const;
				
				Var* substitute(const TemplateVarMap& templateVarMap) const;
				
				std::string toString() const;
				
			private:
				Var();
				
				Kind kind_;
				Type* constructType_;
				Type* type_;
				std::vector<Var*> children_;
				
		};
		
	}
	
}

#endif
