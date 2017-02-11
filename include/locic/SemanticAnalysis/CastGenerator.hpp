#ifndef LOCIC_SEMANTICANALYSIS_CASTGENERATOR_HPP
#define LOCIC_SEMANTICANALYSIS_CASTGENERATOR_HPP

namespace locic {
	
	class OptionalDiag;
	
	namespace AST {
		
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		class SatisfyChecker;
		
		class CastGenerator {
		public:
			CastGenerator(Context& context, SatisfyChecker& checker,
			              const AST::Type* sourceType, bool isNoop,
			              bool canBind);
			
			const AST::Type* type() const;
			
			void setSourceType(const AST::Type* sourceType);
			
			OptionalDiag
			implicitCast(const AST::Type* destType);
			
			OptionalDiag
			implicitCastRefToRef(const AST::Type* destType);
			
			OptionalDiag
			implicitCastValueToRef(const AST::Type* destType);
			
			OptionalDiag
			implicitCastPolyRefToRef(const AST::Type* destType);
			
			OptionalDiag
			implicitCastValueToValue(const AST::Type* destType);
			
			OptionalDiag
			implicitCastVariant(const AST::Type* destType);
			
			OptionalDiag
			implicitCastUser(const AST::Type* destType);
			
			OptionalDiag
			implicitCastNoop(const AST::Type* destType);
			
			OptionalDiag
			implicitCopyRefToValue();
			
		private:
			Context& context_;
			SatisfyChecker& checker_;
			const AST::Type* type_;
			bool isNoop_;
			bool canBind_;
			
		};
		
	}
	
}

#endif
