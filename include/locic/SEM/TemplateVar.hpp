#ifndef LOCIC_SEM_TEMPLATEVAR_HPP
#define LOCIC_SEM_TEMPLATEVAR_HPP

#include <string>

#include <locic/Debug/TemplateVarInfo.hpp>
#include <locic/Support/Name.hpp>
#include <locic/Support/Optional.hpp>

namespace locic {

	namespace SEM {
	
		class Context;
		class Type;
		class TypeInstance;
		class Value;
		
		class TemplateVar {
			public:
				TemplateVar(Context& pContext, Name name, size_t index, bool isVirtual);
				
				Context& context() const;
				
				const Name& name() const;
				
				size_t index() const;
				
				bool isVirtual() const;
				
				void setType(const Type* type);
				const Type* type() const;
				
				Value selfRefValue() const;
				const Type* selfRefType() const;
				
				void setDebugInfo(Debug::TemplateVarInfo debugInfo);
				Optional<Debug::TemplateVarInfo> debugInfo() const;
				
				std::string toString() const;
				
			private:
				Context& context_;
				const Type* type_;
				const Type* selfRefType_;
				Name name_;
				size_t index_;
				bool isVirtual_;
				Optional<Debug::TemplateVarInfo> debugInfo_;
				
		};
		
	}
	
}

#endif
