#include <locic/SemanticAnalysis/Ref.hpp>

#include <locic/AST/Type.hpp>
#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Var.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		size_t getRefCount(const AST::Type* type) {
			size_t count = 0;
			while (type->isRef()) {
				type = type->refTarget();
				count++;
			}
			return count;
		}
		
		const AST::Type* getLastRefType(const AST::Type* type) {
			while (getRefCount(type) > 1) {
				type = type->refTarget();
			}
			return type;
		}
		
		const AST::Type* getSingleDerefType(const AST::Type* type) {
			return type->isRef() ? type->refTarget() : type;
		}
		
		const AST::Type* getDerefType(const AST::Type* type) {
			while (type->isRef()) {
				type = type->refTarget();
			}
			return type;
		}
		
		AST::Value derefOne(AST::Value value) {
			assert(value.type()->isRef() &&
			       value.type()->refTarget()->isRef());
			// TODO: add support for custom ref types.
			return AST::Value::DerefReference(std::move(value));
		}
		
		AST::Value derefValue(AST::Value value) {
			while (value.type()->isRef() &&
			       value.type()->refTarget()->isRef()) {
				// TODO: add support for custom ref types.
				value = AST::Value::DerefReference(std::move(value));
			}
			return value;
		}
		
		AST::Value derefAll(AST::Value value) {
			while (value.type()->isRef()) {
				// TODO: add support for custom ref types.
				value = AST::Value::DerefReference(std::move(value));
			}
			return value;
		}
		
		AST::Value createTypeRef(Context& context, const AST::Type* targetType) {
			const auto typenameType = context.typeBuilder().getTypenameType(targetType);
			return AST::Value::TypeRef(targetType, typenameType);
		}
		
		const AST::Type* createReferenceType(Context& context, const AST::Type* const varType) {
			return getBuiltInType(context, context.getCString("ref_t"), { varType});
		}
		
		AST::Value bindReference(Context& context, AST::Value value) {
			const auto refType = createReferenceType(context, value.type());
			return AST::Value::BindReference(std::move(value), refType);
		}
		
		AST::Value derefOrBindValue(Context& context, AST::Value value,
		                            const size_t targetRefCount) {
			const auto actualRefCount = getRefCount(value.type());
			
			// Bind as many times as needed.
			for (size_t i = actualRefCount; i < targetRefCount; i++) {
				value = bindReference(context, std::move(value));
			}
			
			for (size_t i = actualRefCount; i > targetRefCount; i--) {
				value = derefValue(std::move(value));
			}
			
			return value;
		}
		
		AST::Value createSelfRef(Context& context, const AST::Type* const selfType) {
			return AST::Value::Self(createReferenceType(context, selfType));
		}
		
		AST::Value createLocalVarRef(Context& context, const AST::Var& var) {
			return AST::Value::LocalVar(var, createReferenceType(context, var.type()));
		}
		
		const AST::Type* getMemberType(const AST::Type* const objectType, const AST::Var& var,
		                               const bool isInternalAccess) {
			const auto derefType = getDerefType(objectType);
			const auto varType = var.type();
			
			// If the access is internal (i.e. '@member') then we want
			// to keep the selfconst, whereas external accesses will
			// need to substitute selfconst.
			const auto substitutedVarType =
				varType->substitute(derefType->generateTemplateVarMap(),
				                    /*selfconst=*/isInternalAccess ?
				                        AST::Predicate::SelfConst() :
				                        derefType->constPredicate().copy());
			
			if (var.isOverrideConst()) {
				// Don't apply const predicate to variables marked
				// 'override_const'.
				return substitutedVarType;
			}
			
			auto predicate = derefType->constPredicate().copy();
			if (isInternalAccess) {
				// Create (const_predicate AND selfconst).
				// 
				// It is actually true that selfconst => const_predicate, so
				// we could encode that knowledge into the compiler and return
				// selfconst(T) here.
				// 
				// However it's easier to use an AND expression, which allows
				// casts to both selfconst(T)& and const_predicate(T)&.
				predicate = AST::Predicate::And(std::move(predicate),
				                                AST::Predicate::SelfConst());
			}
			return substitutedVarType->applyConst(std::move(predicate));
		}
		
		AST::Value createMemberVarRef(Context& context, AST::Value object, const AST::Var& var,
		                              const bool isInternalAccess) {
			const auto memberType = getMemberType(object.type(), var, isInternalAccess);
			const auto memberRefType = createReferenceType(context, memberType);
			return AST::Value::MemberAccess(derefOrBindValue(context, std::move(object)), var,
			                                memberRefType);
		}
		
	}
	
}


