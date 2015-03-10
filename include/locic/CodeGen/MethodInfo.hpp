#ifndef LOCIC_CODEGEN_METHODINFO_HPP
#define LOCIC_CODEGEN_METHODINFO_HPP

#include <locic/SEM.hpp>

#include <locic/Support/String.hpp>

namespace locic {
	
	namespace CodeGen {
		
		struct MethodInfo {
			const SEM::Type* parentType;
			String name;
			const SEM::Type* functionType;
			SEM::TypeArray templateArgs;
			
			MethodInfo(const SEM::Type* const argParentType, const String& argName,
				const SEM::Type* const argFunctionType, SEM::TypeArray argTemplateArgs)
			: parentType(argParentType), name(argName), functionType(argFunctionType), templateArgs(std::move(argTemplateArgs)) { }
		};
		
	}
	
}

#endif
