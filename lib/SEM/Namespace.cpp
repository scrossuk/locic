#include <memory>
#include <string>
#include <vector>

#include <locic/AST/AliasDecl.hpp>
#include <locic/AST/Function.hpp>

#include <locic/Support/ErrorHandling.hpp>
#include <locic/Support/MakeString.hpp>
#include <locic/Support/String.hpp>

#include <locic/SEM/Namespace.hpp>
#include <locic/SEM/Scope.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {
	
	namespace SEM {
		
		NamespaceItem NamespaceItem::Alias(AST::AliasDecl& alias) {
			NamespaceItem item(ALIAS);
			item.data_.alias = &alias;
			return item;
		}
		
		NamespaceItem NamespaceItem::Function(AST::Function& function) {
			NamespaceItem item(FUNCTION);
			item.data_.function = &function;
			return item;
		}
		
		NamespaceItem NamespaceItem::Namespace(std::unique_ptr<SEM::Namespace> nameSpace) {
			NamespaceItem item(NAMESPACE);
			item.data_.nameSpace = nameSpace.release();
			return item;
		}
		
		NamespaceItem NamespaceItem::TypeInstance(std::unique_ptr<SEM::TypeInstance> typeInstance) {
			NamespaceItem item(TYPEINSTANCE);
			item.data_.typeInstance = typeInstance.release();
			return item;
		}
		
		NamespaceItem::~NamespaceItem() {
			switch (kind()) {
				case ALIAS:
					//delete data_.typeAlias;
					return;
				case FUNCTION:
					//delete data_.function;
					return;
				case NAMESPACE:
					//delete data_.nameSpace;
					return;
				case TYPEINSTANCE:
					//delete data_.typeInstance;
					return;
			}
		}
		
		NamespaceItem::Kind NamespaceItem::kind() const {
			return kind_;
		}
		
		bool NamespaceItem::isAlias() const {
			return kind() == ALIAS;
		}
		
		bool NamespaceItem::isFunction() const {
			return kind() == FUNCTION;
		}
		
		bool NamespaceItem::isNamespace() const {
			return kind() == NAMESPACE;
		}
		
		bool NamespaceItem::isTypeInstance() const {
			return kind() == TYPEINSTANCE;
		}
		
		AST::AliasDecl& NamespaceItem::alias() const {
			assert(isAlias());
			return *(data_.alias);
		}
		
		AST::Function& NamespaceItem::function() const {
			assert(isFunction());
			return *(data_.function);
		}
		
		Namespace& NamespaceItem::nameSpace() const {
			assert(isNamespace());
			return *(data_.nameSpace);
		}
		
		TypeInstance& NamespaceItem::typeInstance() const {
			assert(isTypeInstance());
			return *(data_.typeInstance);
		}
		
		Debug::SourceLocation NamespaceItem::location() const {
			switch (kind()) {
				case ALIAS:
					return alias().location();
				case FUNCTION:
					return function().debugInfo()->declLocation;
				case NAMESPACE:
					return nameSpace().astNamespaces().front()->location();
				case TYPEINSTANCE:
					return typeInstance().debugInfo()->location;
			}
			
			locic_unreachable("Unknown NamespaceItem kind.");
		}
		
		std::string NamespaceItem::toString() const {
			switch (kind()) {
				case ALIAS:
					return alias().toString();
				case FUNCTION:
					return function().toString();
				case NAMESPACE:
					return nameSpace().toString();
				case TYPEINSTANCE:
					return typeInstance().toString();
			}
			
			locic_unreachable("Unknown NamespaceItem kind.");
		}
		
		NamespaceItem::NamespaceItem(Kind pKind)
			: kind_(pKind) {
				data_.ptr = nullptr;
			}
		
		Namespace::Namespace()
			: parent_(GlobalStructure::Namespace(*this)),
			name_(Name::Absolute()) { }
		
		Namespace::Namespace(Name n, GlobalStructure argParent)
			: parent_(std::move(argParent)),
			  name_(std::move(n)) { }
		
		std::vector<AST::Node<AST::NamespaceDecl>*>&
		Namespace::astNamespaces() {
			return astNamespaces_;
		}
		
		const std::vector<AST::Node<AST::NamespaceDecl>*>&
		Namespace::astNamespaces() const {
			return astNamespaces_;
		}
		
		GlobalStructure& Namespace::parent() {
			return parent_;
		}
		
		const GlobalStructure& Namespace::parent() const {
			return parent_;
		}
		
		const Name& Namespace::name() const {
			return name_;
		}
		
		FastMap<String, NamespaceItem>& Namespace::items() {
			return items_;
		}
		
		const FastMap<String, NamespaceItem>& Namespace::items() const {
			return items_;
		}
		
		std::string Namespace::toString() const {
			return makeString("Namespace(name: %s, items: %s)",
				name().toString().c_str(), makeMapString(items_).c_str());
		}
		
	}
	
}

