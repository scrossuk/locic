#ifndef LOCIC_SEMANTICANALYSIS_CONVERTVAR_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTVAR_HPP

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		Debug::VarInfo makeVarInfo(Debug::VarInfo::Kind kind, const AST::Node<AST::TypeVar>& astTypeVarNode);
		
		void attachVar(Context& context, const std::string& name, const AST::Node<AST::TypeVar>& astTypeVarNode, SEM::Var* var);
		
		SEM::Var* ConvertVar(Context& context, bool isMember, const AST::Node<AST::TypeVar>& typeVar);
		
		SEM::Var* ConvertInitialisedVar(Context& context, bool isMember, const AST::Node<AST::TypeVar>& typeVar, SEM::Type* initialiseType);
		
	}
	
}

#endif
