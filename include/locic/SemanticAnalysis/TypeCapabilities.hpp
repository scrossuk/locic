#ifndef LOCIC_SEMANTICANALYSIS_TYPECAPABILITIES_HPP
#define LOCIC_SEMANTICANALYSIS_TYPECAPABILITIES_HPP

namespace locic {
	
	namespace SEM {
		
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		bool hasCallMethod(Context& context, const SEM::Type* type);
		
		bool supportsImplicitCast(Context& context, const SEM::Type* type);
		
		bool supportsImplicitCopy(Context& context, const SEM::Type* type);
		
		bool supportsExplicitCopy(Context& context, const SEM::Type* type);
		
		bool supportsNoExceptImplicitCopy(Context& context, const SEM::Type* type);
		
		bool supportsNoExceptExplicitCopy(Context& context, const SEM::Type* type);
		
		bool supportsCompare(Context& context, const SEM::Type* type);
		
		bool supportsNoExceptCompare(Context& context, const SEM::Type* type);
		
		bool supportsMove(Context& context, const SEM::Type* const type);
		
		bool supportsDissolve(Context& context, const SEM::Type* const type);
		
		bool supportsCall(Context& context, const SEM::Type* const type);
		
	}
	
}

#endif
