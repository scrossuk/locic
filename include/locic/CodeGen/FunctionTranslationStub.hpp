#ifndef LOCIC_CODEGEN_FUNCTIONTRANSLATIONSTUB_HPP
#define LOCIC_CODEGEN_FUNCTIONTRANSLATIONSTUB_HPP

namespace locic {
	
	namespace SEM {
		
		class FunctionType;
		
	}
	
	namespace CodeGen {
		
		class Function;
		class Module;
		
		/**
		 * \brief Create translation function stub.
		 * 
		 * This creates a function that performs a translation between
		 * accepting/returning values by pointer versus by value.
		 * 
		 * For example:
		 * 
		 * template <typename T : movable>
		 * void f(T value);
		 * 
		 * f<int>(10);
		 * 
		 * Here the function is called with an integer value and as
		 * with all primitives it will be passed by value. However
		 * the function is templated and therefore must accept its
		 * argument by pointer (to support non-primitive types).
		 * 
		 * This problem is resolved by creating a translation
		 * function stub that is roughly:
		 * 
		 * void translateFunctionForF(int value) {
		 *   int* stackMemoryPtr = alloca(sizeof(int));
		 *   *stackMemoryPtr = value;
		 *   f(stackMemoryPtr);
		 * }
		 * 
		 * This translation could be performed inline while the
		 * code is generated, but this would prevent referencing
		 * the function without calling it; for example:
		 * 
		 * auto theFunction = f<int>;
		 * theFunction(10);
		 * 
		 * It is of course notable that the function reference
		 * here actually refers to the *stub* and not to the
		 * templated function.
		 * 
		 * \param module The module in which code is being generated.
		 * \param function The function we ultimately want to call.
                 * \param functionType The type of the function being used.
		 * \param translatedFunctionType The function signature we'd like to use.
		 * \return The translation function stub.
		 */
		llvm::Function* genFunctionTranslationStub(Module& module,
		                                           llvm::Function* function,
		                                           SEM::FunctionType functionType,
		                                           SEM::FunctionType translatedFunctionType);
		
		/**
		 * \brief Get pointer to function with translated type.
		 * 
		 * As explained above, in some cases templates cause function
		 * types to be translated in ways that affect how code
		 * generation emits parameter passing (i.e. passing by value
		 * versus passing by pointer).
		 * 
		 * This function generates a pointer to the given function of
		 * the given translated type; in many cases no translation
		 * stub needs to be generated.
		 * 
		 * \param functionGenerator The function generator for emitting instructions.
		 * \param function The function we ultimately want to call.
                 * \param functionType The type of the function being used.
		 * \param translatedFunctionType The function signature we'd like to use.
		 * \return The translated function pointer.
		 */
		llvm::Value* genTranslatedFunctionPointer(Function& functionGenerator,
		                                          llvm::Function* function,
		                                          SEM::FunctionType functionType,
		                                          SEM::FunctionType translatedFunctionType);
		
	}
	
}

#endif
