#ifndef LOCIC_SEMANTICANALYSIS_TYPECAPABILITIES_HPP
#define LOCIC_SEMANTICANALYSIS_TYPECAPABILITIES_HPP

#include <locic/AST/TypeArray.hpp>

namespace locic {
	
	class String;
	
	namespace AST {
		
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		class TypeCapabilities {
		public:
			TypeCapabilities(Context& context);
			
			const AST::Type*
			getCapabilityType(String capability,
			                  AST::TypeArray templateArgs);
			
			bool
			checkCapabilityWithType(const AST::Type* const rawType,
			                        String capability,
			                        const AST::Type* requireType);
			
			bool
			checkCapability(const AST::Type* const rawType,
			                String capability, AST::TypeArray templateArgs);
			
			bool hasCallMethod(const AST::Type* type);
			
			bool supportsImplicitCast(const AST::Type* type);
			
			bool supportsImplicitCast(const AST::Type* type,
			                          const AST::Type* toType);
			
			bool supportsImplicitCopy(const AST::Type* type);
			
			bool supportsExplicitCopy(const AST::Type* type);
			
			bool supportsNoExceptImplicitCopy(const AST::Type* type);
			
			bool supportsNoExceptExplicitCopy(const AST::Type* type);
			
			bool supportsCompare(const AST::Type* type);
			
			bool supportsNoExceptCompare(const AST::Type* type);
			
			bool supportsMove(const AST::Type* const type);
			
			bool supportsCall(const AST::Type* const type);
			
			bool isSized(const AST::Type* type);
			
		private:
			Context& context_;
			
		};
		
	}
	
}

#endif
