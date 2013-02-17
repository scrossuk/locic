#ifndef LOCIC_CODEGEN_TARGETINFO_HPP
#define LOCIC_CODEGEN_TARGETINFO_HPP

#include <stdint.h>
#include <string>

#include <Locic/Map.hpp>

namespace Locic{
	
	namespace CodeGen{
		
		class TargetInfo{
			public:
				static TargetInfo DefaultTarget();
				
				TargetInfo(const std::string& triple);
				~TargetInfo();
				
				bool isBigEndian() const;
				
				std::string getTargetTriple() const;
				
				size_t getPointerSize() const;
				
				size_t getPrimitiveSize(const std::string& name) const;
				
			private:
				Map<std::string, size_t> primitiveSizes_;
				std::string triple_;
				bool isBigEndian_;
				size_t pointerSize_;
			
		};
		
	}
	
}

#endif
