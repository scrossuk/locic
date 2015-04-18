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
		
		std::unique_ptr<SEM::Function> CreateDefaultConstructorDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name);
		
		std::unique_ptr<SEM::Function> CreateDefaultMoveDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name);
		
		std::unique_ptr<SEM::Function> CreateDefaultImplicitCopyDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name);
		
		std::unique_ptr<SEM::Function> CreateDefaultExplicitCopyDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name);
		
		std::unique_ptr<SEM::Function> CreateDefaultCompareDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name);
		
		std::unique_ptr<SEM::Function> CreateDefaultDeadDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name);
		
		std::unique_ptr<SEM::Function> CreateDefaultIsLiveDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name);
		
		std::unique_ptr<SEM::Function> CreateDefaultMethodDecl(Context& context, SEM::TypeInstance* typeInstance, bool isStatic, const Name& name, const Debug::SourceLocation& location);
		
		bool HasDefaultConstructor(Context& context, SEM::TypeInstance* typeInstance);
		
		bool HasDefaultMove(Context& context, SEM::TypeInstance* typeInstance);
		
		bool HasDefaultImplicitCopy(Context& context, SEM::TypeInstance* typeInstance);
		
		bool HasDefaultExplicitCopy(Context& context, SEM::TypeInstance* typeInstance);
		
		bool HasDefaultCompare(Context& context, SEM::TypeInstance* typeInstance);
		
		bool HasDefaultDead(Context& context, SEM::TypeInstance* typeInstance);
		
		bool HasDefaultIsLive(Context& context, SEM::TypeInstance* typeInstance);
		
		void CreateDefaultConstructor(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location);
		
		bool CreateDefaultMethod(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location);
		
	}
	
}

#endif
