#ifndef LOCIC_SEMANTICANALYSIS_REF_HPP
#define LOCIC_SEMANTICANALYSIS_REF_HPP

#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		size_t getRefCount(SEM::Type* type);
		
		SEM::Type* getLastRefType(SEM::Type* type);
		
		SEM::Type* getSingleDerefType(SEM::Type* type);
		
		SEM::Type* getDerefType(SEM::Type* type);
		
		SEM::Value* derefOne(SEM::Value* value);
		
		SEM::Value* derefValue(SEM::Value* value);
		
		SEM::Value* derefAll(SEM::Value* value);
		
		size_t getStaticRefCount(SEM::Type* type);
		
		SEM::Type* getLastStaticRefType(SEM::Type* type);
		
		SEM::Type* getStaticDerefType(SEM::Type* type);
		
		SEM::Value* staticDerefOne(SEM::Value* value);
		
		SEM::Value* staticDerefValue(SEM::Value* value);
		
		SEM::Value* staticDerefAll(SEM::Value* value);
		
		SEM::Value* createTypeRef(Context& context, SEM::Type* targetType);
		
		SEM::Type* createReferenceType(Context& context, SEM::Type* varType);
		
		SEM::Value* createSelfRef(Context& context, SEM::Type* selfType);
		
		SEM::Value* createLocalVarRef(Context& context, SEM::Var* var);
		
		SEM::Value* createMemberVarRef(Context& context, SEM::Value* object, SEM::Var* var);
		
	}
	
}

#endif
