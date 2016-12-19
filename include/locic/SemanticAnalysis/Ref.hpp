#ifndef LOCIC_SEMANTICANALYSIS_REF_HPP
#define LOCIC_SEMANTICANALYSIS_REF_HPP

#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		size_t getRefCount(const AST::Type* type);
		
		const AST::Type* getLastRefType(const AST::Type* type);
		
		const AST::Type* getSingleDerefType(const AST::Type* type);
		
		const AST::Type* getDerefType(const AST::Type* type);
		
		SEM::Value derefOne(SEM::Value value);
		
		SEM::Value derefValue(SEM::Value value);
		
		SEM::Value derefAll(SEM::Value value);
		
		size_t getStaticRefCount(const AST::Type* type);
		
		const AST::Type* getLastStaticRefType(const AST::Type* type);
		
		const AST::Type* getStaticDerefType(const AST::Type* type);
		
		SEM::Value staticDerefOne(SEM::Value value);
		
		SEM::Value staticDerefValue(SEM::Value value);
		
		SEM::Value staticDerefAll(SEM::Value value);
		
		SEM::Value createTypeRef(Context& context, const AST::Type* targetType);
		
		const AST::Type* createReferenceType(Context& context, const AST::Type* varType);
		
		SEM::Value bindReference(Context& context, SEM::Value value);
		
		SEM::Value derefOrBindValue(Context& context, SEM::Value value);
		
		SEM::Value createSelfRef(Context& context, const AST::Type* selfType);
		
		SEM::Value createLocalVarRef(Context& context, const AST::Var& var);
		
		SEM::Value createMemberVarRef(Context& context, SEM::Value object, const AST::Var& var);
		
	}
	
}

#endif
