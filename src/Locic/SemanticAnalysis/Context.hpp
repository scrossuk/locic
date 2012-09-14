#ifndef LOCIC_SEMANTICANALYSIS_CONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_CONTEXT_HPP

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>
#include <Locic/Map.hpp>
#include <Locic/SEM.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		class Context {
			public:
				virtual SEM::Function* getFunction(const std::string& name, bool searchParent = true) = 0;
				
				virtual SEM::TypeInstance* getTypeInstance(const std::string& name) = 0;
				
				virtual SEM::TypeInstance* getThisTypeInstance() = 0;
				
				virtual SEM::Var * getThisVar(const std::string& name) = 0;
				
		};
		
		class StructuralContext: public Context{
			public:
				virtual bool addFunction(const std::string& name, SEM::Function* function) = 0;
				
				virtual bool addTypeInstance(const std::string& name, SEM::TypeInstance* typeInstance) = 0;
			
		};
		
		class GlobalContext: public StructuralContext {
			public:
				inline GlobalContext(SEM::Namespace * rootNamespace)
					: rootNamespace_(rootNamespace){ }
				
				inline bool addFunction(const std::string& name, SEM::Function* function) {
					return rootNamespace_->children.tryInsert(name, SEM::NamespaceNode::Function(function));
				}
				
				inline SEM::Function* getFunction(const std::string& name, bool searchParent = true) {
					Optional<SEM::NamespaceNode *> node = rootNamespace_->children.tryGet(name);
					return node.hasValue() ? (node.getValue()->getFunction()) : NULL;
				}
				
				inline bool addTypeInstance(const std::string& name, SEM::TypeInstance* typeInstance) {
					return rootNamespace_->children.tryInsert(name, SEM::NamespaceNode::TypeInstance(typeInstance));
				}
				
				inline SEM::TypeInstance* getTypeInstance(const std::string& name) {
					Optional<SEM::NamespaceNode *> node = rootNamespace_->children.tryGet(name);
					return node.hasValue() ? (node.getValue()->getTypeInstance()) : NULL;
				}
				
				inline SEM::TypeInstance* getThisTypeInstance(){
					return NULL;
				}
				
				inline SEM::Var * getThisVar(const std::string& name){
					return NULL;
				}
				
			private:
				SEM::Namespace * rootNamespace_;
				
		};
		
		class ModuleContext: public StructuralContext {
			public:
				inline ModuleContext(StructuralContext& parentContext, SEM::Module* module)
					: parentContext_(parentContext), module_(module) { }
					
				inline bool addFunction(const std::string& name, SEM::Function* function) {
					return parentContext_.addFunction(name, function);
				}
				
				inline SEM::Function* getFunction(const std::string& name, bool searchParent = true) {
					return parentContext_.getFunction(name);
				}
				
				inline bool addTypeInstance(const std::string& name, SEM::TypeInstance* typeInstance) {
					return parentContext_.addTypeInstance(name, typeInstance);
				}
				
				inline SEM::TypeInstance* getTypeInstance(const std::string& name) {
					return parentContext_.getTypeInstance(name);
				}
				
				inline SEM::TypeInstance* getThisTypeInstance(){
					return NULL;
				}
				
				inline SEM::Var * getThisVar(const std::string& name){
					return NULL;
				}
				
			private:
				StructuralContext& parentContext_;
				SEM::Module* module_;
				
		};
		
		/*class NamespaceContext: public Context {
			public:
				inline NamespaceContext(Context& parentContext, SEM::Namespace * nameSpace)
					: parentContext_(parentContext), nameSpace_(nameSpace){ }
				
				
			
		};*/
		
		class TypeInstanceContext: public StructuralContext {
			public:
				inline TypeInstanceContext(StructuralContext& parentContext, SEM::TypeInstance * typeInstance)
					: parentContext_(parentContext), typeInstance_(typeInstance) { }
				
				inline bool addFunction(const std::string& name, SEM::Function* function) {
					return typeInstance_->methods.tryInsert(name, function);
				}
				
				inline SEM::Function* getFunction(const std::string& name, bool searchParent = true) {
					Optional<SEM::Function *> function = typeInstance_->methods.tryGet(name);
					return function.hasValue() ? function.getValue() :
						(searchParent ? parentContext_.getFunction(name) : NULL);
				}
				
				inline bool addTypeInstance(const std::string& name, SEM::TypeInstance* typeInstance) {
					// No nested type instances.
					return false;
				}
				
				inline SEM::TypeInstance* getTypeInstance(const std::string& name) {
					return parentContext_.getTypeInstance(name);
				}
				
				inline SEM::TypeInstance* getThisTypeInstance(){
					return typeInstance_;
				}
				
				inline SEM::Var * getThisVar(const std::string& name){
					Optional<SEM::Var *> varResult = typeInstance_->variables.tryGet(name);
					return varResult.hasValue() ? varResult.getValue() : parentContext_.getThisVar(name);
				}
				
			private:
				StructuralContext& parentContext_;
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
				
				inline SEM::Function* getFunction(const std::string& name, bool searchParent = true) {
					return parentContext_.getFunction(name);
				}
				
				inline SEM::TypeInstance* getThisTypeInstance(){
					return parentContext_.getThisTypeInstance();
				}
				
				inline SEM::TypeInstance* getTypeInstance(const std::string& name) {
					return parentContext_.getTypeInstance(name);
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
