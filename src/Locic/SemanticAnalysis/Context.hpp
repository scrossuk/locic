#ifndef LOCIC_SEMANTICANALYSIS_CONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_CONTEXT_HPP

#include <list>
#include <map>
#include <string>
#include <Locic/SEM.hpp>

// Holds information about a scope during its conversion.
typedef struct Locic_SemanticContext_Scope{
	// The resulting semantic scope that will be generated.
	SEM_Scope * scope;
	
	// Map from local variable names to their semantic variables.
	Locic_StringMap localVariables;
} Locic_SemanticContext_Scope;

// Holds information about a function during its conversion.
typedef struct Locic_SemanticContext_Function{
	// Map from parameter variable names to their semantic variables.
	Locic_StringMap * parameters;
	
	// The id to be assigned to the next local variable.
	size_t nextVarId;
} Locic_SemanticContext_Function;

// Holds information about a class during its conversion.
typedef struct Locic_SemanticContext_Class{
	std::map<std::string, SEM::Var *> memberVariables;
} Locic_SemanticContext_Class;

// Manages conversion from AST to SEM structure.
typedef struct Locic_SemanticContext{
	// The current module.
	SEM_Module * module;

	// The current type instance (e.g. class declaration).
	// (NULL if current function does not have a parent type.)
	SEM_TypeInstance * functionParentType;
	
	// The current function declaration.
	SEM_FunctionDecl * functionDecl;
	
	// Map from function names to their declarations for all modules.
	Locic_StringMap functionDeclarations;
	
	// Map from types names to their type instances for all modules.
	Locic_StringMap typeInstances;
	
	// Information about the class containing the current function.
	// (e.g. member variables).
	Locic_SemanticContext_Class classContext;
	
	// Information about the current function.
	// (e.g. parameters).
	Locic_SemanticContext_Function functionContext;
	
	// A stack of scope contexts.
	Locic_Stack * scopeStack;
} Locic_SemanticContext;

namespace Locic{

	namespace SemanticAnalysis{
	
		class TypeInfoContext{
			public:
				virtual SEM::TypeInstance * getTypeInstance(const std::string& name) = 0;
			
		};
	
		class GlobalContext: public TypeInfoContext{
			public:
				GlobalContext();
				
				bool addTypeInstance(const std::string& name, SEM::TypeInstance * typeInstance);
				
				SEM::TypeInstance * getTypeInstance(const std::string& name);
			
		};
		
		class ModuleContext: public TypeInfoContext{
			public:
				ModuleContext(GlobalContext& globalContext);
				
				SEM::FunctionDecl * getFunction(const std::string& name);
				
				SEM::TypeInstance * getTypeInstance(const std::string& name);
			
		};
		
		class LocalContext: public TypeInfoContext{
			public:
				LocalContext(ModuleContext& moduleContext);
				
				SEM::FunctionDecl * getFunction(const std::string& name);
				
				SEM::TypeInstance * getTypeInstance(const std::string& name);
				
				void pushScope(SEM::Scope * scope);
				
				void popScope();
				
				bool defineFunctionParameter(const std::string& paramName, SEM::Var * paramVar);
				
				SEM::Var * defineLocalVar(const std::string& varName, SEM::Type * type);
				
				SEM::Var * findLocalVar(const std::string& varName);
				
			private:
				ModuleContext& moduleContext_;
				std::stack< std::map<std::string, SEM::Var *> > localVarStack;
				std::stack<SEM::Scope *> scopeStack_;
			
		};
		
		class SemanticContext{
			public:
				SemanticContext();
				
				bool addTypeInstance(const std::string& name, SEM::TypeInstance * typeInstance);
				
				SEM::TypeInstance * getTypeInstance(const std::string& name);
				
				void setModule(SEM::Module * module);
				
				void startFunctionParentType(SEM::TypeInstance * typeInstance);
				
				void endFunctionParentType();
				
				void startFunction(SEM::FunctionDecl * functionDecl);
				
				void endFunction();
				
				void pushScope(SEM::Scope * scope);
				
				void popScope();
				
				SEM::Scope * topScope();
				
				SEM::Var * defineLocalVar(const std::string& varName, SEM::Type * type);
				
				SEM::Var * findLocalVar(const std::string& varName);
			
			private:
				
				
		};	
		
	}

}

#endif
