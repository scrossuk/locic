#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/SEM/Function.hpp>
#include <locic/SEM/Type.hpp>
#include <locic/SEM/TypeInstance.hpp>
#include <locic/SEM/Var.hpp>

namespace locic {
	
	namespace CodeGen {
		
		TypeInfo::TypeInfo(Module& module)
		: module_(module) { }
		
		bool TypeInfo::isSizeAlwaysKnown(const SEM::Type* const type) const {
			switch (type->kind()) {
				case SEM::Type::OBJECT:
					if (type->isPrimitive()) {
						const auto& primitive = module_.getPrimitive(*(type->getObjectType()));
						return primitive.isSizeAlwaysKnown(*this,
						                                   arrayRef(type->templateArguments()));
					} else {
						return isObjectSizeAlwaysKnown(*(type->getObjectType()));
					}
				case SEM::Type::TEMPLATEVAR:
					return false;
				case SEM::Type::ALIAS:
					return isSizeAlwaysKnown(type->resolveAliases());
				default:
					llvm_unreachable("Unknown SEM type kind enum.");
			}
		}
		
		bool TypeInfo::isObjectSizeAlwaysKnown(const SEM::TypeInstance& typeInstance) const {
			if (typeInstance.isEnum() || typeInstance.isStruct() || typeInstance.isUnion()) {
				// C types can only contain known size members.
				return true;
			} else if (typeInstance.isDatatype() || typeInstance.isException()) {
				// All members of the type must have a known size
				// for it to have a known size.
				for (auto var: typeInstance.variables()) {
					if (!isSizeAlwaysKnown(var->type())) {
						return false;
					}
				}
				return true;
			} else if (typeInstance.isUnionDatatype()) {
				for (auto variantTypeInstance: typeInstance.variants()) {
					if (!isObjectSizeAlwaysKnown(*variantTypeInstance)) {
						return false;
					}
				}
				return true;
			} else {
				return false;
			}
		}
		
		bool TypeInfo::isSizeKnownInThisModule(const SEM::Type* const type) const {
			switch (type->kind()) {
				case SEM::Type::OBJECT: {
					if (type->isPrimitive()) {
						const auto& primitive = module_.getPrimitive(*(type->getObjectType()));
						return primitive.isSizeKnownInThisModule(*this,
						                                         arrayRef(type->templateArguments()));
					} else {
						return isObjectSizeKnownInThisModule(*(type->getObjectType()));
					}
				}
				case SEM::Type::TEMPLATEVAR:
					return false;
				case SEM::Type::ALIAS:
					return isSizeKnownInThisModule(type->resolveAliases());
				default:
					llvm_unreachable("Unknown SEM type kind enum.");
			}
		}
		
		bool TypeInfo::isObjectSizeKnownInThisModule(const SEM::TypeInstance& typeInstance) const {
			if (typeInstance.isEnum() || typeInstance.isStruct() || typeInstance.isUnion()) {
				// C types can only contain known size members.
				return true;
			} else if (typeInstance.isClassDef() || typeInstance.isDatatype() || typeInstance.isException()) {
				// All members of the type must have a known size
				// for it to have a known size.
				for (auto var: typeInstance.variables()) {
					if (!isSizeKnownInThisModule(var->type())) {
						return false;
					}
				}
				return true;
			} else if (typeInstance.isUnionDatatype()) {
				for (auto variantTypeInstance: typeInstance.variants()) {
					if (!isObjectSizeKnownInThisModule(*variantTypeInstance)) {
						return false;
					}
				}
				return true;
			} else {
				return false;
			}
		}
		
		bool TypeInfo::hasCustomDestructor(const SEM::Type* const type) const {
			if (type->isObject()) {
				if (type->isPrimitive()) {
					const auto& primitive = module_.getPrimitive(*(type->getObjectType()));
					return primitive.hasCustomDestructor(*this,
					                                     arrayRef(type->templateArguments()));
				} else {
					return objectHasCustomDestructor(*(type->getObjectType()));
				}
			} else {
				return type->isTemplateVar();
			}
		}
		
		bool TypeInfo::objectHasCustomDestructor(const SEM::TypeInstance& typeInstance) const {
			if (typeInstance.isClassDecl()) {
				// Assume a destructor exists.
				return true;
			}
			
			if (typeInstance.isPrimitive()) {
				const auto& primitive = module_.getPrimitive(typeInstance);
				return primitive.hasCustomDestructor(*this,
				                                     arrayRef(typeInstance.selfTemplateArgs()));
			}
			
			if (typeInstance.isUnionDatatype()) {
				for (const auto variantTypeInstance: typeInstance.variants()) {
					if (objectHasCustomDestructor(*variantTypeInstance)) {
						return true;
					}
				}
				
				return false;
			} else {
				const auto destroyFunction = typeInstance.functions().at(module_.getCString("__destroy")).get();
				if (!destroyFunction->isDefault()) {
					return true;
				}
				
				for (const auto var: typeInstance.variables()) {
					if (hasCustomDestructor(var->type())) {
						return true;
					}
				}
				
				return false;
			}
		}
		
		bool TypeInfo::hasCustomMove(const SEM::Type* const type) const {
			if (type->isObject()) {
				if (type->isPrimitive()) {
					const auto& primitive = module_.getPrimitive(*(type->getObjectType()));
					return primitive.hasCustomMove(*this,
					                               arrayRef(type->templateArguments()));
				} else {
					return objectHasCustomMove(*(type->getObjectType()))
					       // We have a custom move operation if there's a liveness indicator,
					       // since we need to set the source value to be dead.
					       || objectHasLivenessIndicator(*(type->getObjectType()));
				}
			} else {
				return type->isTemplateVar();
			}
		}
		
		bool TypeInfo::objectHasCustomMove(const SEM::TypeInstance& typeInstance) const {
			if (typeInstance.isClassDecl()) {
				// Assume a custom move function exists.
				return true;
			}
			
			if (typeInstance.isPrimitive()) {
				const auto& primitive = module_.getPrimitive(typeInstance);
				return primitive.hasCustomMove(*this,
				                               arrayRef(typeInstance.selfTemplateArgs()));
			}
			
			if (typeInstance.isUnionDatatype()) {
				for (const auto variantTypeInstance: typeInstance.variants()) {
					if (objectHasCustomMove(*variantTypeInstance)) {
						return true;
					}
				}
				
				return false;
			} else {
				if (objectHasCustomMoveMethod(typeInstance)) {
					return true;
				}
				
				for (const auto var: typeInstance.variables()) {
					if (hasCustomMove(var->type())) {
						return true;
					}
				}
				
				return false;
			}
		}
		
		bool TypeInfo::objectHasCustomMoveMethod(const SEM::TypeInstance& typeInstance) const {
			const auto moveFunction = typeInstance.functions().at(module_.getCString("__moveto")).get();
			return !moveFunction->isDefault();
		}
		
		bool TypeInfo::hasLivenessIndicator(const SEM::Type* const type) const {
			return type->isObject() && objectHasLivenessIndicator(*(type->getObjectType()));
		}
		
		bool TypeInfo::objectHasLivenessIndicator(const SEM::TypeInstance& typeInstance) const {
			// A liveness indicator is only required if the object has a custom destructor or move,
			// since the indicator is used to determine whether the destructor/move is run.
			return typeInstance.isClassDef() &&
			       (objectHasCustomMoveMethod(typeInstance) ||
			        objectHasCustomDestructor(typeInstance));
		}
		
	}
	
}
