#ifndef LOCIC_CODEGEN_SCOPEEMITTER_HPP
#define LOCIC_CODEGEN_SCOPEEMITTER_HPP

namespace locic {
	
	namespace AST {
		
		class Scope;
		
	}
	
	namespace CodeGen {
		
		class IREmitter;
		
		class ScopeEmitter {
		public:
			ScopeEmitter(IREmitter& irEmitter);
			
			void emitScope(const AST::Scope& scope);
			
		private:
			IREmitter& irEmitter_;
			
		};
		
	}
	
}

#endif
