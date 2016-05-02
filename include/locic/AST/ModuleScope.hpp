#ifndef LOCIC_AST_MODULESCOPE_HPP
#define LOCIC_AST_MODULESCOPE_HPP

#include <string>

#include <locic/Support/Name.hpp>
#include <locic/Support/Version.hpp>

namespace locic {
	
	namespace AST {
	
		class ModuleScope {
			public:
				enum Kind {
					INTERNAL,
					IMPORT,
					EXPORT
				};
				
				static ModuleScope Internal();
				static ModuleScope Import(Name moduleName, Version moduleVersion);
				static ModuleScope Export(Name moduleName, Version moduleVersion);
				
				ModuleScope copy() const;
				
				Kind kind() const;
				
				bool isInternal() const;
				bool isImport() const;
				bool isExport() const;
				
				const Name& moduleName() const;
				const Version& moduleVersion() const;
				
				std::string kindString() const;
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
