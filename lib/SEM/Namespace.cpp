#include <string>
#include <vector>

#include <locic/Support/MakeString.hpp>
#include <locic/Support/String.hpp>

#include <locic/SEM/Function.hpp>
#include <locic/SEM/Namespace.hpp>
#include <locic/SEM/TypeAlias.hpp>
#include <locic/SEM/TypeInstance.hpp>

namespace locic {

	namespace SEM {
	
		NamespaceItem NamespaceItem::Function(SEM::Function* function) {
			NamespaceItem item(FUNCTION);
			item.data_.function = function;
			return item;
		}
		
		NamespaceItem NamespaceItem::Namespace(SEM::Namespace* nameSpace) {
			NamespaceItem item(NAMESPACE);
			item.data_.nameSpace = nameSpace;
			return item;
		}
		
		NamespaceItem NamespaceItem::TypeAlias(SEM::TypeAlias* typeAlias) {
			NamespaceItem item(TYPEALIAS);
			item.data_.typeAlias = typeAlias;
			return item;
		}
		
		NamespaceItem NamespaceItem::TypeInstance(SEM::TypeInstance* typeInstance) {
			NamespaceItem item(TYPEINSTANCE);
			item.data_.typeInstance = typeInstance;
			return item;
		}
		
		NamespaceItem::Kind NamespaceItem::kind() const {
			return kind_;
		}
		
		bool NamespaceItem::isFunction() const {
			return kind() == FUNCTION;
		}
		
		bool NamespaceItem::isNamespace() const {
			return kind() == NAMESPACE;
		}
		
		bool NamespaceItem::isTypeAlias() const {
			return kind() == TYPEALIAS;
		}
		
		bool NamespaceItem::isTypeInstance() const {
			return kind() == TYPEINSTANCE;
		}
		
		Function* NamespaceItem::function() const {
			assert(isFunction());
			return data_.function;
		}
		
		Namespace* NamespaceItem::nameSpace() const {
			assert(isNamespace());
			return data_.nameSpace;
		}
		
		TypeAlias* NamespaceItem::typeAlias() const {
			assert(isTypeAlias());
			return data_.typeAlias;
		}
		
		TypeInstance* NamespaceItem::typeInstance() const {
			assert(isTypeInstance());
			return data_.typeInstance;
		}
		
		std::string NamespaceItem::toString() const {
			switch (kind()) {
				case FUNCTION:
					return function()->toString();
				case NAMESPACE:
					return nameSpace()->toString();
				case TYPEALIAS:
					return typeAlias()->toString();
				case TYPEINSTANCE:
					return typeInstance()->toString();
				default:
					return "";
			}
		}
		
		NamespaceItem::NamespaceItem(Kind pKind)
			: kind_(pKind) {
				data_.ptr = nullptr;
			}
		
		Namespace::Namespace(Name n)
			: name_(std::move(n)) { }
			
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

