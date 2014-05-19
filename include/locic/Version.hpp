#ifndef LOCIC_VERSION_HPP
#define LOCIC_VERSION_HPP

#include <string>
#include <vector>

namespace locic {
	
	class Version {
		public:
			static Version FromString(const std::string& versionString);
			
			Version(size_t pMajor, size_t pMinor, size_t pBuild);
			
			size_t majorVersion() const;
			
			size_t minorVersion() const;
			
			size_t buildVersion() const;
			
			std::string toString() const;
			
		private:
			size_t major_;
			size_t minor_;
			size_t build_;
			
	};
	
}

#endif
