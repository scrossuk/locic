#ifndef LOCIC_SEMANTICANALYSIS_REF_HPP
#define LOCIC_SEMANTICANALYSIS_REF_HPP

#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		size_t getRefCount(const SEM::Type* type);
		
		const SEM::Type* getLastRefType(const SEM::Type* type);
		
		const SEM::Type* getSingleDerefType(const SEM::Type* type);
		
		const SEM::Type* getDerefType(const SEM::Type* type);
		
		SEM::Value derefOne(SEM::Value value);
		
		SEM::Value derefValue(SEM::Value value);
		
		SEM::Value derefAll(SEM::Value value);
		
		size_t getStaticRefCount(const SEM::Type* type);
		
		const SEM::Type* getLastStaticRefType(const SEM::Type* type);
		
		const SEM::Type* getStaticDerefType(const SEM::Type* type);
		
		SEM::Value staticDerefOne(SEM::Value value);
		
		SEM::Value staticDerefValue(SEM::Value value);
		
		SEM::Value staticDerefAll(SEM::Value value);
		
		SEM::Value createTypeRef(Context& context, const SEM::Type* targetType);
		
		const SEM::Type* createReferenceType(Context& context, const SEM::Type* varType);
		
		SEM::Value createSelfRef(Context& context, const SEM::Type* selfType);
		
		SEM::Value createLocalVarRef(Context& context, SEM::Var* var);
		
		SEM::Value createMemberVarRef(Context& context, SEM::Value object, SEM::Var* var);
		
	}
	
}

#endif
