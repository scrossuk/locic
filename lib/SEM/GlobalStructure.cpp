#include <locic/AST/AliasDecl.hpp>

#include <locic/SEM/GlobalStructure.hpp>
#include <locic/SEM/Namespace.hpp>
#include <locic/SEM/TypeInstance.hpp>

#include <locic/Support/Name.hpp>

namespace locic {
	
	namespace SEM {
		
		GlobalStructure GlobalStructure::Alias(AST::AliasDecl& alias) {
			GlobalStructure globalStructure(ALIAS);
			globalStructure.data_.alias = &alias;
			return globalStructure;
		}
		
		GlobalStructure GlobalStructure::Namespace(SEM::Namespace& nameSpace) {
			GlobalStructure globalStructure(NAMESPACE);
			globalStructure.data_.nameSpace = &nameSpace;
			return globalStructure;
		}
		
		GlobalStructure GlobalStructure::TypeInstance(SEM::TypeInstance& typeInstance) {
			GlobalStructure globalStructure(TYPEINSTANCE);
			globalStructure.data_.typeInstance = &typeInstance;
			return globalStructure;
		}
		
		GlobalStructure::Kind GlobalStructure::kind() const {
			return kind_;
		}
		
		bool GlobalStructure::isAlias() const {
			return kind() == ALIAS;
		}
		
		bool GlobalStructure::isNamespace() const {
			return kind() == NAMESPACE;
		}
		
		bool GlobalStructure::isTypeInstance() const {
			return kind() == TYPEINSTANCE;
		}
		
		const Name& GlobalStructure::name() const {
			switch (kind()) {
				case ALIAS:
					return alias().fullName();
				case NAMESPACE:
					return nameSpace().name();
				case TYPEINSTANCE:
					return typeInstance().fullName();
			}
			
			locic_unreachable("Unknown GlobalStructure kind.");
		}
		
		GlobalStructure& GlobalStructure::parent() {
			switch (kind()) {
				case ALIAS:
					return alias().parent();
				case NAMESPACE:
					return nameSpace().parent();
				case TYPEINSTANCE:
					return typeInstance().parent();
			}
			
			locic_unreachable("Unknown GlobalStructure kind.");
		}
		
		const GlobalStructure& GlobalStructure::parent() const {
			switch (kind()) {
				case ALIAS:
					return alias().parent();
				case NAMESPACE:
					return nameSpace().parent();
				case TYPEINSTANCE:
					return typeInstance().parent();
			}
			
			locic_unreachable("Unknown GlobalStructure kind.");
		}
		
		AST::AliasDecl& GlobalStructure::alias() {
			return *(data_.alias);
		}
		
		const AST::AliasDecl& GlobalStructure::alias() const {
			return *(data_.alias);
		}
		
		SEM::Namespace& GlobalStructure::nameSpace() {
			return *(data_.nameSpace);
		}
		
		const SEM::Namespace& GlobalStructure::nameSpace() const {
			return *(data_.nameSpace);
		}
		
		SEM::TypeInstance& GlobalStructure::typeInstance() {
			return *(data_.typeInstance);
		}
		
		const SEM::TypeInstance& GlobalStructure::typeInstance() const {
			return *(data_.typeInstance);
		}
		
		SEM::Namespace& GlobalStructure::nextNamespace() {
			auto next = this;
			while (!next->isNamespace()) {
				next = &(next->parent());
			}
			return next->nameSpace();
		}
		
		const SEM::Namespace& GlobalStructure::nextNamespace() const {
			auto next = this;
			while (!next->isNamespace()) {
				next = &(next->parent());
			}
			return next->nameSpace();
		}
		
		std::string GlobalStructure::toString() const {
			switch (kind()) {
				case ALIAS:
					return alias().toString();
				case NAMESPACE:
					return nameSpace().toString();
				case TYPEINSTANCE:
					return typeInstance().toString();
			}
			
			locic_unreachable("Unknown GlobalStructure kind.");
		}
		
		GlobalStructure::GlobalStructure(Kind pKind)
		: kind_(pKind) { }
		
	}
	
}
