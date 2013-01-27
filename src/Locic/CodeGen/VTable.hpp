#ifndef LOCIC_CODEGEN_HASH_HPP
#define LOCIC_CODEGEN_HASH_HPP

#include <stdint.h>
#include <string>
#include <vector>

namespace Locic{

	namespace CodeGen{
		
		/**
		 * All method names are 'hashed' to a 32 bit
		 * unsigned integer, for use in virtual method
		 * tables.
		 */
		typedef uint32_t MethodHash;
		
		MethodHash CreateMethodNameHash(const std::string& methodName);
		
		typedef uint8_t MethodHashShift;
		
		class VirtualTable{
			public:
				static VirtualTable CalculateFromHashes(const std::vector<MethodHash>& methods);
				
				static VirtualTable CalculateFromNames(const std::vector<std::string>& methods);
				
				const std::vector<MethodHash>& table() const;
				
				MethodHash mask() const;
				
				MethodHashShift shift() const;
				
				std::string toString() const;
				
			private:
				VirtualTable();
				
				std::vector<MethodHash> table_;
				MethodHash mask_;
				MethodHashShift shift_;
				
		};
	
	}	
	
}

#endif
