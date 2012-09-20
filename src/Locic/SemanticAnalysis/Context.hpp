#ifndef LOCIC_SEMANTICANALYSIS_CONTEXT_HPP
#define LOCIC_SEMANTICANALYSIS_CONTEXT_HPP

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/SEM.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		inline SEM::NamespaceNode PerformLookup(SEM::Namespace * nameSpace, const Name& name){
			Name absoluteName = nameSpace->name.makeAbsolute(name);
			
			SEM::NamespaceNode node = SEM::NamespaceNode::Namespace(nameSpace);
			
			while(!node.isNone()){
				if(node.getName() == absoluteName){
					return node;
				}
				switch(node.typeEnum){
					case SEM::NamespaceNode::NAMESPACE:
					{
						node = node.getNamespace()->lookup(absoluteName);
						break;
					}
					case SEM::NamespaceNode::TYPEINSTANCE:
					{
						node = node.getTypeInstance()->lookup(absoluteName);
						break;
					}
					case SEM::NamespaceNode::FUNCTION:
					{
						// Functions have no children.
						return SEM::NamespaceNode::None();
					}
					default:
					{
						assert(false);
						return SEM::NamespaceNode::None();
					}
				}
			}
			
			return SEM::NamespaceNode::None();
		}
	
		class Context {
			public:
				virtual Name getName() = 0;
				
				virtual SEM::NamespaceNode getNode(const Name& name) = 0;
				
				virtual SEM::TypeInstance* getThisTypeInstance() = 0;
				
				virtual SEM::Var * getThisVar(const std::string& name) = 0;
				
				virtual bool addFunction(const Name& name, SEM::Function* function, bool isMethod = false) = 0;
				
				virtual bool addTypeInstance(const Name& name, SEM::TypeInstance* typeInstance) = 0;
				
		};
		
		class RootContext: public Context {
			public:
				inline RootContext(SEM::Namespace * rootNamespace)
					: rootNamespace_(rootNamespace){ }
				
				inline bool addFunction(const Name& name, SEM::Function* function, bool isMethod = false) {
					assert(name.isAbsolute());
					return false;
				}
				
				inline bool addTypeInstance(const Name& name, SEM::TypeInstance* typeInstance) {
					assert(name.isAbsolute());
					return false;
				}
					
				inline Name getName(){
					return Name::Absolute();
				}
				
				inline SEM::NamespaceNode getNode(const Name& name){
					return (name.empty() && name.isAbsolute()) ? SEM::NamespaceNode::Namespace(rootNamespace_) : SEM::NamespaceNode::None();
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
		
		class ModuleContext: public Context {
			public:
				inline ModuleContext(Context& parentContext, SEM::Module* module)
					: parentContext_(parentContext), module_(module) { }
				
				inline Name getName(){
					return parentContext_.getName();
				}
				
				inline SEM::NamespaceNode getNode(const Name& name){
					return parentContext_.getNode(name);
				}
				
				inline bool addFunction(const Name& name, SEM::Function* function, bool isMethod = false) {
					assert(name.isAbsolute());
					
					module_->functions.push_back(function);
					return parentContext_.addFunction(name, function, isMethod);
				}
				
				inline bool addTypeInstance(const Name& name, SEM::TypeInstance* typeInstance) {
					assert(name.isAbsolute());
					
					module_->typeInstances.push_back(typeInstance);
					return parentContext_.addTypeInstance(name, typeInstance);
				}
				
				inline SEM::TypeInstance* getThisTypeInstance(){
					return NULL;
				}
				
				inline SEM::Var * getThisVar(const std::string& name){
					return NULL;
				}
				
			private:
				Context& parentContext_;
				SEM::Module* module_;
				
		};
		
		class NamespaceContext: public Context {
			public:
				inline NamespaceContext(Context& parentContext, SEM::Namespace * nameSpace)
					: parentContext_(parentContext), nameSpace_(nameSpace){ }
					
				inline Name getName(){
					return nameSpace_->name;
				}
				
				inline SEM::NamespaceNode getNode(const Name& name){
					SEM::NamespaceNode resultNode = PerformLookup(nameSpace_, name);
					return resultNode.isNotNone() ? resultNode : parentContext_.getNode(name);
				}
				
				inline bool addFunction(const Name& name, SEM::Function* function, bool isMethod = false) {
					assert(name.isAbsolute());
					
					bool inserted = false;
					if(nameSpace_->name.isExactPrefixOf(name)){
						assert(!isMethod);
						inserted |= nameSpace_->children.tryInsert(name.last(), SEM::NamespaceNode::Function(function));
					}
					inserted |= parentContext_.addFunction(name, function, isMethod);
					return inserted;
				}
				
				inline bool addTypeInstance(const Name& name, SEM::TypeInstance* typeInstance) {
					assert(name.isAbsolute());
					
					bool inserted = false;
					if(nameSpace_->name.isExactPrefixOf(name)){
						inserted |= nameSpace_->children.tryInsert(name.last(), SEM::NamespaceNode::TypeInstance(typeInstance));
					}
					inserted |= parentContext_.addTypeInstance(name, typeInstance);
					return inserted;
				}
				
				inline SEM::TypeInstance* getThisTypeInstance(){
					return NULL;
				}
				
				inline SEM::Var * getThisVar(const std::string& name){
					return NULL;
				}
				
			private:
				Context& parentContext_;
				SEM::Namespace * nameSpace_;
				
		};
		
		class TypeInstanceContext: public Context {
			public:
				inline TypeInstanceContext(Context& parentContext, SEM::TypeInstance * typeInstance)
					: parentContext_(parentContext), typeInstance_(typeInstance) { }
				
				inline Name getName(){
					return typeInstance_->name;
				}
				
				inline bool addFunction(const Name& name, SEM::Function* function, bool isMethod = false) {
					assert(name.isAbsolute());
					
					bool inserted = false;
					if(typeInstance_->name.isExactPrefixOf(name)){
						if(!isMethod){
							inserted |= typeInstance_->constructors.tryInsert(name.last(), function);
						}else{
							inserted |= typeInstance_->methods.tryInsert(name.last(), function);
						}
					}
					inserted |= parentContext_.addFunction(name, function, isMethod);
					return inserted;
				}
				
				inline bool addTypeInstance(const Name& name, SEM::TypeInstance* typeInstance) {
					assert(name.isAbsolute());
					return parentContext_.addTypeInstance(name, typeInstance);
				}
				
				inline SEM::NamespaceNode getNode(const Name& name){
					return parentContext_.getNode(name);
				}
				
				inline SEM::TypeInstance* getThisTypeInstance(){
					return typeInstance_;
				}
				
				inline SEM::Var * getThisVar(const std::string& name){
					Optional<SEM::Var *> varResult = typeInstance_->variables.tryGet(name);
					return varResult.hasValue() ? varResult.getValue() : parentContext_.getThisVar(name);
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
				
				inline bool addFunction(const Name& name, SEM::Function* function, bool isMethod = false) {
					assert(name.isAbsolute());
					return parentContext_.addFunction(name, function, isMethod);
				}
				
				inline bool addTypeInstance(const Name& name, SEM::TypeInstance* typeInstance) {
					assert(name.isAbsolute());
					return parentContext_.addTypeInstance(name, typeInstance);
				}
				
				inline Name getName(){
					return function_->name;
				}
				
				inline SEM::Type * getReturnType() {
					return function_->type->functionType.returnType;
				}
				
				inline SEM::NamespaceNode getNode(const Name& name){
					return parentContext_.getNode(name);
				}
				
				inline SEM::TypeInstance* getThisTypeInstance(){
					return parentContext_.getThisTypeInstance();
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
