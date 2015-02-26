#include <string>
#include <vector>

#include <locic/String.hpp>

#include <locic/AST/Function.hpp>
#include <locic/AST/ModuleScope.hpp>
#include <locic/AST/Namespace.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/TypeInstance.hpp>

namespace locic {

	namespace AST {
	
		std::string NamespaceData::toString() const {
			std::string s;
			
			bool isFirst = true;
			
			for(auto node : functions) {
				if(!isFirst) s += ", ";
				
				isFirst = false;
				s += node.toString();
			}
			
			for(auto node : moduleScopes) {
				if(!isFirst) s += ", ";
				
				isFirst = false;
				s += node.toString();
			}
			
			for(auto node : namespaces) {
				if(!isFirst) s += ", ";
				
				isFirst = false;
				s += node.toString();
			}
			
			for(auto node : typeInstances) {
				if(!isFirst) s += ", ";
				
				isFirst = false;
				s += node.toString();
			}
			
			return s;
		}
		
		Namespace::Namespace(const String& n, AST::Node<NamespaceData> d)
				: name(n), data(d) { }
		
		std::string Namespace::toString() const {
			return makeString("Namespace[name = %s](", name.c_str()) + data->toString() + ")";
		}
		
	}
	
}

