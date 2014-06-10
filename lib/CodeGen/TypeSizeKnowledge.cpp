#include <vector>

#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
	
		bool isObjectTypeSizeKnownInThisModule(Module& module, SEM::TypeInstance* objectType) {
			if (objectType->isStruct()) {
				// Structs can only contain known size members.
				return true;
			} else if (objectType->isClassDef() || objectType->isDatatype() || objectType->isException()) {
				// All members of the type must have a known size
				// for it to have a known size.
				for (auto var: objectType->variables()) {
					if (!isTypeSizeKnownInThisModule(module, var->type())) {
						return false;
					}
				}
				return true;
			} else if (objectType->isUnionDatatype()) {
				for (auto variantTypeInstance: objectType->variants()) {
					if (!isObjectTypeSizeKnownInThisModule(module, variantTypeInstance)) {
						return false;
					}
				}
				return true;
			} else {
				return false;
			}
		}
		
		bool isTypeSizeKnownInThisModule(Module& module, SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
					return true;
				case SEM::Type::OBJECT: {
					const auto objectType = type->getObjectType();
					if (objectType->isPrimitive()) {
						return isPrimitiveTypeSizeKnownInThisModule(module, type);
					} else {
						return isObjectTypeSizeKnownInThisModule(module, objectType);
					}
				}
				case SEM::Type::TEMPLATEVAR:
					return false;
				default:
					llvm_unreachable("Unknown SEM type kind enum.");
			}
		}
		
		bool isTypeSizeAlwaysKnown(Module& module, SEM::Type* type) {
			switch (type->kind()) {
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
					return true;
				case SEM::Type::OBJECT:
					if (type->isPrimitive()) {
						return isPrimitiveTypeSizeAlwaysKnown(module, type);
					} else if (type->isStruct()) {
						// Structs must always have a known size.
						return true;
					} else {
						// TODO: datatypes containing known size types should
						//       also have a known size.
						return false;
					}
				case SEM::Type::TEMPLATEVAR:
					return false;
				default:
					llvm_unreachable("Unknown SEM type kind enum.");
			}
		}
		
	}
	
}

