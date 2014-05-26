#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <set>
#include <string>
#include <vector>

#include <locic/CodeGen/LLVMIncludes.hpp>

#include <locic/Name.hpp>
#include <locic/String.hpp>

#include <locic/CodeGen/ConstantGenerator.hpp>
#include <locic/CodeGen/md5.h>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/VTable.hpp>

namespace locic {

	namespace CodeGen {
	
		static MethodHash getMethodHash(const std::string& hash) {
			return strtoul(hash.substr(0, 16).c_str(), NULL, 16);
		}
		
		MethodHash CreateMethodNameHash(const std::string& methodName) {
			const std::string md5Hash = md5(CanonicalizeMethodName(methodName));
			return getMethodHash(md5Hash);
		}
		
		llvm::Value* CreateHashValue(Module& module, const std::string& methodName) {
			return ConstantGenerator(module).getI64(CreateMethodNameHash(methodName));
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
				
				s += makeString("%llu: ", (unsigned long long) i);
								
				const std::list<MethodHash>& slotList = table_.at(i);
				
				s += "(";
				
				bool isFirst = true;
				for (auto hash: slotList) {
					if (isFirst) {
						isFirst = false;
					} else {
						s += ", ";
					}
					
					s += makeString("%llu", (unsigned long long) hash);
				}
				
				s += ")";
			}
			
			s += "}";
			
			return s;
		}
		
		std::string VirtualTable::toStringWithMapping(const Map<MethodHash, std::string>& mapping) const {
			std::string s = "VirtualTable{";
			
			for (size_t i = 0; i < VTABLE_SIZE; i++) {
				if (i > 0) {
					s += ", ";
				}
				
				s += makeString("%llu: ", (unsigned long long) i);
								
				const std::list<MethodHash>& slotList = table_.at(i);
				
				s += "(";
				
				bool isFirst = true;
				for (auto hash: slotList) {
					if (isFirst) {
						isFirst = false;
					} else {
						s += ", ";
					}
					
					s += makeString("%s: %llu", mapping.get(hash).c_str(),
						(unsigned long long) hash);
				}
				
				s += ")";
			}
			
			s += "}";
			
			return s;
		}
		
		VirtualTable::VirtualTable() { }
		
	}
	
}

