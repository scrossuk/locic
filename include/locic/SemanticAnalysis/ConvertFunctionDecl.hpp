#ifndef LOCIC_SEMANTICANALYSIS_CONVERTFUNCTIONDECL_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTFUNCTIONDECL_HPP

#include <memory>

#include <locic/AST/Node.hpp>

namespace locic {
	
	namespace AST {
		
		class Function;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		void
		ConvertFunctionDecl(Context& context,
		                    AST::Node<AST::Function>& function);
		
		void
		ConvertFunctionDeclType(Context& context,
		                        AST::Node<AST::Function>& function);
		
	}
	
}

#endif
