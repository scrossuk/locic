#include <cassert>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>
#include <Locic/Map.hpp>
#include <Locic/Name.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context/RootContext.hpp>

namespace Locic {

	namespace SemanticAnalysis {
		
		RootContext::RootContext(){ }
				
				bool RootContext::addFunction(const Name& name, SEM::Function* function) {
					assert(name.isAbsolute());
					return false;
				}
				
				bool RootContext::addNamespace(const Name& name, SEM::Namespace* nameSpace){
					assert(name.isAbsolute());
					return false;
				}
				
				bool RootContext::addTypeInstance(const Name& name, SEM::TypeInstance* typeInstance) {
					assert(name.isAbsolute());
					return false;
				}
					
				Name RootContext::getName(){
					return Name::Absolute();
				}
				
				SEM::NamespaceNode RootContext::getNode(const Name& name){
					return SEM::NamespaceNode::None();
				}
				
				SEM::TypeInstance* RootContext::getThisTypeInstance(){
					return NULL;
				}
				
				SEM::Var * RootContext::getThisVar(const std::string& name){
					return NULL;
				}
		
	}
	
}

