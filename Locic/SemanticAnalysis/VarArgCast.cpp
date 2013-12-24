#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Exception.hpp>
#include <Locic/SemanticAnalysis/Lval.hpp>
#include <Locic/SemanticAnalysis/TypeProperties.hpp>
#include <Locic/SemanticAnalysis/VarArgCast.hpp>

namespace Locic {

	namespace SemanticAnalysis {
		
		bool isValidVarArgType(SEM::Type* type) {
			if (!type->isObject()) return false;
			if (!type->getObjectType()->isPrimitive()) return false;
			
			const std::string name = type->getObjectType()->name().first();
			
			// TODO: find a better (cleaner) way to do this...
			if (name == "char") return true;
			if (name == "short") return true;
			if (name == "int") return true;
			if (name == "long") return true;
			if (name == "float") return true;
			if (name == "double") return true;
			if (name == "ptr") return true;
			
			return false;
		}
		
		static inline SEM::Value* VarArgCastCheckType(SEM::Value* value) {
			if (!isValidVarArgType(value->type())) {
				throw TodoException(makeString("Var arg parameter '%s' has invalid type '%s'.",
					value->toString().c_str(), value->type()->toString().c_str()));
			}
			
			return value;
		}
		
		static inline SEM::Value* VarArgCastRefImplicitCopy(SEM::Value* value) {
			try {
				return VarArgCastCheckType(value);
			} catch(const Exception& e) {
				// Didn't work; try using dereference with implicit copy if possible.
				SEM::Type* type = value->type();
				if (!type->isReference()) {
					throw;
				}
				
				if (!type->getReferenceTarget()->supportsImplicitCopy()) {
					throw;
				}
				
				SEM::Value* derefValue = SEM::Value::DerefReference(value);
				SEM::Type* derefType = derefValue->type();
				
				SEM::Value* copyValue = derefType->isObject() ?
					CallProperty(derefValue, "implicitCopy", std::vector<SEM::Value*>()) :
					SEM::Value::CopyValue(derefValue);
				
				return VarArgCastCheckType(copyValue);
			}
		}
		
		static inline SEM::Value* VarArgCastImplicitCopy(SEM::Value* value) {
			try {
				return VarArgCastRefImplicitCopy(value);
			} catch(const Exception& e) {
				// Didn't work; try using implicit copy if possible.
				SEM::Type* type = value->type();
				if (!type->supportsImplicitCopy()) {
					throw;
				}
				
				SEM::Value* copyValue = type->isObject() ?
					CallProperty(value, "implicitCopy", std::vector<SEM::Value*>()) :
					SEM::Value::CopyValue(value);
				
				return VarArgCastRefImplicitCopy(copyValue);
			}
		}
		
		static inline SEM::Value* VarArgCastOpDissolve(SEM::Value* value) {
			try {
				return VarArgCastImplicitCopy(value);
			} catch(const Exception& e) {
				// Didn't work; try using dissolve if possible.
				if (!canDissolveValue(value)) {
					throw;
				}
				
				return VarArgCastImplicitCopy(dissolveLval(value));
			}
		}
		
		SEM::Value* VarArgCast(SEM::Value* value) {
			return VarArgCastOpDissolve(value);
		}
		
	}
	
}


