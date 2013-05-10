#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <set>
#include <string>
#include <vector>

#include <Locic/String.hpp>
#include <Locic/CodeGen/md5.h>
#include <Locic/CodeGen/VTable.hpp>

namespace Locic {

	namespace CodeGen {
	
		static MethodHash getMethodHash(const std::string& hash) {
			return strtoul(hash.substr(0, 8).c_str(), NULL, 16);
		}
		
		MethodHash CreateMethodNameHash(const std::string& methodName) {
			const std::string md5Hash = md5(methodName);
			return getMethodHash(md5Hash);
		}
		
		VirtualTable VirtualTable::CalculateFromHashes(const std::vector<MethodHash>& methods) {
			VirtualTable table;
			table.table_.resize(VTABLE_SIZE);
			
			for (size_t i = 0; i < methods.size(); i++) {
				const MethodHash method = methods.at(i);
				std::list<MethodHash>& slotList = table.table_.at(method % VTABLE_SIZE);
				slotList.push_back(method);
			}
			
			return table;
		}
		
		VirtualTable VirtualTable::CalculateFromNames(const std::vector<std::string>& methods) {
			std::vector<MethodHash> hashes;
			
			for (size_t i = 0; i < methods.size(); i++) {
				hashes.push_back(CreateMethodNameHash(methods.at(i)));
			}
			
			return VirtualTable::CalculateFromHashes(hashes);
		}
		
		const std::vector< std::list<MethodHash> >& VirtualTable::table() const {
			return table_;
		}
		
		std::string VirtualTable::toString() const {
			std::string s = "VirtualTable{";
			
			for (size_t i = 0; i < VTABLE_SIZE; i++) {
				if (i > 0) {
					s += ", ";
				}
				
				s += makeString("%llu: ",
								(unsigned long long) i);
								
				const std::list<MethodHash>& slotList = table_.at(i);
				
				if (slotList.empty()) {
					s += "empty";
					continue;
				} else if (slotList.size() == 1) {
					s += makeString("%llu",
									(unsigned long long) slotList.front());
					continue;
				}
				
				s += "clash[";
				
				for (std::list<MethodHash>::const_iterator it = slotList.begin();
					 it != slotList.end(); ++it) {
					 
					if (it != slotList.begin()) {
						s += ", ";
					}
					
					s += makeString("%llu",
									(unsigned long long) * it);
				}
				
				s += "]";
			}
			
			s += "}";
			
			return s;
		}
		
		VirtualTable::VirtualTable() { }
		
	}
	
}

