#ifndef LOCIC_SEM_FUNCTION_HPP
#define LOCIC_SEM_FUNCTION_HPP

#include <map>
#include <string>
#include <unordered_map>
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
		
		typedef std::unordered_map<TemplateVar*, const Type*> TemplateVarMap;
		
		class Function {
			public:
				Function(const Name& pName, ModuleScope* pModuleScope);
				
				const Name& name() const;
				
				void setType(const Type* pType);
				const Type* type() const;
				
				ModuleScope* moduleScope() const;
				
				bool isDeclaration() const;
				
				bool isDefinition() const;
                                
                                void setPrimitive(bool pIsPrimitive);
                                bool isPrimitive() const;
				
				void setMethod(bool pIsMethod);
				bool isMethod() const;
				
				void setStaticMethod(bool pIsStaticMethod);
				bool isStaticMethod() const;
				
				void setConstMethod(bool pIsConstMethod);
				bool isConstMethod() const;
				
				std::vector<TemplateVar*>& templateVariables();
				const std::vector<TemplateVar*>& templateVariables() const;
				
				std::map<std::string, TemplateVar*>& namedTemplateVariables();
				const std::map<std::string, TemplateVar*>& namedTemplateVariables() const;
				
				void setParameters(std::vector<Var*> pParameters);
				const std::vector<Var*>& parameters() const;
				
				std::map<std::string, Var*>& namedVariables();
				const std::map<std::string, Var*>& namedVariables() const;
				
				void setScope(Scope* newScope);
				const Scope& scope() const;
				
				Function* createTemplatedDecl() const;
				
				Function* fullSubstitute(const Name& declName, const TemplateVarMap& templateVarMap) const;
				
				std::string toString() const;
				
			private:
				bool isPrimitive_;
				bool isMethod_, isStaticMethod_, isConstMethod_;
				const Type* type_;
				Name name_;
				
				std::vector<TemplateVar*> templateVariables_;
				std::map<std::string, TemplateVar*> namedTemplateVariables_;
				
				std::vector<Var*> parameters_;
				std::map<std::string, Var*> namedVariables_;
				
				ModuleScope* moduleScope_;
				Scope* scope_;
				
		};
		
	}
	
}

#endif
