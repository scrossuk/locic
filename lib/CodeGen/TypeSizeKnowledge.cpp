#include <vector>

#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitives.hpp>
#include <locic/CodeGen/TypeSizeKnowledge.hpp>

namespace locic {

	namespace CodeGen {
	
		bool isTypeSizeKnownInThisModule(Module& module, SEM::Type* unresolvedType) {
			auto type = module.resolveType(unresolvedType);
			switch (type->kind()) {
				case SEM::Type::VOID:
				case SEM::Type::NULLT:
				case SEM::Type::REFERENCE:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
					return true;
				case SEM::Type::OBJECT:
				{
					SEM::TypeInstance* objectType = type->getObjectType();
					if (objectType->isPrimitive()) {
						// Not all primitives have a known size (e.g. value_lval).
						return isPrimitiveTypeSizeKnownInThisModule(module, type);
					} else if (objectType->isStruct()) {
						// Structs can only contain known size members.
						return true;
					} else if (objectType->isClassDef() || objectType->isDatatype()) {
						// All members of the type must have a known size
						// for it to have a known size.
						const Module::TemplateVarMap templateVarMap = type->generateTemplateVarMap();
						TemplateVarMapStackEntry templateVarMapStackEntry(module, templateVarMap);
						for (size_t i = 0; i < objectType->variables().size(); i++) {
							SEM::Type* varType = objectType->variables().at(i)->type();
							if (!isTypeSizeKnownInThisModule(module, varType)) {
								return false;
							}
						}
						return true;
					} else {
						// TODO: union datatypes.
						return false;
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
				case SEM::Type::NULLT:
				case SEM::Type::REFERENCE:
				case SEM::Type::FUNCTION:
				case SEM::Type::METHOD:
					return true;
				case SEM::Type::OBJECT:
					// Not all primitives have a known size (e.g. value_lval).
					return type->isPrimitive() && isPrimitiveTypeSizeAlwaysKnown(module, type);
				case SEM::Type::TEMPLATEVAR:
					return false;
				default:
					assert(false && "Unknown SEM type kind enum");
					return false;
			}
		}
		
	}
	
}

