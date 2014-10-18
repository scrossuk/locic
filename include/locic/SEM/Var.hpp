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
		
		typedef std::unordered_map<TemplateVar*, const Type*> TemplateVarMap;
		
		class Var {
			public:
				static Var* Any(const Type* constructType);
				
				static Var* Basic(const Type* constructType, const Type* type);
				
				static Var* Composite(const Type* type, const std::vector<Var*>& children);
				
				enum Kind {
					ANY,
					BASIC,
					COMPOSITE
				};
				
				Kind kind() const;
				
				bool isAny() const;
				bool isBasic() const;
				bool isComposite() const;
				
				const Type* constructType() const;
				const Type* type() const;
				const std::vector<Var*>& children() const;
				
				Var* substitute(const TemplateVarMap& templateVarMap) const;
				
				std::string toString() const;
				
			private:
				Var();
				
				Kind kind_;
				const Type* constructType_;
				const Type* type_;
				std::vector<Var*> children_;
				
		};
		
	}
	
}

#endif
