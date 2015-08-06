#ifndef LOCIC_CODEGEN_SEMCODEEMITTER_HPP
#define LOCIC_CODEGEN_SEMCODEEMITTER_HPP

namespace locic {
	
	namespace SEM {
		
		class Function;
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class Function;
		
		class SEMCodeEmitter {
		public:
			SEMCodeEmitter(Function& functionGenerator);
			
			void emitFunctionCode(const SEM::TypeInstance* typeInstance,
			                      const SEM::Function& function,
			                      bool isInnerMethod);
			
			void emitDefaultFunctionCode(const SEM::TypeInstance& typeInstance,
			                             const SEM::Function& function,
			                             bool isInnerMethod);
			
			void emitUserFunctionCode(const SEM::Function& function);
			
			void emitParameterAllocas(const SEM::Function& function);
			
			void emitScopeFunctionCode(const SEM::Function& function);
			
		private:
			Function& functionGenerator_;
			
		};
		
	}
	
}

#endif
