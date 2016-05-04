#ifndef LOCIC_SEMANTICANALYSIS_CONVERTVAR_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTVAR_HPP

#include <locic/AST.hpp>
#include <locic/Debug/VarInfo.hpp>

namespace locic {
	
	namespace SEM {
		
		class Type;
		class Var;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		Debug::VarInfo makeVarInfo(Debug::VarInfo::Kind kind, const AST::Node<AST::Var>& astVarNode);
		
		void attachVar(Context& context, const String& name, const AST::Node<AST::Var>& astVarNode, SEM::Var& var, Debug::VarInfo::Kind varKind);
		
		const SEM::Type* getVarType(Context& context, const AST::Node<AST::Var>& astVarNode, const SEM::Type* initialiseType);
		
		std::unique_ptr<SEM::Var>
		ConvertVar(Context& context, Debug::VarInfo::Kind varKind,
		           AST::Node<AST::Var>& typeVar);
		
		// Note that this function assumes that the variable is a local variable.
		std::unique_ptr<SEM::Var>
		ConvertInitialisedVar(Context& context, AST::Node<AST::Var>& typeVar,
		                      const SEM::Type* initialiseType);
		
	}
	
}

#endif
