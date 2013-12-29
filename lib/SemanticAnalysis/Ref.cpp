#include <locic/SEM.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		size_t getRefCount(SEM::Type* type) {
			size_t count = 0;
			while (type->isRef()) {
				type = type->refTarget();
				count++;
			}
			return count;
		}
		
		SEM::Type* getDerefType(SEM::Type* type) {
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
		
	}
	
}


