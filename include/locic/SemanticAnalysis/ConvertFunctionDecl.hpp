#ifndef LOCIC_SEMANTICANALYSIS_CONVERTFUNCTIONDECL_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTFUNCTIONDECL_HPP

#include <memory>

#include <locic/AST.hpp>

namespace locic {
	
	namespace SEM {
		
		class Function;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		void
		ConvertFunctionDecl(Context& context,
		                    AST::Node<AST::FunctionDecl>& function);
		
		void
		ConvertFunctionDeclType(Context& context,
		                        AST::Node<AST::FunctionDecl>& function);
		
	}
	
}

#endif
