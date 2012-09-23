#include <cassert>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context/ModuleContext.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		ModuleContext::ModuleContext(Context& parentContext, SEM::Module* module)
					: parentContext_(parentContext), module_(module) { }
				
				Name ModuleContext::getName(){
					return parentContext_.getName();
				}
				
				SEM::NamespaceNode ModuleContext::getNode(const Name& name){
					return parentContext_.getNode(name);
				}
				
				bool ModuleContext::addFunction(const Name& name, SEM::Function* function, bool isMethod) {
					assert(name.isAbsolute());
					
					module_->functions.push_back(function);
					return parentContext_.addFunction(name, function, isMethod);
				}
				
				bool ModuleContext::addNamespace(const Name& name, SEM::Namespace* nameSpace){
					assert(name.isAbsolute());
					return parentContext_.addNamespace(name, nameSpace);
				}
				
				bool ModuleContext::addTypeInstance(const Name& name, SEM::TypeInstance* typeInstance) {
					assert(name.isAbsolute());
					
					module_->typeInstances.push_back(typeInstance);
					return parentContext_.addTypeInstance(name, typeInstance);
				}
				
				SEM::TypeInstance* ModuleContext::getThisTypeInstance(){
					return NULL;
				}
				
				SEM::Var * ModuleContext::getThisVar(const std::string& name){
					return NULL;
				}
		
	}
	
}

