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
		
		std::unique_ptr<SEM::Function>
		ConvertFunctionDecl(Context& context, AST::Node<AST::Function>& function,
		                    AST::ModuleScope moduleScope);
		
		void ConvertFunctionDeclType(Context& context, SEM::Function& function);
		
	}
	
}

#endif
