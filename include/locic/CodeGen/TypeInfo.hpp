#ifndef LOCIC_CODEGEN_TYPEINFO_HPP
#define LOCIC_CODEGEN_TYPEINFO_HPP

namespace locic {
	
	namespace SEM {
		
		class Type;
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class Module;
		
		class TypeInfo {
		public:
			TypeInfo(Module& module);
			
			bool isSizeAlwaysKnown(const SEM::Type* type) const;
			
			bool isObjectSizeAlwaysKnown(const SEM::TypeInstance& typeInstance) const;
			
			bool isSizeKnownInThisModule(const SEM::Type* type) const;
			
			bool isObjectSizeKnownInThisModule(const SEM::TypeInstance& typeInstance) const;
			
			bool hasCustomDestructor(const SEM::Type* type) const;
			
			bool objectHasCustomDestructor(const SEM::TypeInstance& typeInstance) const;
			
			bool hasCustomMove(const SEM::Type* type) const;
			
			bool objectHasCustomMove(const SEM::TypeInstance& typeInstance) const;
			
			bool objectHasCustomMoveMethod(const SEM::TypeInstance& typeInstance) const;
			
			bool hasLivenessIndicator(const SEM::Type* type) const;
			
			bool objectHasLivenessIndicator(const SEM::TypeInstance& typeInstance) const;
			
		private:
			Module& module_;
			
		};
		
	}
	
}

#endif
