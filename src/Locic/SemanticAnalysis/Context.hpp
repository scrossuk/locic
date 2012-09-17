#ifndef LOCIC_SEMANTICANALYSIS_CONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_CONTEXT_HPP

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/SEM.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		class Context {
			public:
				virtual Name getName() = 0;
				
				virtual SEM::Function* getFunction(const Name& name, bool searchParent = true) = 0;
				
				virtual SEM::TypeInstance* getTypeInstance(const Name& name) = 0;
				
				virtual SEM::TypeInstance* getThisTypeInstance() = 0;
				
				virtual SEM::Var * getThisVar(const std::string& name) = 0;
				
		};
		
		class StructuralContext: public Context{
			public:
				virtual bool addFunction(const std::string& name, SEM::Function* function, bool isMethod = false) = 0;
				
				virtual bool addTypeInstance(const std::string& name, SEM::TypeInstance* typeInstance) = 0;
			
		};
		
		class GlobalContext: public StructuralContext {
			public:
				inline GlobalContext(SEM::Namespace * rootNamespace)
					: rootNamespace_(rootNamespace){ }
					
				inline Name getName(){
					return Name();
				}
				
				inline bool addFunction(const std::string& name, SEM::Function* function, bool isMethod = false) {
					assert(!isMethod);
					return rootNamespace_->children.tryInsert(name, SEM::NamespaceNode::Function(function));
				}
				
				inline SEM::Function* getFunction(const Name& name, bool searchParent = true) {
					SEM::NamespaceNode * node = rootNamespace_->lookup(name);
					if(node != NULL) return node->getFunction();
					return NULL;
				}
				
				inline bool addTypeInstance(const std::string& name, SEM::TypeInstance* typeInstance) {
					return rootNamespace_->children.tryInsert(name, SEM::NamespaceNode::TypeInstance(typeInstance));
				}
				
				inline SEM::TypeInstance* getTypeInstance(const Name& name){
					SEM::NamespaceNode * node = rootNamespace_->lookup(name);
					if(node != NULL) return node->getTypeInstance();
					return NULL;
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
				
				inline Name getName(){
					return parentContext_.getName();
				}
					
				inline bool addFunction(const std::string& name, SEM::Function* function, bool isMethod = false) {
					assert(!isMethod);
					return parentContext_.addFunction(name, function);
				}
				
				inline SEM::Function* getFunction(const Name& name, bool searchParent = true) {
					return parentContext_.getFunction(name);
				}
				
				inline bool addTypeInstance(const std::string& name, SEM::TypeInstance* typeInstance) {
					return parentContext_.addTypeInstance(name, typeInstance);
				}
				
				inline SEM::TypeInstance* getTypeInstance(const Name& name) {
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
				
				inline Name getName(){
					return typeInstance_->name;
				}
				
				inline bool addFunction(const std::string& name, SEM::Function* function, bool isMethod = false) {
					if(!isMethod){
						return typeInstance_->constructors.tryInsert(name, function);
					}else{
						return typeInstance_->methods.tryInsert(name, function);
					}
				}
				
				inline SEM::Function* getFunction(const Name& name, bool searchParent = true) {
					if(typeInstance_->name.isPrefixOf(name)){
						assert((typeInstance_->name.size() + 1) == name.size());
						
						const std::string nameEnd = name.last();
						Optional<SEM::Function *> constructor = typeInstance_->constructors.tryGet(nameEnd);
						if(constructor.hasValue()) return constructor.getValue();
						
						Optional<SEM::Function *> method = typeInstance_->methods.tryGet(nameEnd);
						if(method.hasValue()) return method.getValue();
						
						// No need to search parent, because this type instance's name is
						// the prefix of the target name, so the function must be in it.
						return NULL;
					}
					
					if(searchParent) return parentContext_.getFunction(name);
					return NULL;
				}
				
				inline bool addTypeInstance(const std::string& name, SEM::TypeInstance* typeInstance) {
					// No nested type instances.
					return false;
				}
				
				inline SEM::TypeInstance* getTypeInstance(const Name& name) {
					if(name == typeInstance_->name) return typeInstance_;
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
				
				inline Name getName(){
					return function_->name;
				}
				
				inline SEM::Type * getReturnType() {
					return function_->type->functionType.returnType;
				}
				
				inline SEM::Function* getFunction(const Name& name, bool searchParent = true) {
					return parentContext_.getFunction(name);
				}
				
				inline SEM::TypeInstance* getThisTypeInstance(){
					return parentContext_.getThisTypeInstance();
				}
				
				inline SEM::TypeInstance* getTypeInstance(const Name& name) {
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
