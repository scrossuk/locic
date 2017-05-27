#ifndef LOCIC_SEMANTICANALYSIS_CASTOPERATION_HPP
#define LOCIC_SEMANTICANALYSIS_CASTOPERATION_HPP

namespace locic {
	
	class OptionalDiag;
	
	namespace AST {
		
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		class CastOperation {
		public:
			CastOperation(Context& context,
			              const AST::Type* sourceType, bool isNoop,
			              bool canBind);
			
			Context& context();
			
			const AST::Type* type() const;
			
			bool isNoop() const;
			
			bool canBind() const;
			
			void addBind();
			
			void addPolyCast(const AST::Type* destType);
			
			void addNoopCast(const AST::Type* destType);
			
			void addCopy(const AST::Type* destType);
			
			void addUserCast(const AST::Type* destType);
			
			void addVariantCast(const AST::Type* destType);
			
		private:
			Context& context_;
			const AST::Type* type_;
			bool isNoop_;
			bool canBind_;
			
		};
		
	}
	
}

#endif
