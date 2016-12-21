#ifndef LOCIC_CODEGEN_ASTFUNCTIONGENERATOR_HPP
#define LOCIC_CODEGEN_ASTFUNCTIONGENERATOR_HPP

namespace locic {
	
	class String;
	
	namespace AST {
		
		class Function;
		class FunctionType;
		class TemplateVar;
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class Module;
		
		/**
		 * \brief AST Function Generator
		 * 
		 * This class generates LLVM IR functions that correspond to AST
		 * functions.
		 */
		class ASTFunctionGenerator {
		public:
			ASTFunctionGenerator(Module& module);
			
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
			getLinkage(const AST::TypeInstance* typeInstance,
			           const AST::Function& function) const;
			
			llvm::GlobalValue::LinkageTypes
			getTypeLinkage(const AST::TypeInstance& typeInstance) const;
			
			void
			addStandardFunctionAttributes(AST::FunctionType type,
			                              llvm::Function& llvmFunction);
			
			llvm::Function*
			createNamedFunction(const String& name,
			                    AST::FunctionType type,
			                    llvm::GlobalValue::LinkageTypes linkage);
			
			llvm::Function*
			getNamedFunction(const String& name,
			                 AST::FunctionType type,
			                 llvm::GlobalValue::LinkageTypes linkage);
			
			/**
			 * \brief Get LLVM function for AST function.
			 * 
			 * This gets the LLVM function that corresponds to the
			 * given AST function.
			 */
			llvm::Function*
			getDecl(const AST::TypeInstance* typeInstance,
			        const AST::Function& function,
			        bool isInnerMethod = false);
			
			/**
			 * \brief Query whether definition should be generated.
			 */
			bool
			hasDef(const AST::TypeInstance* typeInstance,
			       const AST::Function& function);
			
			/**
			 * \brief Generate function definition.
			 * 
			 * This generates the function code and returns a
			 * pointer to the generated LLVM function.
			 */
			llvm::Function*
			genDef(const AST::TypeInstance* typeInstance,
			       const AST::Function& function,
			       bool isInnerMethod = false);
			
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
			genTemplateFunctionStub(const AST::TemplateVar* templateVar,
			                        const String& functionName,
			                        AST::FunctionType functionType,
			                        llvm::DebugLoc debugLoc);
			
		private:
			Module& module_;
			
		};
		
	}
	
}

#endif
