#ifndef LOCIC_SEMANTICANALYSIS_CONVERTEXCEPTION_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTEXCEPTION_HPP

#include <memory>

#include <locic/AST.hpp>

namespace locic {
	
	namespace AST {
		
		class Function;
		class TypeInstance;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		std::unique_ptr<AST::Function>
		CreateExceptionConstructorDecl(Context& context,
		                               AST::TypeInstance& typeInstance);
		
		void CreateExceptionConstructor(Context& context,
		                                AST::Node<AST::TypeInstance>& typeInstanceNode,
		                                AST::Function& function);
		
	}
	
}

#endif
