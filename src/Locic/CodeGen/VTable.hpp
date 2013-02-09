#ifndef LOCIC_CODEGEN_HASH_HPP
#define LOCIC_CODEGEN_HASH_HPP

#include <stdint.h>

#include <list>
#include <string>
#include <vector>

namespace Locic{

	namespace CodeGen{
		
		const size_t VTABLE_SIZE = 20;
		
		/**
		 * All method names are 'hashed' to a 32 bit
		 * unsigned integer, for use in virtual method
		 * tables.
		 */
		typedef uint32_t MethodHash;
		
		MethodHash CreateMethodNameHash(const std::string& methodName);
		
		class VirtualTable{
			public:
				static VirtualTable CalculateFromHashes(const std::vector<MethodHash>& methods);
				
				static VirtualTable CalculateFromNames(const std::vector<std::string>& methods);
				
				const std::vector< std::list<MethodHash> >& table() const;
				
				std::string toString() const;
				
			private:
				VirtualTable();
				
				std::vector< std::list<MethodHash> > table_;
				
		};
	
	}	
	
}

#endif
