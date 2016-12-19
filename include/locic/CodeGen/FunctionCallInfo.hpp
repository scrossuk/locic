#ifndef LOCIC_CODEGEN_FUNCTIONCALLINFO_HPP
#define LOCIC_CODEGEN_FUNCTIONCALLINFO_HPP

#include <locic/Support/Optional.hpp>

namespace locic {
	
	namespace AST {
		
		class Function;
		class FunctionType;
		class Type;
		class Value;
		
	}
	
	namespace CodeGen {
		
		class Function;
		class Module;
		class PendingResult;
		struct TypeInfoComponents;
		struct VirtualMethodComponents;
		
		/**
		 * \brief Function Call Information
		 * 
		 * This data provides the information needed to call a
		 * function, which includes:
		 * 
		 * - A pointer to the function.
		 * - The template generator for the function (if it's templated).
		 * - A pointer to the 'context' (i.e. the parent object).
		 */
		struct FunctionCallInfo {
			llvm::Value* functionPtr;
			llvm::Value* templateGenerator;
			llvm::Value* contextPointer;
			
			FunctionCallInfo()
				: functionPtr(nullptr),
				templateGenerator(nullptr),
				contextPointer(nullptr) { }
		};
		
		/**
		 * \brief Get function pointer.
		 * 
		 * This call returns a function pointer to the given function;
		 * note that this might be *INDIRECT* since in some cases a
		 * translation stub function must be generated.
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
		 * This is resolved by creating a function stub that
		 * translates between the outer use of the function and
		 * its internal implementation.
		 * 
		 * A key point is that calling this for the same function
		 * but *different* function types will in some cases
		 * return different function pointers.
		 * 
		 * \param function The current function generator (may need to generate a pointer cast).
		 * \param parentType The parent type of the function (or NULL if none).
		 * \param semFunction The SEM function to which we're getting a pointer.
		 * \param functionType The type of the function (as it's being used).
		 * \return The function pointer.
		 */
		llvm::Value* genFunctionRef(Function& function, const AST::Type* parentType,
		                            const AST::Function* astFunction, AST::FunctionType functionType);
		
		/**
		 * \brief Query whether value is a trivial function.
		 * 
		 * A trivial function (or method) is one that can be automatically
		 * generated inline by this front-end. For example:
		 * 
		 * int a = ...;
		 * int b = a.add(1); // Written with method call for clarity.
		 * 
		 * In this case the 'add' call on the primitive int type is
		 * considered trivial (as are other primitive methods); the
		 * front-end can then simply emit an 'ADD' instruction instead.
		 * 
		 * \param module The current module.
		 * \param value The SEM value which may be a trivial function.
		 * \return Whether the value is a trivial function.
		 */
		bool isTrivialFunction(Module& module, const AST::Value& value);
		
		/**
		 * \brief Generate trivial function call.
		 * 
		 * Once a value has been determined to be a trivial function,
		 * the function can be 'called' by using this. The trivial
		 * function will actually then be generated inline.
		 * 
		 * \param function The current function generator.
		 * \param value The function SEM value.
		 * \param args The SEM value arguments.
		 * \param hintResultValue A result pointer (to avoid unnecessary allocs/moves) or null if none available.
		 * \return The result of the function call.
		 */
		llvm::Value* genTrivialFunctionCall(Function& function, const AST::Value& value, llvm::ArrayRef<AST::Value> args,
			llvm::Value* const hintResultValue = nullptr);
		
		FunctionCallInfo genFunctionCallInfo(Function& function, const AST::Value& value);
		
		TypeInfoComponents genTypeInfoComponents(Function& function, const AST::Value& value);
		
		TypeInfoComponents genBoundTypeInfoComponents(Function& function, const AST::Value& value);
		
		VirtualMethodComponents genVirtualMethodComponents(Function& function, const AST::Value& value);
		
	}
	
}

#endif
