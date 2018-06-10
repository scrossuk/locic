#ifndef LOCIC_SEMANTICANALYSIS_CASTGENERATOR_HPP
#define LOCIC_SEMANTICANALYSIS_CASTGENERATOR_HPP

#include <locic/Frontend/ResultOrDiag.hpp>

namespace locic {
	
	class OptionalDiag;
	
	namespace AST {
		
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class CastRules;
		class CastSequence;
		class Context;
		class SatisfyChecker;
		
		class CastGenerator {
		public:
			CastGenerator(Context& context, SatisfyChecker& checker);
			
			ResultOrDiag<CastSequence>
			implicitCast(const AST::Type* sourceType,
			             const AST::Type* destType, bool canBind);
			
			ResultOrDiag<CastSequence>
			implicitCastNoop(const AST::Type* sourceType,
			                 const AST::Type* destType);
			
			OptionalDiag
			implicitCastAnyToAny(const CastRules& rules, CastSequence& cast,
			                     const AST::Type* destType);
			
			OptionalDiag
			implicitCastRefToRef(const CastRules& rules, CastSequence& cast,
			                     const AST::Type* destType);
			
			OptionalDiag
			implicitCastValueToRef(const CastRules& rules, CastSequence& cast,
			                       const AST::Type* destType);
			
			OptionalDiag
			implicitCastPolyRefToRef(const CastRules& rules, CastSequence& cast,
			                         const AST::Type* destType);
			
			OptionalDiag
			implicitCastValueToValue(const CastRules& rules, CastSequence& cast,
			                         const AST::Type* destType);
			
			OptionalDiag
			implicitCastVariant(CastSequence& cast,
			                    const AST::Type* destType);
			
			OptionalDiag
			implicitCastUser(CastSequence& cast,
			                 const AST::Type* destType);
			
			OptionalDiag
			implicitCastNoopOnly(CastSequence& cast,
			                     const AST::Type* destType);
			
			OptionalDiag
			implicitCopyRefToValue(const CastRules& rules, CastSequence& cast);
			
		private:
			Context& context_;
			SatisfyChecker& checker_;
			
		};
		
	}
	
}

#endif
