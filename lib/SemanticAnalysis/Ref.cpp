#include <locic/SEM.hpp>

#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ScopeElement.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>
#include <locic/SemanticAnalysis/TypeBuilder.hpp>

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
		
		SEM::Value derefOne(SEM::Value value) {
			assert(value.type()->isRef() && value.type()->refTarget()->isRef());
			// TODO: add support for custom ref types.
			return SEM::Value::DerefReference(std::move(value));
		}
		
		SEM::Value derefValue(SEM::Value value) {
			while (value.type()->isRef() && value.type()->refTarget()->isRef()) {
				// TODO: add support for custom ref types.
				value = SEM::Value::DerefReference(std::move(value));
			}
			return value;
		}
		
		SEM::Value derefAll(SEM::Value value) {
			while (value.type()->isRef()) {
				// TODO: add support for custom ref types.
				value = SEM::Value::DerefReference(std::move(value));
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
		
		SEM::Value staticDerefOne(SEM::Value value) {
			assert(value.type()->isStaticRef() && value.type()->staticRefTarget()->isStaticRef());
			// TODO: add support for custom ref types.
			return SEM::Value::DerefReference(std::move(value));
		}
		
		SEM::Value staticDerefValue(SEM::Value value) {
			while (value.type()->isStaticRef() && value.type()->staticRefTarget()->isStaticRef()) {
				// TODO: add support for custom ref types.
				value = SEM::Value::DerefReference(std::move(value));
			}
			return value;
		}
		
		SEM::Value staticDerefAll(SEM::Value value) {
			while (value.type()->isStaticRef()) {
				// TODO: add support for custom ref types.
				value = SEM::Value::DerefReference(std::move(value));
			}
			return value;
		}
		
		SEM::Value createTypeRef(Context& context, const SEM::Type* targetType) {
			const auto typenameType = context.typeBuilder().getTypenameType();
			return SEM::Value::TypeRef(targetType, typenameType->createStaticRefType(targetType));
		}
		
		const SEM::Type* createReferenceType(Context& context, const SEM::Type* const varType) {
			return getBuiltInType(context, context.getCString("ref_t"), { varType})->createRefType(varType);
		}
		
		SEM::Value bindReference(Context& context, SEM::Value value) {
			const auto refType = createReferenceType(context, value.type());
			return SEM::Value::BindReference(std::move(value), refType);
		}
		
		SEM::Value derefOrBindValue(Context& context, SEM::Value value) {
			if (value.type()->isRef()) {
				return derefValue(std::move(value));
			} else {
				return bindReference(context, std::move(value));
			}
		}
		
		SEM::Value createSelfRef(Context& context, const SEM::Type* const selfType) {
			return SEM::Value::Self(createReferenceType(context, selfType));
		}
		
		SEM::Value createLocalVarRef(Context& context, const SEM::Var& var) {
			return SEM::Value::LocalVar(var, createReferenceType(context, var.type()));
		}
		
		SEM::Value createMemberVarRef(Context& context, SEM::Value object, const SEM::Var& var) {
			// If the object type is const, then the members must
			// also be, *UNLESS* the variable is marked '__override_const'.
			const auto derefType = getDerefType(object.type());
			const auto memberType = var.type()->createTransitiveConstType(derefType->constPredicate().copy());
			const auto memberTypeSub = memberType->substitute(derefType->generateTemplateVarMap());
			const auto resultMemberType = var.isOverrideConst() ? memberTypeSub->withoutConst() : memberTypeSub;
			return SEM::Value::MemberAccess(derefOrBindValue(context, std::move(object)), var, createReferenceType(context, resultMemberType));
		}
		
	}
	
}


