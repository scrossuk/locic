#ifndef LOCIC_SEMANTICANALYSIS_CONVERTVAR_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTVAR_HPP

#include <locic/AST.hpp>
#include <locic/Debug/VarInfo.hpp>

namespace locic {
	
	namespace SEM {
		
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		Debug::VarInfo makeVarInfo(Debug::VarInfo::Kind kind, const AST::Node<AST::Var>& astVarNode);
		
		const SEM::Type* getVarType(Context& context, const AST::Node<AST::Var>& astVarNode, const SEM::Type* initialiseType);
		
		AST::Var*
		ConvertVar(Context& context, Debug::VarInfo::Kind varKind,
		           AST::Node<AST::Var>& typeVar);
		
		// Note that this function assumes that the variable is a local variable.
		AST::Var*
		ConvertInitialisedVar(Context& context, AST::Node<AST::Var>& typeVar,
		                      const SEM::Type* initialiseType);
		
	}
	
}

#endif
