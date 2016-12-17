#ifndef LOCIC_SEMANTICANALYSIS_CONVERTEXCEPTION_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTEXCEPTION_HPP

#include <memory>

#include <locic/AST.hpp>

namespace locic {
	
	namespace AST {
		
		class FunctionDecl;
		
	}
	
	namespace SEM {
		
		class TypeInstance;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		std::unique_ptr<AST::FunctionDecl>
		CreateExceptionConstructorDecl(Context& context, SEM::TypeInstance* semTypeInstance);
		
		void CreateExceptionConstructor(Context& context,
		                                const AST::Node<AST::TypeInstance>& astTypeInstanceNode,
		                                SEM::TypeInstance* semTypeInstance,
		                                AST::FunctionDecl& function);
		
	}
	
}

#endif
