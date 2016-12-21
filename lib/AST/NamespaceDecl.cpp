#include <string>
#include <vector>

#include <locic/Support/String.hpp>

#include <locic/AST/Alias.hpp>
#include <locic/AST/Function.hpp>
#include <locic/AST/ModuleScopeDecl.hpp>
#include <locic/AST/NamespaceDecl.hpp>
#include <locic/AST/Node.hpp>
#include <locic/AST/StaticAssert.hpp>
#include <locic/AST/TypeInstance.hpp>

namespace locic {
	
	namespace AST {
		
		NamespaceData::NamespaceData() { }
		
		NamespaceData::~NamespaceData() { }
		
		std::string NamespaceData::toString() const {
			std::string s;
			
			bool isFirst = true;
			
			for(const auto& node : aliases) {
				if(!isFirst) s += ", ";
				
				isFirst = false;
				s += node.toString();
			}
			
			for(const auto& node : functions) {
				if(!isFirst) s += ", ";
				
				isFirst = false;
				s += node.toString();
			}
			
			for(const auto& node : moduleScopes) {
				if(!isFirst) s += ", ";
				
				isFirst = false;
				s += node.toString();
			}
			
			for(const auto& node : namespaces) {
				if(!isFirst) s += ", ";
				
				isFirst = false;
				s += node.toString();
			}
			
			for(const auto& node : typeInstances) {
				if(!isFirst) s += ", ";
				
				isFirst = false;
				s += node.toString();
			}
			
			return s;
		}
		
		NamespaceDecl::NamespaceDecl(const String& n, Node<NamespaceData> d)
		: name_(n), data_(std::move(d)), namespace_(nullptr) { }
		
		NamespaceDecl::~NamespaceDecl() { }
		
		String NamespaceDecl::name() const {
			return name_;
		}
		
		const Node<NamespaceData>& NamespaceDecl::data() const {
			return data_;
		}
		
		void NamespaceDecl::setNamespace(Namespace& astNameSpace) {
			assert(namespace_ == nullptr);
			namespace_ = &astNameSpace;
		}
		
		Namespace& NamespaceDecl::nameSpace() {
			assert(namespace_ != nullptr);
			return *namespace_;
		}
		
		const Namespace& NamespaceDecl::nameSpace() const {
			assert(namespace_ != nullptr);
			return *namespace_;
		}
		
		std::string NamespaceDecl::toString() const {
			return makeString("NamespaceDecl[name = %s](", name().c_str()) + data()->toString() + ")";
		}
		
	}
	
}

