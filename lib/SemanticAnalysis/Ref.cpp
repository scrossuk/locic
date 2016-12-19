#include <locic/AST/Type.hpp>
#include <locic/AST/ValueDecl.hpp>
#include <locic/AST/Var.hpp>
#include <locic/SEM.hpp>

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
			assert(value.type()->isRef() && value.type()->refTarget()->isRef());
			// TODO: add support for custom ref types.
			return AST::Value::DerefReference(std::move(value));
		}
		
		AST::Value derefValue(AST::Value value) {
			while (value.type()->isRef() && value.type()->refTarget()->isRef()) {
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
		
		size_t getStaticRefCount(const AST::Type* type) {
			size_t count = 0;
			while (type->isStaticRef()) {
				type = type->staticRefTarget();
				count++;
			}
			return count;
		}
		
		const AST::Type* getLastStaticRefType(const AST::Type* type) {
			while (getStaticRefCount(type) > 1) {
				type = type->staticRefTarget();
			}
			return type;
		}
		
		const AST::Type* getStaticDerefType(const AST::Type* type) {
			while (type->isStaticRef()) {
				type = type->staticRefTarget();
			}
			return type;
		}
		
		AST::Value staticDerefOne(AST::Value value) {
			assert(value.type()->isStaticRef() && value.type()->staticRefTarget()->isStaticRef());
			// TODO: add support for custom ref types.
			return AST::Value::DerefReference(std::move(value));
		}
		
		AST::Value staticDerefValue(AST::Value value) {
			while (value.type()->isStaticRef() && value.type()->staticRefTarget()->isStaticRef()) {
				// TODO: add support for custom ref types.
				value = AST::Value::DerefReference(std::move(value));
			}
			return value;
		}
		
		AST::Value staticDerefAll(AST::Value value) {
			while (value.type()->isStaticRef()) {
				// TODO: add support for custom ref types.
				value = AST::Value::DerefReference(std::move(value));
			}
			return value;
		}
		
		AST::Value createTypeRef(Context& context, const AST::Type* targetType) {
			const auto typenameType = context.typeBuilder().getTypenameType();
			return AST::Value::TypeRef(targetType, typenameType->createStaticRefType(targetType));
		}
		
		const AST::Type* createReferenceType(Context& context, const AST::Type* const varType) {
			return getBuiltInType(context, context.getCString("ref_t"), { varType})->createRefType(varType);
		}
		
		AST::Value bindReference(Context& context, AST::Value value) {
			const auto refType = createReferenceType(context, value.type());
			return AST::Value::BindReference(std::move(value), refType);
		}
		
		AST::Value derefOrBindValue(Context& context, AST::Value value) {
			if (value.type()->isRef()) {
				return derefValue(std::move(value));
			} else {
				return bindReference(context, std::move(value));
			}
		}
		
		AST::Value createSelfRef(Context& context, const AST::Type* const selfType) {
			return AST::Value::Self(createReferenceType(context, selfType));
		}
		
		AST::Value createLocalVarRef(Context& context, const AST::Var& var) {
			return AST::Value::LocalVar(var, createReferenceType(context, var.lvalType()));
		}
		
		AST::Value createMemberVarRef(Context& context, AST::Value object, const AST::Var& var) {
			// If the object type is const, then the members must
			// also be, *UNLESS* the variable is marked '__override_const'.
			const auto derefType = getDerefType(object.type());
			const auto memberType = var.lvalType()->createTransitiveConstType(derefType->constPredicate().copy());
			const auto memberTypeSub = memberType->substitute(derefType->generateTemplateVarMap());
			const auto resultMemberType = var.isOverrideConst() ? memberTypeSub->withoutConst() : memberTypeSub;
			return AST::Value::MemberAccess(derefOrBindValue(context, std::move(object)), var, createReferenceType(context, resultMemberType));
		}
		
	}
	
}


