#ifndef LOCIC_CODEGEN_UNWINDACTION_HPP
#define LOCIC_CODEGEN_UNWINDACTION_HPP

#include <locic/CodeGen/LLVMIncludes.hpp>

namespace locic {

	namespace CodeGen {
	
		class UnwindAction {
			public:
				static UnwindAction Destroy(SEM::Type* type, llvm::Value* value);
				
				static UnwindAction CatchException(llvm::BasicBlock* catchBlock, llvm::Constant* catchTypeInfo);
				
				static UnwindAction ScopeMarker();
				
				enum Kind {
					DESTRUCTOR,
					CATCH,
					SCOPEMARKER
				};
				
				Kind kind() const;
				
				bool isDestructor() const;
				
				bool isCatch() const;
				
				bool isScopeMarker() const;
				
				SEM::Type* destroyType() const;
				
				llvm::Value* destroyValue() const;
				
				llvm::BasicBlock* catchBlock() const;
				
				llvm::Constant* catchTypeInfo() const;
				
			private:
				inline UnwindAction(Kind pKind)
					: kind_(pKind) { }
				
				Kind kind_;
				
				union {
					struct {
						SEM::Type* type;
						llvm::Value* value;
					} destructor_;
					
					struct {
						llvm::BasicBlock* block;
						llvm::Constant* typeInfo;
					} catch_;
				};
				
		};
		
	}
	
}

#endif
