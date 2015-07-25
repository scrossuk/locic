#ifndef LOCIC_CODEGEN_SEMFUNCTIONGENERATOR_HPP
#define LOCIC_CODEGEN_SEMFUNCTIONGENERATOR_HPP

namespace locic {
	
	class String;
	
	namespace SEM {
		
		class Function;
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class Module;
		
		/**
		 * \brief SEM Function Generator
		 * 
		 * This class generates LLVM IR functions that correspond to
		 * SEM functions.
		 */
		class SEMFunctionGenerator {
		public:
			SEMFunctionGenerator(Module& module);
			
			/**
			 * \brief Get function linkage.
			 * 
			 * This uses information about the function and its
			 * parent type to determine the correct linkage, which
			 * involves:
			 * 
			 *   * Giving primitive methods LinkOnce linkage.
			 *   * Giving internal functions Internal linkage.
			 *   * Giving auto-generated datatype methods LinkOnce
			 *     linkage.
			 *   * Giving other imported/exported External linkage.
			 */
			llvm::GlobalValue::LinkageTypes
			getLinkage(const SEM::TypeInstance* typeInstance,
			           const SEM::Function& function) const;
			
			llvm::GlobalValue::LinkageTypes
			getTypeLinkage(const SEM::TypeInstance& typeInstance) const;
			
			void
			addStandardFunctionAttributes(SEM::FunctionType type,
			                              llvm::Function& llvmFunction);
			
			llvm::Function*
			createNamedFunction(const String& name,
			                    SEM::FunctionType type,
			                    llvm::GlobalValue::LinkageTypes linkage);
			
			llvm::Function*
			getNamedFunction(const String& name,
			                 SEM::FunctionType type,
			                 llvm::GlobalValue::LinkageTypes linkage);
			
			String
			mangleUserFunctionName(const SEM::TypeInstance* typeInstance,
			                       const SEM::Function& function);
			
			/**
			 * \brief Get callable function declaration.
			 * 
			 * This gets a declaration which can be used to call the
			 * given function.
			 */
			llvm::Function*
			getCallableDecl(const SEM::TypeInstance* typeInstance,
			                const SEM::Function& function);
			
			/**
			 * \brief Get user function declaration.
			 * 
			 * This returns a declaration for the function's
			 * user-specified 'inner' function. For example, some
			 * built-in methods such as destructors have an outer
			 * callable function which calls the user's inner
			 * function.
			 */
			llvm::Function*
			getUserDecl(const SEM::TypeInstance* typeInstance,
			            const SEM::Function& function);
			
			/**
			 * \brief Generate function definition.
			 * 
			 * This generates the function code and returns a
			 * pointer to the generated LLVM function.
			 */
			llvm::Function*
			genDef(const SEM::TypeInstance* typeInstance,
			       const SEM::Function& function);
			
			/**
			 * \brief Create template function stub.
			 * 
			 * This creates a function to be referenced when calling methods
			 * on a templated type. For example:
			 * 
			 * template <typename T : SomeRequirement>
			 * void f(T& value) {
			 *     value.method();
			 * }
			 * 
			 * It would be possible to generate the virtual call inline,
			 * but it's possible to reference the method itself. For example:
			 * 
			 * template <typename T : SomeRequirement>
			 * void f(T& value) {
			 *     auto methodValue = value.method;
			 *     methodValue();
			 * }
			 * 
			 * Hence this function needs to be created so it can be
			 * subsequently referenced.
			 */
			llvm::Function*
			genTemplateFunctionStub(const SEM::TemplateVar* templateVar,
			                        const String& functionName,
			                        SEM::FunctionType functionType,
			                        llvm::DebugLoc debugLoc);
			
		private:
			Module& module_;
			
		};
		
	}
	
}

#endif
