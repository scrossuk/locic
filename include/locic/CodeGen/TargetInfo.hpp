#ifndef LOCIC_CODEGEN_TARGETINFO_HPP
#define LOCIC_CODEGEN_TARGETINFO_HPP

#include <stdint.h>
#include <string>

#include <locic/Map.hpp>

namespace locic {

	namespace CodeGen {
	
		class TargetInfo {
			public:
				static TargetInfo DefaultTarget();
				
				TargetInfo(const std::string& triple);
				~TargetInfo();
				
				bool isBigEndian() const;
				
				std::string getTargetTriple() const;
				
				size_t getPointerSize() const;
				
				inline size_t getPointerSizeInBytes() const {
					const size_t size = getPointerSize();
					assert((size % 8) == 0);
					return size / 8;
				}
				
				size_t getPrimitiveSize(const std::string& name) const;
				
				inline size_t getPrimitiveSizeInBytes(const std::string& name) const {
					const size_t size = getPrimitiveSize(name);
					assert((size % 8) == 0);
					return size / 8;
				}
				
			private:
				Map<std::string, size_t> primitiveSizes_;
				std::string triple_;
				bool isBigEndian_;
				size_t pointerSize_;
				
		};
		
	}
	
}

#endif
