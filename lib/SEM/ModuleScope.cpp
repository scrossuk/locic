#include <stdexcept>
#include <string>

#include <locic/MakeString.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/String.hpp>
#include <locic/Support/Version.hpp>

#include <locic/SEM/ModuleScope.hpp>

namespace locic {

	namespace SEM {
	
		ModuleScope ModuleScope::Internal() {
			return ModuleScope(INTERNAL, Name::Absolute(), Version(0,0,0));
		}
		
		ModuleScope ModuleScope::Import(Name moduleName, Version moduleVersion) {
			return ModuleScope(IMPORT, std::move(moduleName), std::move(moduleVersion));
		}
		
		ModuleScope ModuleScope::Export(Name moduleName, Version moduleVersion) {
			return ModuleScope(EXPORT, std::move(moduleName), std::move(moduleVersion));
		}
		
		ModuleScope ModuleScope::copy() const {
			return ModuleScope(kind(), moduleName_.copy(), moduleVersion_);
		}
		
		ModuleScope::Kind ModuleScope::kind() const {
			return kind_;
		}
		
		bool ModuleScope::isInternal() const {
			return kind_ == INTERNAL;
		}
		
		bool ModuleScope::isImport() const {
			return kind_ == IMPORT;
		}
		
		bool ModuleScope::isExport() const {
			return kind_ == EXPORT;
		}
		
		const Name& ModuleScope::moduleName() const {
			assert(isImport() || isExport());
			return moduleName_;
		}
		
		const Version& ModuleScope::moduleVersion() const {
			assert(isImport() || isExport());
			return moduleVersion_;
		}
		
		std::string ModuleScope::kindString() const {
			switch (kind()) {
				case INTERNAL:
					return "Internal";
				case IMPORT:
					return "Import";
				case EXPORT:
					return "Export";
			}
			
			throw std::logic_error("Unknown SEM::ModuleScope kind.");
		}
		
		std::string ModuleScope::toString() const {
			return makeString("%s(name = %s, version = %s)",
				kindString().c_str(),
				moduleName().toString().c_str(),
				moduleVersion().toString().c_str());
		}
		
		ModuleScope::ModuleScope(Kind k, Name n, Version v)
			: kind_(std::move(k)),
			moduleName_(std::move(n)),
			moduleVersion_(std::move(v)) { }
		
	}
	
}

