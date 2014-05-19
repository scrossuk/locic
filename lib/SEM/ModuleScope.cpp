#include <string>

#include <locic/Name.hpp>
#include <locic/String.hpp>
#include <locic/Version.hpp>

#include <locic/SEM/ModuleScope.hpp>

namespace locic {

	namespace SEM {
	
		ModuleScope* ModuleScope::Import(Name moduleName, Version moduleVersion) {
			return new ModuleScope(IMPORT, moduleName, moduleVersion);
		}
		
		ModuleScope* ModuleScope::Export(Name moduleName, Version moduleVersion) {
			return new ModuleScope(EXPORT, moduleName, moduleVersion);
		}
		
		ModuleScope::Kind ModuleScope::kind() const {
			return kind_;
		}
		
		bool ModuleScope::isImport() const {
			return kind_ == IMPORT;
		}
		
		bool ModuleScope::isExport() const {
			return kind_ == EXPORT;
		}
		
		const Name& ModuleScope::moduleName() const {
			return moduleName_;
		}
		
		const Version& ModuleScope::moduleVersion() const {
			return moduleVersion_;
		}
		
		std::string ModuleScope::toString() const {
			return makeString("%s(name = %s, version = %s)",
				isImport() ? "Import" : "Export",
				moduleName().toString().c_str(),
				moduleVersion().toString().c_str());
		}
		
		ModuleScope::ModuleScope(Kind k, Name n, Version v)
			: kind_(k), moduleName_(n), moduleVersion_(v) { }
		
	}
	
}

