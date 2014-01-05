#ifndef LOCIC_SEM_FUNCTION_HPP
#define LOCIC_SEM_FUNCTION_HPP

#include <string>
#include <vector>

#include <locic/Name.hpp>

namespace locic {
	
	template <typename Key, typename Value>
	class Map;

	namespace SEM {
	
		class Scope;
		class TemplateVar;
		class Type;
		class Var;
		
		class Function {
			public:
				static Function* Decl(bool isMethod, bool isStatic, bool isConst, Type* type,
					const Name& name, const std::vector<Var*>& parameters);
				
				static Function* Def(bool isMethod, bool isStatic, bool isConst, Type* type,
					const Name& name, const std::vector<Var*>& parameters, Scope* scope);
				
				static Function* DefDefault(bool isStatic, Type* type, const Name& name);
				
				const Name& name() const;
				
				Type* type() const;
				
				bool isDeclaration() const;
				
				bool isDefinition() const;
				
				bool isMethod() const;
				
				bool isStaticMethod() const;
				
				bool isConstMethod() const;
				
				bool hasDefaultImplementation() const;
				
				const std::vector<Var*>& parameters() const;
				
				const Scope& scope() const;
				
				Function* createDecl() const;
				
				Function* fullSubstitute(const Name& declName, const Map<TemplateVar*, Type*>& templateVarMap) const;
				
				void setScope(Scope* newScope);
				
				std::string toString() const;
				
			private:
				Function(bool isM, bool isS, bool isC, bool hasD, Type* t, const Name& n, const std::vector<Var*>& p, Scope* s);
				
				bool isMethod_, isStatic_, isConst_;
				bool hasDefaultImplementation_;
				Type* type_;
				Name name_;
				std::vector<Var*> parameters_;
				Scope* scope_;
				
		};
		
	}
	
}

#endif
