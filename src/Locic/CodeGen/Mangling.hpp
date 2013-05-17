#ifndef LOCIC_CODEGEN_MANGLING_HPP
#define LOCIC_CODEGEN_MANGLING_HPP

#include <string>
#include <vector>

#include <Locic/Name.hpp>
#include <Locic/SEM.hpp>

namespace Locic {

	namespace CodeGen {
	
		std::string mangleFunctionName(const Name& name);
		
		std::string mangleType(SEM::Type* type);
		
		std::string mangleObjectType(SEM::TypeInstance* typeInstance, const std::vector<SEM::Type*>& templateArguments);
		
		std::string mangleTypeList(const std::vector<SEM::Type*> typeList);
		
		std::string mangleTypeName(const Name& name);
		
	}
	
}

#endif
