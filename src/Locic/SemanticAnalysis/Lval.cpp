#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/Lval.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Type* makeValueLvalType(Context& context, bool isLvalMutable, SEM::Type* valueType) {
			SEM::TypeInstance* valueLvalTypeInstance = context.getBuiltInType("value_lval");
			SEM::Type* valueLvalType = SEM::Type::Object(isLvalMutable, valueLvalTypeInstance, std::vector<SEM::Type*>(1, valueType));
			return valueLvalType;
		}
		
		SEM::Type* makeLvalType(Context& context, bool isCustomLval, bool isLvalMutable, SEM::Type* valueType) {
			return isCustomLval ? valueType : makeValueLvalType(context, isLvalMutable, valueType);
		}
		
	}
	
}


