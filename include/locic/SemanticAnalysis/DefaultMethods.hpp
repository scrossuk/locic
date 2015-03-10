#ifndef LOCIC_SEMANTICANALYSIS_DEFAULTMETHODS_HPP
#define LOCIC_SEMANTICANALYSIS_DEFAULTMETHODS_HPP

#include <locic/Support/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Function* CreateDefaultConstructorDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name);
		
		SEM::Function* CreateDefaultMoveDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name);
		
		SEM::Function* CreateDefaultImplicitCopyDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name);
		
		SEM::Function* CreateDefaultExplicitCopyDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name);
		
		SEM::Function* CreateDefaultCompareDecl(Context& context, SEM::TypeInstance* typeInstance, const Name& name);
		
		SEM::Function* CreateDefaultMethodDecl(Context& context, SEM::TypeInstance* typeInstance, bool isStatic, const Name& name, const Debug::SourceLocation& location);
		
		bool HasDefaultConstructor(Context&, SEM::TypeInstance* typeInstance);
		
		bool HasDefaultMove(Context&, SEM::TypeInstance* typeInstance);
		
		bool HasDefaultImplicitCopy(Context& context, SEM::TypeInstance* typeInstance);
		
		bool HasDefaultExplicitCopy(Context& context, SEM::TypeInstance* typeInstance);
		
		bool HasDefaultCompare(Context& context, SEM::TypeInstance* typeInstance);
		
		void CreateDefaultConstructor(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location);
		
		void CreateDefaultImplicitCopy(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location);
		
		void CreateDefaultExplicitCopy(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location);
		
		void CreateDefaultCompare(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location);
		
		void CreateDefaultMethod(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location);
		
	}
	
}

#endif
