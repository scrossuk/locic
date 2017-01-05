#include <locic/AST/Function.hpp>
#include <locic/AST/Type.hpp>
#include <locic/CodeGen/Module.hpp>
#include <locic/CodeGen/Primitive.hpp>
#include <locic/CodeGen/Support.hpp>
#include <locic/CodeGen/TypeInfo.hpp>
#include <locic/AST/TypeInstance.hpp>

namespace locic {
	
	namespace CodeGen {
		
		TypeInfo::TypeInfo(Module& module)
		: module_(module) { }
		
		bool TypeInfo::canPassByValue(const AST::Type* const type) const {
			// Can only pass by value if the type's size is always known
			// (it's not enough for its size to be known in this module
			// since other modules may end up using it) and if it
			// doesn't have a custom move method (which means it
			// must stay in memory and we must hold references to it).
			return isSizeAlwaysKnown(type) && !hasCustomMove(type);
		}
		
		bool TypeInfo::isSizeAlwaysKnown(const AST::Type* const type) const {
			switch (type->kind()) {
				case AST::Type::OBJECT:
					if (type->isPrimitive()) {
						const auto& primitive = module_.getPrimitive(*(type->getObjectType()));
						return primitive.isSizeAlwaysKnown(*this,
						                                   arrayRef(type->templateArguments()));
					} else {
						return isObjectSizeAlwaysKnown(*(type->getObjectType()));
					}
				case AST::Type::TEMPLATEVAR:
					return false;
				case AST::Type::ALIAS:
					return isSizeAlwaysKnown(type->resolveAliases());
				default:
					llvm_unreachable("Unknown AST type kind enum.");
			}
		}
		
		bool TypeInfo::isObjectSizeAlwaysKnown(const AST::TypeInstance& typeInstance) const {
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
		
		bool TypeInfo::isSizeKnownInThisModule(const AST::Type* const type) const {
			switch (type->kind()) {
				case AST::Type::OBJECT: {
					if (type->isPrimitive()) {
						const auto& primitive = module_.getPrimitive(*(type->getObjectType()));
						return primitive.isSizeKnownInThisModule(*this,
						                                         arrayRef(type->templateArguments()));
					} else {
						return isObjectSizeKnownInThisModule(*(type->getObjectType()));
					}
				}
				case AST::Type::TEMPLATEVAR:
					return false;
				case AST::Type::ALIAS:
					return isSizeKnownInThisModule(type->resolveAliases());
				default:
					llvm_unreachable("Unknown AST type kind enum.");
			}
		}
		
		bool TypeInfo::isObjectSizeKnownInThisModule(const AST::TypeInstance& typeInstance) const {
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
		
		bool TypeInfo::hasCustomDestructor(const AST::Type* const type) const {
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
		
		bool TypeInfo::objectHasCustomDestructor(const AST::TypeInstance& typeInstance) const {
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
				if (typeInstance.isClassDef() &&
				     objectHasCustomDestructorMethod(typeInstance)) {
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
		
		bool TypeInfo::objectHasCustomDestructorMethod(const AST::TypeInstance& typeInstance) const {
			assert(typeInstance.isClassDef());
			const auto& destroyFunction = typeInstance.getFunction(module_.getCString("__destroy"));
			return !destroyFunction.isAutoGenerated();
		}
		
		bool TypeInfo::hasCustomMove(const AST::Type* const type) const {
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
		
		bool TypeInfo::objectHasCustomMove(const AST::TypeInstance& typeInstance) const {
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
				if (typeInstance.isClassDef() &&
				     objectHasCustomMoveMethod(typeInstance)) {
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
		
		bool TypeInfo::objectHasCustomMoveMethod(const AST::TypeInstance& typeInstance) const {
			assert(typeInstance.isClassDef());
			const auto& moveFunction = typeInstance.getFunction(module_.getCString("__move"));
			return !moveFunction.isAutoGenerated();
		}
		
		bool TypeInfo::hasLivenessIndicator(const AST::Type* const type) const {
			return type->isObject() && objectHasLivenessIndicator(*(type->getObjectType()));
		}
		
		bool TypeInfo::objectHasLivenessIndicator(const AST::TypeInstance& typeInstance) const {
			// A liveness indicator is only required if the object has a custom destructor or move,
			// since the indicator is used to determine whether the destructor/move is run.
			return typeInstance.isClassDef() &&
			       (objectHasCustomMoveMethod(typeInstance) ||
			        objectHasCustomDestructorMethod(typeInstance));
		}
		
	}
	
}
