#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		size_t getRefCount(const SEM::Type* type) {
			size_t count = 0;
			while (type->isRef()) {
				type = type->refTarget();
				count++;
			}
			return count;
		}
		
		const SEM::Type* getLastRefType(const SEM::Type* type) {
			while (getRefCount(type) > 1) {
				type = type->refTarget();
			}
			return type;
		}
		
		const SEM::Type* getSingleDerefType(const SEM::Type* type) {
			return type->isRef() ? type->refTarget() : type;
		}
		
		const SEM::Type* getDerefType(const SEM::Type* type) {
			while (type->isRef()) {
				type = type->refTarget();
			}
			return type;
		}
		
		SEM::Value* derefOne(SEM::Value* value) {
			assert(value->type()->isRef() && value->type()->refTarget()->isRef());
			// TODO: add support for custom ref types.
			return SEM::Value::DerefReference(value);
		}
		
		SEM::Value* derefValue(SEM::Value* value) {
			while (value->type()->isRef() && value->type()->refTarget()->isRef()) {
				// TODO: add support for custom ref types.
				value = SEM::Value::DerefReference(value);
			}
			return value;
		}
		
		SEM::Value* derefAll(SEM::Value* value) {
			while (value->type()->isRef()) {
				// TODO: add support for custom ref types.
				value = SEM::Value::DerefReference(value);
			}
			return value;
		}
		
		size_t getStaticRefCount(const SEM::Type* type) {
			size_t count = 0;
			while (type->isStaticRef()) {
				type = type->staticRefTarget();
				count++;
			}
			return count;
		}
		
		const SEM::Type* getLastStaticRefType(const SEM::Type* type) {
			while (getStaticRefCount(type) > 1) {
				type = type->staticRefTarget();
			}
			return type;
		}
		
		const SEM::Type* getStaticDerefType(const SEM::Type* type) {
			while (type->isStaticRef()) {
				type = type->staticRefTarget();
			}
			return type;
		}
		
		SEM::Value* staticDerefOne(SEM::Value* value) {
			assert(value->type()->isStaticRef() && value->type()->staticRefTarget()->isStaticRef());
			// TODO: add support for custom ref types.
			return SEM::Value::DerefReference(value);
		}
		
		SEM::Value* staticDerefValue(SEM::Value* value) {
			while (value->type()->isStaticRef() && value->type()->staticRefTarget()->isStaticRef()) {
				// TODO: add support for custom ref types.
				value = SEM::Value::DerefReference(value);
			}
			return value;
		}
		
		SEM::Value* staticDerefAll(SEM::Value* value) {
			while (value->type()->isStaticRef()) {
				// TODO: add support for custom ref types.
				value = SEM::Value::DerefReference(value);
			}
			return value;
		}
		
		SEM::Value* createTypeRef(Context& context, const SEM::Type* targetType) {
			const auto typenameType = getBuiltInType(context.scopeStack(), "typename_t");
			return SEM::Value::TypeRef(targetType, typenameType->createStaticRefType(targetType));
		}
		
		const SEM::Type* createReferenceType(Context& context, const SEM::Type* varType) {
			const auto referenceTypeInst = getBuiltInType(context.scopeStack(), "__ref")->getObjectType();
			return SEM::Type::Object(referenceTypeInst, { varType})->createRefType(varType)->createConstType();
		}
		
		SEM::Value* createSelfRef(Context& context, const SEM::Type* selfType) {
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


