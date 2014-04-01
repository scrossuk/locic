#ifndef LOCIC_SEMANTICANALYSIS_DEFAULTMETHODS_HPP
#define LOCIC_SEMANTICANALYSIS_DEFAULTMETHODS_HPP

#include <locic/Name.hpp>
#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Function* CreateDefaultConstructorDecl(Context& context, SEM::TypeInstance* typeInstance);
		
		SEM::Function* CreateDefaultImplicitCopyDecl(SEM::TypeInstance* typeInstance);
		
		SEM::Function* CreateDefaultCompareDecl(Context& context, SEM::TypeInstance* typeInstance);
		
		SEM::Function* CreateDefaultMethodDecl(Context& context, SEM::TypeInstance* typeInstance, bool isStatic, const Name& name, const Debug::SourceLocation& location);
		
		bool HasDefaultImplicitCopy(SEM::TypeInstance* typeInstance);
		
		bool HasDefaultCompare(SEM::TypeInstance* typeInstance);
		
		void CreateDefaultConstructor(SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location);
		
		void CreateDefaultImplicitCopy(SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location);
		
		void CreateDefaultCompare(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location);
		
		void CreateDefaultMethod(Context& context, SEM::TypeInstance* typeInstance, SEM::Function* function, const Debug::SourceLocation& location);
		
	}
	
}

#endif
