#include <vector>

#include <Locic/CodeGen/Module.hpp>
#include <Locic/CodeGen/Primitives.hpp>
#include <Locic/CodeGen/TypeSizeKnowledge.hpp>

namespace Locic {

	namespace CodeGen {
	
		bool isTypeSizeKnownInThisModule(Module& module, SEM::Type* unresolvedType) {
			SEM::Type* type = module.resolveType(unresolvedType);
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
					} else if (objectType->isStructDef()) {
						// Structs can only contain known size members.
						return true;
					} else if (objectType->isClassDef()) {
						// All members of the class must have a known size.
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

