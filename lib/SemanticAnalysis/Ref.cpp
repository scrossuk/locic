#include <locic/SEM.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Type* getDerefType(SEM::Type* type) {
			while (type->isRef()) {
				type = type->refTarget();
			}
			return type;
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


