#ifndef LOCIC_SEMANTICANALYSIS_CASTGENERATOR_HPP
#define LOCIC_SEMANTICANALYSIS_CASTGENERATOR_HPP

namespace locic {
	
	class OptionalDiag;
	
	namespace AST {
		
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class CastOperation;
		class Context;
		class SatisfyChecker;
		
		class CastGenerator {
		public:
			CastGenerator(Context& context, SatisfyChecker& checker);
			
			OptionalDiag
			implicitCast(const AST::Type* sourceType,
			             const AST::Type* destType, bool canBind);
			
			OptionalDiag
			implicitCastNoop(const AST::Type* sourceType,
			                 const AST::Type* destType);
			
			OptionalDiag
			implicitCastAnyToAny(CastOperation& cast,
			                     const AST::Type* destType);
			
			OptionalDiag
			implicitCastRefToRef(CastOperation& cast,
			                     const AST::Type* destType);
			
			OptionalDiag
			implicitCastValueToRef(CastOperation& cast,
			                       const AST::Type* destType);
			
			OptionalDiag
			implicitCastPolyRefToRef(CastOperation& cast,
			                         const AST::Type* destType);
			
			OptionalDiag
			implicitCastValueToValue(CastOperation& cast,
			                         const AST::Type* destType);
			
			OptionalDiag
			implicitCastVariant(CastOperation& cast,
			                    const AST::Type* destType);
			
			OptionalDiag
			implicitCastUser(CastOperation& cast,
			                 const AST::Type* destType);
			
			OptionalDiag
			implicitCastNoopOnly(CastOperation& cast,
			                     const AST::Type* destType);
			
			OptionalDiag
			implicitCopyRefToValue(CastOperation& cast);
			
		private:
			Context& context_;
			SatisfyChecker& checker_;
			
		};
		
	}
	
}

#endif
