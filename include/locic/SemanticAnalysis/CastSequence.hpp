#ifndef LOCIC_SEMANTICANALYSIS_CASTSEQUENCE_HPP
#define LOCIC_SEMANTICANALYSIS_CASTSEQUENCE_HPP

namespace locic {
	
	class OptionalDiag;
	
	namespace AST {
		
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		class CastSequence {
		public:
			CastSequence(Context& context,
			             const AST::Type* sourceType);
			
			const AST::Type* type() const;
			
			void addBind();
			
			void addPolyCast(const AST::Type* destType);
			
			void addNoopCast(const AST::Type* destType);
			
			void addCopy(const AST::Type* destType);
			
			void addUserCast(const AST::Type* destType);
			
			void addVariantCast(const AST::Type* destType);
			
		private:
			Context& context_;
			const AST::Type* type_;
			
		};
		
	}
	
}

#endif
