#ifndef LOCIC_SEMANTICANALYSIS_CONVERTEXCEPTION_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTEXCEPTION_HPP

#include <memory>

#include <locic/AST.hpp>

namespace locic {
	
	namespace SEM {
		
		class Function;
		class TypeInstance;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		std::unique_ptr<SEM::Function> CreateExceptionConstructorDecl(Context& context, SEM::TypeInstance* semTypeInstance);
		
		void CreateExceptionConstructor(Context& context, const AST::Node<AST::TypeInstance>& astTypeInstanceNode, SEM::TypeInstance* semTypeInstance, SEM::Function* function);
		
	}
	
}

#endif
