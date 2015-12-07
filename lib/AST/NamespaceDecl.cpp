#include <string>
#include <vector>

#include <locic/Support/String.hpp>

#include <locic/AST/Alias.hpp>
#include <locic/AST/Function.hpp>
#include <locic/AST/ModuleScope.hpp>
#include <locic/AST/NamespaceDecl.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/TypeInstance.hpp>

namespace locic {

	namespace AST {
	
		std::string NamespaceData::toString() const {
			std::string s;
			
			bool isFirst = true;
			
			for(auto node : aliases) {
				if(!isFirst) s += ", ";
				
				isFirst = false;
				s += node.toString();
			}
			
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
		
		NamespaceDecl::NamespaceDecl(const String& n, AST::Node<NamespaceData> d)
		: name_(n), data_(d), namespace_(nullptr) { }
		
		String NamespaceDecl::name() const {
			return name_;
		}
		
		const AST::Node<NamespaceData>& NamespaceDecl::data() const {
			return data_;
		}
		
		void NamespaceDecl::setNamespace(SEM::Namespace& nameSpace) {
			assert(namespace_ == nullptr);
			namespace_ = &nameSpace;
		}
		
		SEM::Namespace& NamespaceDecl::nameSpace() {
			assert(namespace_ != nullptr);
			return *namespace_;
		}
		
		const SEM::Namespace& NamespaceDecl::nameSpace() const {
			assert(namespace_ != nullptr);
			return *namespace_;
		}
		
		std::string NamespaceDecl::toString() const {
			return makeString("NamespaceDecl[name = %s](", name().c_str()) + data()->toString() + ")";
		}
		
	}
	
}

