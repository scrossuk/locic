#ifndef LOCIC_CODEGEN_TYPESIZEKNOWLEDGE_HPP
#define LOCIC_CODEGEN_TYPESIZEKNOWLEDGE_HPP

namespace locic {
	
	namespace SEM {
		
		class Type;
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class Module;
		
		bool isObjectTypeSizeKnownInThisModule(Module& module, const SEM::TypeInstance* objectType);
		
		bool isTypeSizeKnownInThisModule(Module& module, const SEM::Type* type);
		
		bool isTypeSizeAlwaysKnown(Module& module, const SEM::Type* type);
		
	}
	
}

#endif
