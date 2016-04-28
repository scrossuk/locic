#ifndef LOCIC_SEMANTICANALYSIS_TYPECAPABILITIES_HPP
#define LOCIC_SEMANTICANALYSIS_TYPECAPABILITIES_HPP

#include <locic/SEM/TypeArray.hpp>

namespace locic {
	
	class String;
	
	namespace SEM {
		
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		class TypeCapabilities {
		public:
			TypeCapabilities(Context& context);
			
			const SEM::Type*
			getCapabilityType(String capability,
			                  SEM::TypeArray templateArgs);
			
			bool
			checkCapabilityWithType(const SEM::Type* const rawType,
			                        String capability,
			                        const SEM::Type* requireType);
			
			bool
			checkCapability(const SEM::Type* const rawType,
			                String capability, SEM::TypeArray templateArgs);
			
			bool hasCallMethod(const SEM::Type* type);
			
			bool supportsImplicitCast(const SEM::Type* type);
			
			bool supportsImplicitCopy(const SEM::Type* type);
			
			bool supportsExplicitCopy(const SEM::Type* type);
			
			bool supportsNoExceptImplicitCopy(const SEM::Type* type);
			
			bool supportsNoExceptExplicitCopy(const SEM::Type* type);
			
			bool supportsCompare(const SEM::Type* type);
			
			bool supportsNoExceptCompare(const SEM::Type* type);
			
			bool supportsMove(const SEM::Type* const type);
			
			bool supportsDissolve(const SEM::Type* const type);
			
			bool supportsCall(const SEM::Type* const type);
			
		private:
			Context& context_;
			
		};
		
	}
	
}

#endif
