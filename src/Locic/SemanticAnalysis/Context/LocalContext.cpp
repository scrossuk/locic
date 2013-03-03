#include <cassert>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/SEM.hpp>

#include <Locic/SemanticAnalysis/Exception.hpp>
#include <Locic/SemanticAnalysis/Context/LocalContext.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		LocalContext::LocalContext(Context& parentContext, SEM::Function* function)
			: nextVarId_(0), parentContext_(parentContext), function_(function) {
			// Lowest stack entry holds parameter variables.
			localVarStack_.push_back(std::map<std::string, SEM::Var*>());
		}
		
		LocalContext::~LocalContext() {
			assert(localVarStack_.size() == 1);
			assert(scopeStack_.size() == 0);
		}
		
		bool LocalContext::addFunction(const Name& name, SEM::Function* function) {
			assert(name.isAbsolute());
			return parentContext_.addFunction(name, function);
		}
		
		bool LocalContext::addNamespace(const Name& name, SEM::Namespace* nameSpace) {
			assert(name.isAbsolute());
			return parentContext_.addNamespace(name, nameSpace);
		}
		
		bool LocalContext::addTypeInstance(const Name& name, SEM::TypeInstance* typeInstance) {
			assert(name.isAbsolute());
			return parentContext_.addTypeInstance(name, typeInstance);
		}
		
		Name LocalContext::getName() {
			return function_->name();
		}
		
		SEM::Type* LocalContext::getReturnType() {
			return function_->type()->getFunctionReturnType();
		}
		
		SEM::NamespaceNode LocalContext::getNode(const Name& name) {
			return parentContext_.getNode(name);
		}
		
		SEM::TypeInstance* LocalContext::getThisTypeInstance() {
			return parentContext_.getThisTypeInstance();
		}
		
		void LocalContext::pushScope(SEM::Scope* scope) {
			assert(localVarStack_.size() == (scopeStack_.size() + 1));
			localVarStack_.push_back(std::map<std::string, SEM::Var*>());
			scopeStack_.push_back(scope);
		}
		
		void LocalContext::popScope() {
			assert(localVarStack_.size() == (scopeStack_.size() + 1));
			localVarStack_.pop_back();
			scopeStack_.pop_back();
		}
		
		bool LocalContext::defineFunctionParameter(const std::string& paramName, SEM::Var* paramVar) {
			std::pair<std::map<std::string, SEM::Var*>::iterator, bool> s =
				localVarStack_.front().insert(std::make_pair(paramName, paramVar));
			return s.second;
		}
		
		SEM::Var* LocalContext::defineLocalVar(const std::string& varName, SEM::Type* type) {
			assert(localVarStack_.size() >= 2);
			
			// Variable shadowing is not allowed.
			if(findLocalVar(varName) != NULL){
				throw LocalVariableShadowingException(varName);
			}
			
			SEM::Var* var = new SEM::Var(SEM::Var::LOCAL, nextVarId_++, type);
			
			std::pair<std::map<std::string, SEM::Var*>::iterator, bool> s =
				localVarStack_.back().insert(std::make_pair(varName, var));
			
			assert(s.second && "Local variable map insertion must succeed.");
			
			scopeStack_.back()->localVariables().push_back(var);
			
			return var;
		}
		
		SEM::Var* LocalContext::findLocalVar(const std::string& varName) {
			std::vector< std::map<std::string, SEM::Var*> >::reverse_iterator it;
			
			for(it = localVarStack_.rbegin(); it != localVarStack_.rend(); ++it) {
				std::map<std::string, SEM::Var*>::iterator mapIt = it->find(varName);
				
				if(mapIt != it->end()) {
					return mapIt->second;
				}
			}
			
			return NULL;
		}
		
		SEM::Var* LocalContext::getThisVar(const std::string& name) {
			return parentContext_.getThisVar(name);
		}
		
	}
	
}

