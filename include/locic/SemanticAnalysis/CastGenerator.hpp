#ifndef LOCIC_SEMANTICANALYSIS_CASTGENERATOR_HPP
#define LOCIC_SEMANTICANALYSIS_CASTGENERATOR_HPP

#include <locic/Frontend/ResultOrDiag.hpp>

namespace locic {
	
	class OptionalDiag;
	
	namespace AST {
		
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
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
			implicitCastAnyToAny(CastSequence& cast,
			                     const AST::Type* destType);
			
			OptionalDiag
			implicitCastRefToRef(CastSequence& cast,
			                     const AST::Type* destType);
			
			OptionalDiag
			implicitCastValueToRef(CastSequence& cast,
			                       const AST::Type* destType);
			
			OptionalDiag
			implicitCastPolyRefToRef(CastSequence& cast,
			                         const AST::Type* destType);
			
			OptionalDiag
			implicitCastValueToValue(CastSequence& cast,
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
			implicitCopyRefToValue(CastSequence& cast);
			
		private:
			Context& context_;
			SatisfyChecker& checker_;
			
		};
		
	}
	
}

#endif
