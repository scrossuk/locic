#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		size_t getRefCount(SEM::Type* type) {
			size_t count = 0;
			while (type->isRef() || type->isStaticRef()) {
				type = type->isRef() ?
					type->refTarget() :
					type->staticRefTarget();
				count++;
			}
			return count;
		}
		
		SEM::Type* getLastRefType(SEM::Type* type) {
			while (getRefCount(type) > 1) {
				type = type->isRef() ?
					type->refTarget() :
					type->staticRefTarget();
			}
			return type;
		}
		
		SEM::Type* getDerefType(SEM::Type* type) {
			while (type->isRef() || type->isStaticRef()) {
				type = type->isRef() ?
					type->refTarget() :
					type->staticRefTarget();
			}
			return type;
		}
		
		SEM::Value* derefOne(SEM::Value* value) {
			assert(getRefCount(value->type()) > 1);
			// TODO: add support for custom ref types.
			return SEM::Value::DerefReference(value);
		}
		
		SEM::Value* derefValue(SEM::Value* value) {
			while (getRefCount(value->type()) > 1) {
				// TODO: add support for custom ref types.
				value = SEM::Value::DerefReference(value);
			}
			return value;
		}
		
		SEM::Value* derefAll(SEM::Value* value) {
			while (getRefCount(value->type()) > 0) {
				// TODO: add support for custom ref types.
				value = SEM::Value::DerefReference(value);
			}
			return value;
		}
		
		SEM::Type* createReferenceType(Context& context, SEM::Type* varType) {
			const auto referenceTypeInst = getBuiltInType(context.scopeStack(), "__ref");
			return SEM::Type::Object(referenceTypeInst, { varType})->createRefType(varType)->createConstType();
		}
		
		SEM::Value* createSelfRef(Context& context, SEM::Type* selfType) {
			return SEM::Value::Self(createReferenceType(context, selfType));
		}
		
		SEM::Value* createLocalVarRef(Context& context, SEM::Var* var) {
			return SEM::Value::LocalVar(var, createReferenceType(context, var->type()));
		}
		
		SEM::Value* createMemberVarRef(Context& context, SEM::Value* object, SEM::Var* var) {
			// If the object type is const, then
			// the members must also be.
			const auto derefType = getDerefType(object->type());
			const auto memberType = derefType->isConst() ? var->type()->createConstType() : var->type();
			const auto memberTypeSub = memberType->substitute(derefType->generateTemplateVarMap());
			return SEM::Value::MemberAccess(derefValue(object), var, createReferenceType(context, memberTypeSub));
		}
		
	}
	
}


