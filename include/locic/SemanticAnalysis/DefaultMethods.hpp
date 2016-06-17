#ifndef LOCIC_SEMANTICANALYSIS_DEFAULTMETHODS_HPP
#define LOCIC_SEMANTICANALYSIS_DEFAULTMETHODS_HPP

#include <memory>

namespace locic {
	
	class Name;
	
	namespace Debug {
		
		struct FunctionInfo;
		class SourceLocation;
		
	}
	
	namespace SEM {
		
		class Function;
		class TypeInstance;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		Debug::FunctionInfo makeDefaultFunctionInfo(const SEM::TypeInstance& parentType, const SEM::Function& function);
		
		class DefaultMethods {
		public:
			DefaultMethods(Context& context);
			
			std::unique_ptr<SEM::Function>
			createDefaultConstructorDecl(SEM::TypeInstance* typeInstance,
			                             const Name& name);
			
			std::unique_ptr<SEM::Function>
			createDefaultAlignMaskDecl(SEM::TypeInstance* typeInstance,
			                           const Name& name);
			
			std::unique_ptr<SEM::Function>
			createDefaultSizeOfDecl(SEM::TypeInstance* typeInstance,
			                        const Name& name);
			
			std::unique_ptr<SEM::Function>
			createDefaultDestroyDecl(SEM::TypeInstance* typeInstance,
			                         const Name& name);
			
			std::unique_ptr<SEM::Function>
			createDefaultMoveDecl(SEM::TypeInstance* typeInstance,
			                      const Name& name);
			
			std::unique_ptr<SEM::Function>
			createDefaultImplicitCopyDecl(SEM::TypeInstance* typeInstance,
			                              const Name& name);
			
			std::unique_ptr<SEM::Function>
			createDefaultExplicitCopyDecl(SEM::TypeInstance* typeInstance,
			                              const Name& name);
			
			std::unique_ptr<SEM::Function>
			createDefaultCompareDecl(SEM::TypeInstance* typeInstance,
			                         const Name& name);
			
			std::unique_ptr<SEM::Function>
			createDefaultSetDeadDecl(SEM::TypeInstance* typeInstance,
			                         const Name& name);
			
			std::unique_ptr<SEM::Function>
			createDefaultIsLiveDecl(SEM::TypeInstance* typeInstance,
			                        const Name& name);
			
			std::unique_ptr<SEM::Function>
			createDefaultMethodDecl(SEM::TypeInstance* typeInstance,
			                        bool isStatic, const Name& name,
			                        const Debug::SourceLocation& location);
			
			bool hasDefaultConstructor(SEM::TypeInstance* typeInstance);
			
			bool hasDefaultAlignMask(SEM::TypeInstance* typeInstance);
			
			bool hasDefaultSizeOf(SEM::TypeInstance* typeInstance);
			
			bool hasDefaultDestroy(SEM::TypeInstance* typeInstance);
			
			bool hasDefaultMove(SEM::TypeInstance* typeInstance);
			
			bool hasDefaultImplicitCopy(SEM::TypeInstance* typeInstance);
			
			bool hasDefaultExplicitCopy(SEM::TypeInstance* typeInstance);
			
			bool hasDefaultCompare(SEM::TypeInstance* typeInstance);
			
			bool hasDefaultSetDead(SEM::TypeInstance* typeInstance);
			
			bool hasDefaultIsLive(SEM::TypeInstance* typeInstance);
			
			void
			createDefaultConstructor(SEM::TypeInstance* typeInstance,
			                         SEM::Function* function,
			                         const Debug::SourceLocation& location);
			
			bool
			createDefaultImplicitCopy(SEM::TypeInstance* typeInstance,
			                          SEM::Function* function,
			                          const Debug::SourceLocation& location);
			
			bool
			createDefaultExplicitCopy(SEM::TypeInstance* typeInstance,
			                          SEM::Function* function,
			                          const Debug::SourceLocation& location);
			
			bool
			createDefaultCompare(SEM::TypeInstance* typeInstance,
			                     SEM::Function* function,
			                     const Debug::SourceLocation& location);
			
			bool
			createDefaultMethod(SEM::TypeInstance* typeInstance,
			                    SEM::Function* function,
			                    const Debug::SourceLocation& location);
			
		private:
			Context& context_;
			
		};
		
	}
	
}

#endif
