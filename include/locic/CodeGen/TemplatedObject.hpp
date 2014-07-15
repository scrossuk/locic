#ifndef LOCIC_CODEGEN_TEMPLATEDOBJECT_HPP
#define LOCIC_CODEGEN_TEMPLATEDOBJECT_HPP

#include <locic/SEM.hpp>

#include <locic/CodeGen/LLVMIncludes.hpp>

namespace locic {

	namespace CodeGen {
		
		class TemplatedObject {
			public:
				enum Kind {
					TYPEINSTANCE,
					FUNCTION
				};
				
				static TemplatedObject TypeInstance(SEM::TypeInstance* typeInstance);
				
				static TemplatedObject Function(SEM::TypeInstance* parentTypeInstance, SEM::Function* function);
				
				Kind kind() const;
				
				bool isTypeInstance() const;
				bool isFunction() const;
				
				SEM::TypeInstance* typeInstance() const;
				
				SEM::TypeInstance* parentTypeInstance() const;
				SEM::Function* function() const;
				
				bool operator==(const TemplatedObject& other) const;
				bool operator!=(const TemplatedObject& other) const;
				bool operator<(const TemplatedObject& other) const;
				
			private:
				TemplatedObject(Kind pKind);
				
				Kind kind_;
				
				union {
					SEM::TypeInstance* typeInstance;
					struct {
						SEM::TypeInstance* parentTypeInstance;
						SEM::Function* function;
					} functionPair;
				} data_;
				
		};
		
		class TemplateInst {
			public:
				static TemplateInst Type(SEM::Type* type);
				
				static TemplateInst Function(SEM::Type* parentType, SEM::Function* function, llvm::ArrayRef<SEM::Type*> functionArgs);
				
				TemplateInst(TemplatedObject pObject, llvm::ArrayRef<SEM::Type*> pArguments);
				
				TemplatedObject object() const;
				
				llvm::ArrayRef<SEM::Type*> arguments() const;
				
				bool operator<(const TemplateInst& other) const;
				
			private:
				TemplatedObject object_;
				llvm::SmallVector<SEM::Type*, 10> arguments_;
				
		};
		
	}
	
}

#endif
