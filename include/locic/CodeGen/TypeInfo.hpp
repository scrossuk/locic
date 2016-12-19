#ifndef LOCIC_CODEGEN_TYPEINFO_HPP
#define LOCIC_CODEGEN_TYPEINFO_HPP

namespace locic {
	
	namespace AST {
		
		class Type;
		
	}
	
	namespace SEM {
		
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class Module;
		
		class TypeInfo {
		public:
			TypeInfo(Module& module);
			
			bool canPassByValue(const AST::Type* type) const;
			
			bool isSizeAlwaysKnown(const AST::Type* type) const;
			
			bool isObjectSizeAlwaysKnown(const SEM::TypeInstance& typeInstance) const;
			
			bool isSizeKnownInThisModule(const AST::Type* type) const;
			
			bool isObjectSizeKnownInThisModule(const SEM::TypeInstance& typeInstance) const;
			
			bool hasCustomDestructor(const AST::Type* type) const;
			
			bool objectHasCustomDestructor(const SEM::TypeInstance& typeInstance) const;
			
			bool objectHasCustomDestructorMethod(const SEM::TypeInstance& typeInstance) const;
			
			/**
			 * \brief Query whether a type has a custom move operation.
			 * 
			 * This will determine whether the type has a custom
			 * move operation (i.e. we can't just issue a memcpy).
			 * 
			 * For class declarations this always returns true, since
			 * we must assume they have a custom move method. For
			 * known types they will have a custom move method if
			 * one of their child types has a custom move method,
			 * so this will also return true in that case.
			 */
			bool hasCustomMove(const AST::Type* type) const;
			
			/**
			 * \brief Query whether a type instance has a custom move operation.
			 * 
			 * This will determine whether the type has a custom
			 * move method.
			 * 
			 * For class declarations this always returns true, since
			 * we must assume they have a custom move method. For
			 * known types they will have a custom move method if
			 * one of their child types has a custom move method, so
			 * this will also return true in that case.
			 */
			bool objectHasCustomMove(const SEM::TypeInstance& typeInstance) const;
			
			/**
			 * \brief Query whether a type instance has a custom move method.
			 * 
			 * This will determine whether the type has a custom move method;
			 * it doesn't check whether any of the member values have custom
			 * move methods.
			 */
			bool objectHasCustomMoveMethod(const SEM::TypeInstance& typeInstance) const;
			
			/**
			 * \brief Query whether a type has a liveness indicator.
			 */
			bool hasLivenessIndicator(const AST::Type* type) const;
			
			/**
			 * \brief Query whether a type instance has a liveness indicator.
			 */
			bool objectHasLivenessIndicator(const SEM::TypeInstance& typeInstance) const;
			
		private:
			Module& module_;
			
		};
		
	}
	
}

#endif
