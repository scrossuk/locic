#ifndef LOCIC_CODEGEN_TEMPLATEDOBJECT_HPP
#define LOCIC_CODEGEN_TEMPLATEDOBJECT_HPP

#include <locic/SEM/Value.hpp>
#include <locic/Support/Array.hpp>

namespace locic {
	
	namespace SEM {
		
		class Function;
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
				
				static TemplatedObject Function(const SEM::TypeInstance* parentTypeInstance, SEM::Function* function);
				
				Kind kind() const;
				
				bool isTypeInstance() const;
				bool isFunction() const;
				
				const SEM::TypeInstance* typeInstance() const;
				
				const SEM::TypeInstance* parentTypeInstance() const;
				SEM::Function* function() const;
				
				bool operator==(const TemplatedObject& other) const;
				bool operator!=(const TemplatedObject& other) const;
				bool operator<(const TemplatedObject& other) const;
				
			private:
				TemplatedObject(Kind pKind);
				
				Kind kind_;
				
				union {
					const SEM::TypeInstance* typeInstance;
					struct {
						const SEM::TypeInstance* parentTypeInstance;
						SEM::Function* function;
					} functionPair;
				} data_;
				
		};
		
		class TemplateInst {
			public:
				static TemplateInst Type(const SEM::Type* type);
				
				static TemplateInst Function(const SEM::Type* parentType, SEM::Function* function, llvm::ArrayRef<SEM::Value> functionArgs);
				
				TemplateInst(TemplatedObject pObject, llvm::ArrayRef<SEM::Value> pArguments);
				
				TemplateInst copy() const;
				
				TemplatedObject object() const;
				
				llvm::ArrayRef<SEM::Value> arguments() const;
				
				bool operator<(const TemplateInst& other) const;
				
			private:
				TemplatedObject object_;
				Array<SEM::Value, 10> arguments_;
				
		};
		
	}
	
}

#endif
