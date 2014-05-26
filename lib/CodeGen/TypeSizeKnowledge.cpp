#include <vector>

#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
	
		namespace {
			
			bool isObjectTypeSizeKnownInThisModule(Module& module, SEM::TypeInstance* objectType, const TemplateVarMap& templateVarMap) {
				if (objectType->isStruct()) {
					// Structs can only contain known size members.
					return true;
				} else if (objectType->isClassDef() || objectType->isDatatype() || objectType->isException()) {
					TemplateVarMapStackEntry templateVarMapStackEntry(module, templateVarMap);
					
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
						if (!isObjectTypeSizeKnownInThisModule(module, variantTypeInstance, templateVarMap)) {
							return false;
						}
					}
					return true;
				} else {
					return false;
				}
			}
			
		}
		
		bool isTypeSizeKnownInThisModule(Module& module, SEM::Type* unresolvedType) {
			auto type = module.resolveType(unresolvedType);
			switch (type->kind()) {
				case SEM::Type::VOID:
				case SEM::Type::REFERENCE:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
					return true;
				case SEM::Type::OBJECT:
				{
					auto objectType = type->getObjectType();
					if (objectType->isPrimitive()) {
						// Not all primitives have a known size (e.g. value_lval).
						return isPrimitiveTypeSizeKnownInThisModule(module, type);
					} else {
						return isObjectTypeSizeKnownInThisModule(module, objectType, type->generateTemplateVarMap());
					}
				}
				case SEM::Type::TEMPLATEVAR:
					return false;
				default:
					assert(false && "Unknown SEM type kind enum");
					return false;
			}
		}
		
		bool isTypeSizeAlwaysKnown(Module& module, SEM::Type* unresolvedType) {
			SEM::Type* type = module.resolveType(unresolvedType);
			switch (type->kind()) {
				case SEM::Type::VOID:
				case SEM::Type::REFERENCE:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
					return true;
				case SEM::Type::OBJECT:
					if (type->isPrimitive()) {
						// Not all primitives have a known size (e.g. value_lval).
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
					llvm_unreachable("Unknown SEM type kind enum");
			}
		}
		
	}
	
}

