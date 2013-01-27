#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <set>
#include <string>
#include <vector>

#include <Locic/String.hpp>
#include <Locic/CodeGen/md5.h>
#include <Locic/CodeGen/VTable.hpp>

namespace Locic{

	namespace CodeGen{
		
		static MethodHash getMethodHash(const std::string& hash){
			return strtoul(hash.substr(0, 8).c_str(), NULL, 16);
		}
		
		MethodHash CreateMethodNameHash(const std::string& methodName){
			const std::string md5Hash = md5(methodName);
			return getMethodHash(md5Hash);
		}
		
		static bool doesClash(const std::vector<MethodHash>& methods, const MethodHashShift shift, const MethodHash mask){
			std::set<MethodHash> values;
			
			for(size_t i = 0; i < methods.size(); i++){
				const MethodHash method = methods.at(i);
				const MethodHash transformedMethod = (method >> shift) & mask;
				if(values.find(transformedMethod) != values.end()){
					return true;
				}
				values.insert(transformedMethod);
			}
				
			return false;
		}
		
		static bool getBestValues(const std::vector<MethodHash>& methods, MethodHashShift& bestShift, MethodHash& bestMask){
			for(MethodHashShift power = 0; power <= 32; power++){
				const MethodHash powerValue = static_cast<MethodHash>(1) << static_cast<MethodHash>(power);
				const MethodHash mask = powerValue - 1;
				for(MethodHashShift shift = 0; shift < 32; shift++){
					if(!doesClash(methods, shift, mask)){
						bestShift = shift;
						bestMask = mask;
						return true;
					}
				}
			}
			return false;
		}
		
		VirtualTable VirtualTable::CalculateFromHashes(const std::vector<MethodHash>& methods){
			MethodHashShift shift = 0;
			MethodHash mask = 0;
			
			if(!getBestValues(methods, shift, mask)){
				assert(false && "Clash in method hashes");
			}
			
			VirtualTable table;
			table.table_.resize(mask + 1, 0);
			
			for(size_t i = 0; i < methods.size(); i++){
				table.table_.at((methods.at(i) >> shift) & mask) = methods.at(i);
			}
			
			table.shift_ = shift;
			table.mask_ = mask;
			
			return table;
		}
		
		VirtualTable VirtualTable::CalculateFromNames(const std::vector<std::string>& methods){
			std::vector<MethodHash> hashes;
			
			for(size_t i = 0; i < methods.size(); i++){
				hashes.push_back(CreateMethodNameHash(methods.at(i)));
			}
			
			return VirtualTable::CalculateFromHashes(hashes);
		}
		
		const std::vector<MethodHash>& VirtualTable::table() const {
			return table_;
		}
		
		MethodHash VirtualTable::mask() const {
			return mask_;
		}
		
		MethodHashShift VirtualTable::shift() const {
			return shift_;
		}
		
		std::string VirtualTable::toString() const {
			std::string s = makeString(
				"VirtualTable[shift = %llu, mask = %llu]{",
				(unsigned long long) shift_,
				(unsigned long long) mask_);
			
			for(size_t i = 0; i < table_.size(); i++){
				if(i != 0) s += ", ";
				s += makeString("%llu: %llu",
					(unsigned long long) i,
					(unsigned long long) table_.at(i));
			}
			
			s += "}";
			
			return s;
		}
			
		VirtualTable::VirtualTable()
			: mask_(0), shift_(0){ }
	
	}
	
}

