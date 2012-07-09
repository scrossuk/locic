#ifndef LOCIC_SEMANTICANALYSIS_CONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_CONTEXT_HPP

#include <cassert>
#include <cstddef>
#include <map>
#include <string>
#include <vector>
#include <Locic/SEM.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		class Context {
			public:
				virtual SEM::Function* getFunction(const std::string& name) = 0;
				
				virtual SEM::TypeInstance* getTypeInstance(const std::string& name) = 0;
				
				// Returns true if type instance wasn't already referenced.
				virtual bool referTypeInstance(SEM::TypeInstance* typeInstance) = 0;
				
				virtual SEM::Var * getThisVar(const std::string& name) = 0;
				
		};
		
		class GlobalContext: public Context {
			public:
				inline GlobalContext() { }
				
				inline bool addFunction(const std::string& name, SEM::Function* function) {
					std::pair<std::map<std::string, SEM::Function*>::iterator, bool> s =
					    functions_.insert(std::make_pair(name, function));
					return s.second;
				}
				
				inline SEM::Function* getFunction(const std::string& name) {
					std::map<std::string, SEM::Function*>::iterator it =
					    functions_.find(name);
					return (it != functions_.end()) ? it->second : NULL;
				}
				
				inline bool addTypeInstance(const std::string& name, SEM::TypeInstance* typeInstance) {
					std::pair<std::map<std::string, SEM::TypeInstance*>::iterator, bool> s =
					    typeInstances_.insert(std::make_pair(name, typeInstance));
					return s.second;
				}
				
				inline SEM::TypeInstance* getTypeInstance(const std::string& name) {
					std::map<std::string, SEM::TypeInstance*>::iterator it =
					    typeInstances_.find(name);
					return (it != typeInstances_.end()) ? it->second : NULL;
				}
				
				inline bool referTypeInstance(SEM::TypeInstance* typeInstance) {
					return false;
				}
				
				inline SEM::Var * getThisVar(const std::string& name){
					return NULL;
				}
				
			private:
				std::map<std::string, SEM::Function*> functions_;
				std::map<std::string, SEM::TypeInstance*> typeInstances_;
				
		};
		
		class ModuleContext: public Context {
			public:
				inline ModuleContext(Context& parentContext, SEM::Module* module)
					: parentContext_(parentContext), module_(module) { }
					
				inline SEM::Function* getFunction(const std::string& name) {
					SEM::Function* function = parentContext_.getFunction(name);
					
					if(function != NULL) {
						// Modules need to know what functions they use.
						module_->functions.insert(std::make_pair(name, function));
					}
					
					return function;
				}
				
				inline SEM::TypeInstance* getTypeInstance(const std::string& name) {
					SEM::TypeInstance* typeInstance = parentContext_.getTypeInstance(name);
					
					if(typeInstance != NULL) {
						referTypeInstance(typeInstance);
					}
					
					return typeInstance;
				}
				
				inline bool referTypeInstance(SEM::TypeInstance * typeInstance){
					std::pair<std::map<std::string, SEM::TypeInstance *>::iterator, bool> s;
					s = module_->typeInstances.insert(std::make_pair(typeInstance->name, typeInstance));
					return s.second;
				}
				
				inline SEM::Var * getThisVar(const std::string& name){
					return NULL;
				}
				
			private:
				Context& parentContext_;
				SEM::Module* module_;
				
		};
		
		class TypeInstanceContext: public Context {
			public:
				inline TypeInstanceContext(Context& parentContext, SEM::TypeInstance * typeInstance)
					: parentContext_(parentContext), typeInstance_(typeInstance) {
					assert(typeInstance->typeEnum == SEM::TypeInstance::CLASSDEF);	
				}
				
				inline SEM::Function* getFunction(const std::string& name) {
					return parentContext_.getFunction(name);
				}
				
				inline SEM::TypeInstance* getTypeInstance(const std::string& name) {
					return parentContext_.getTypeInstance(name);
				}
				
				inline bool referTypeInstance(SEM::TypeInstance * typeInstance){
					return parentContext_.referTypeInstance(typeInstance);
				}
				
				inline SEM::Var * getThisVar(const std::string& name){
					for(std::size_t i = 0; i < typeInstance_->variableNames.size(); i++){
						if(name == typeInstance_->variableNames.at(i)){
							return typeInstance_->variables.at(i);
						}
					}
					return parentContext_.getThisVar(name);
				}
				
			private:
				Context& parentContext_;
				SEM::TypeInstance * typeInstance_;
				
		};
		
		class LocalContext: public Context {
			public:
				inline LocalContext(Context& parentContext, SEM::Function * function)
					: nextVarId_(0), parentContext_(parentContext), function_(function) {
					// Lowest stack entry holds parameter variables.
					localVarStack_.push_back(std::map<std::string, SEM::Var*>());
				}
				
				inline ~LocalContext() {
					assert(localVarStack_.size() == 1);
					assert(scopeStack_.size() == 0);
				}
				
				inline SEM::Type * getReturnType() {
					return function_->type->functionType.returnType;
				}
				
				inline SEM::Function* getFunction(const std::string& name) {
					return parentContext_.getFunction(name);
				}
				
				inline SEM::TypeInstance* getTypeInstance(const std::string& name) {
					return parentContext_.getTypeInstance(name);
				}
				
				inline bool referTypeInstance(SEM::TypeInstance * typeInstance){
					return parentContext_.referTypeInstance(typeInstance);
				}
				
				inline void pushScope(SEM::Scope* scope) {
					assert(localVarStack_.size() == (scopeStack_.size() + 1));
					localVarStack_.push_back(std::map<std::string, SEM::Var*>());
					scopeStack_.push_back(scope);
				}
				
				inline void popScope() {
					assert(localVarStack_.size() == (scopeStack_.size() + 1));
					localVarStack_.pop_back();
					scopeStack_.pop_back();
				}
				
				inline bool defineFunctionParameter(const std::string& paramName, SEM::Var* paramVar) {
					std::pair<std::map<std::string, SEM::Var*>::iterator, bool> s =
					    localVarStack_.front().insert(std::make_pair(paramName, paramVar));
					return s.second;
				}
				
				inline SEM::Var* defineLocalVar(const std::string& varName, SEM::Type* type) {
					assert(localVarStack_.size() >= 2);
					SEM::Var* var = new SEM::Var(SEM::Var::LOCAL, nextVarId_++, type);
					
					std::pair<std::map<std::string, SEM::Var*>::iterator, bool> s =
					    localVarStack_.back().insert(std::make_pair(varName, var));
					
					if(s.second) scopeStack_.back()->localVariables.push_back(var);
					    
					return s.second ? var : NULL;
				}
				
				inline SEM::Var* findLocalVar(const std::string& varName) {
					std::vector< std::map<std::string, SEM::Var*> >::reverse_iterator it;
					
					for(it = localVarStack_.rbegin(); it != localVarStack_.rend(); ++it) {
						std::map<std::string, SEM::Var*>::iterator mapIt = it->find(varName);
						
						if(mapIt != it->end()) {
							return mapIt->second;
						}
					}
					
					return NULL;
				}
				
				inline SEM::Var * getThisVar(const std::string& name){
					return parentContext_.getThisVar(name);
				}
				
			private:
				std::size_t nextVarId_;
				Context& parentContext_;
				SEM::Function * function_;
				std::vector< std::map<std::string, SEM::Var*> > localVarStack_;
				std::vector<SEM::Scope*> scopeStack_;
				
		};
		
	}
	
}

#endif
