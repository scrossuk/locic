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
		
		AST::Value derefOne(AST::Value value);
		
		AST::Value derefValue(AST::Value value);
		
		AST::Value derefAll(AST::Value value);
		
		size_t getStaticRefCount(const AST::Type* type);
		
		const AST::Type* getLastStaticRefType(const AST::Type* type);
		
		const AST::Type* getStaticDerefType(const AST::Type* type);
		
		AST::Value staticDerefOne(AST::Value value);
		
		AST::Value staticDerefValue(AST::Value value);
		
		AST::Value staticDerefAll(AST::Value value);
		
		AST::Value createTypeRef(Context& context, const AST::Type* targetType);
		
		const AST::Type* createReferenceType(Context& context, const AST::Type* varType);
		
		AST::Value bindReference(Context& context, AST::Value value);
		
		AST::Value derefOrBindValue(Context& context, AST::Value value);
		
		AST::Value createSelfRef(Context& context, const AST::Type* selfType);
		
		AST::Value createLocalVarRef(Context& context, const AST::Var& var);
		
		AST::Value createMemberVarRef(Context& context, AST::Value object, const AST::Var& var);
		
	}
	
}

#endif
