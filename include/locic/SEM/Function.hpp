#ifndef LOCIC_SEM_FUNCTION_HPP
#define LOCIC_SEM_FUNCTION_HPP

#include <map>
#include <string>
#include <vector>

#include <locic/Name.hpp>

namespace locic {
	
	template <typename Key, typename Value>
	class Map;

	namespace SEM {
	
		class ModuleScope;
		class Scope;
		class TemplateVar;
		class Type;
		class Var;
		
		class Function {
			public:
				static Function* Decl(bool isMethod, bool isStatic, bool isConst, Type* type,
					const Name& name, const std::vector<Var*>& parameters, ModuleScope* moduleScope);
				
				const Name& name() const;
				
				Type* type() const;
				
				ModuleScope* moduleScope() const;
				
				bool isDeclaration() const;
				
				bool isDefinition() const;
				
				bool isMethod() const;
				
				bool isStaticMethod() const;
				
				bool isConstMethod() const;
				
				const std::vector<Var*>& parameters() const;
				
				std::map<std::string, Var*>& namedVariables();
				const std::map<std::string, Var*>& namedVariables() const;
				
				const Scope& scope() const;
				
				Function* createTemplatedDecl() const;
				
				Function* fullSubstitute(const Name& declName, const Map<TemplateVar*, Type*>& templateVarMap) const;
				
				void setScope(Scope* newScope);
				
				std::string toString() const;
				
			private:
				Function(bool isM, bool isS, bool isC, Type* t, const Name& n, const std::vector<Var*>& p, ModuleScope* m, Scope* s);
				
				bool isMethod_, isStatic_, isConst_;
				Type* type_;
				Name name_;
				std::vector<Var*> parameters_;
				std::map<std::string, Var*> namedVariables_;
				ModuleScope* moduleScope_;
				Scope* scope_;
				
		};
		
	}
	
}

#endif
