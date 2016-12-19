#ifndef LOCIC_CODEGEN_TEMPLATEDOBJECT_HPP
#define LOCIC_CODEGEN_TEMPLATEDOBJECT_HPP

#include <string>

#include <locic/AST/Value.hpp>
#include <locic/Support/Array.hpp>

namespace locic {
	
	namespace AST {
		
		class Function;
		
	}
	
	namespace SEM {
		
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class TemplatedObject {
			public:
				enum Kind {
					TYPEINSTANCE,
					FUNCTION
				};
				
				static TemplatedObject TypeInstance(const SEM::TypeInstance* typeInstance);
				
				static TemplatedObject Function(const SEM::TypeInstance* parentTypeInstance,
				                                const AST::Function* function);
				
				Kind kind() const;
				
				bool isTypeInstance() const;
				bool isFunction() const;
				
				const SEM::TypeInstance* typeInstance() const;
				
				const SEM::TypeInstance* parentTypeInstance() const;
				const AST::Function* function() const;
				
				AST::TemplateVarArray templateVariables() const;
				
				bool operator==(const TemplatedObject& other) const;
				bool operator!=(const TemplatedObject& other) const;
				bool operator<(const TemplatedObject& other) const;
				
				std::string toString() const;
				
			private:
				TemplatedObject(Kind pKind);
				
				Kind kind_;
				
				union {
					const SEM::TypeInstance* typeInstance;
					struct {
						const SEM::TypeInstance* parentTypeInstance;
						const AST::Function* function;
					} functionPair;
				} data_;
				
		};
		
		class TemplateInst {
			public:
				static TemplateInst Type(const AST::Type* type);
				
				static TemplateInst Function(const AST::Type* parentType,
				                             const AST::Function* function,
				                             llvm::ArrayRef<AST::Value> functionArgs);
				
				TemplateInst(TemplatedObject pObject, llvm::ArrayRef<AST::Value> pArguments);
				
				TemplateInst copy() const;
				
				TemplatedObject object() const;
				
				llvm::ArrayRef<AST::Value> arguments() const;
				
				bool allArgumentsAreTemplateVars(const AST::TemplateVarArray& templateVariables) const;
				
				bool allArgumentsAreSelfTemplateVars() const;
				
				bool operator<(const TemplateInst& other) const;
				
			private:
				TemplatedObject object_;
				Array<AST::Value, 10> arguments_;
				
		};
		
	}
	
}

#endif
