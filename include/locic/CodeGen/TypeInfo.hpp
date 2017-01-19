#ifndef LOCIC_CODEGEN_TYPEINFO_HPP
#define LOCIC_CODEGEN_TYPEINFO_HPP

namespace locic {
	
	namespace AST {
		
		class Type;
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class Module;
		
		class TypeInfo {
		public:
			TypeInfo(Module& module);
			
			/**
			 * \brief Query whether type is passed by value.
			 * 
			 * Types will be passed by value if:
			 * 
			 * 1. Their size is known in all modules - If a module
			 *    doesn't know a type's size at compile-time it
			 *    can't pass/return by value.
			 * 2. They don't have a custom move method - Rather than
			 *    emitting raw loads/stores the compiler must call
			 *    the move method.
			 */
			bool isPassedByValue(const AST::Type* type) const;
			
			/**
			 * \brief Query whether a type's size is always known.
			 * 
			 * Some types, such as primitives, structs, etc. have a
			 * compile-time known size in all modules. This needs to
			 * be queried to determine if they can be passed by
			 * value.
			 */
			bool isSizeAlwaysKnown(const AST::Type* type) const;
			
			bool isObjectSizeAlwaysKnown(const AST::TypeInstance& typeInstance) const;
			
			/**
			 * \brief Query whether a type's size is known in this module.
			 */
			bool isSizeKnownInThisModule(const AST::Type* type) const;
			
			bool isObjectSizeKnownInThisModule(const AST::TypeInstance& typeInstance) const;
			
			bool hasCustomDestructor(const AST::Type* type) const;
			
			bool objectHasCustomDestructor(const AST::TypeInstance& typeInstance) const;
			
			bool objectHasCustomDestructorMethod(const AST::TypeInstance& typeInstance) const;
			
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
			bool objectHasCustomMove(const AST::TypeInstance& typeInstance) const;
			
			/**
			 * \brief Query whether a type instance has a custom move method.
			 * 
			 * This will determine whether the type has a custom move method;
			 * it doesn't check whether any of the member values have custom
			 * move methods.
			 */
			bool objectHasCustomMoveMethod(const AST::TypeInstance& typeInstance) const;
			
			/**
			 * \brief Query whether a type has a liveness indicator.
			 */
			bool hasLivenessIndicator(const AST::Type* type) const;
			
			/**
			 * \brief Query whether a type instance has a liveness indicator.
			 */
			bool objectHasLivenessIndicator(const AST::TypeInstance& typeInstance) const;
			
		private:
			Module& module_;
			
		};
		
	}
	
}

#endif
