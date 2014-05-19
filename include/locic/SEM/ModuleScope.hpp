#ifndef LOCIC_SEM_MODULESCOPE_HPP
#define LOCIC_SEM_MODULESCOPE_HPP

#include <string>

#include <locic/Name.hpp>
#include <locic/Version.hpp>

namespace locic {
	
	namespace SEM {
	
		class ModuleScope {
			public:
				enum Kind {
					IMPORT,
					EXPORT
				};
				
				static ModuleScope* Import(Name moduleName, Version moduleVersion);
				static ModuleScope* Export(Name moduleName, Version moduleVersion);
				
				Kind kind() const;
				
				bool isImport() const;
				bool isExport() const;
				
				const Name& moduleName() const;
				const Version& moduleVersion() const;
				
				std::string toString() const;
				
			private:
				ModuleScope(Kind k, Name n, Version v);
					
				Kind kind_;
				Name moduleName_;
				Version moduleVersion_;
				
		};
		
	}
	
}

#endif
