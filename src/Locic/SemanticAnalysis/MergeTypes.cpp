#include <assert.h>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Exception.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::TypeInstance* MergeClasses(SEM::TypeInstance* first, SEM::TypeInstance* second) {
			assert(first->isClass());
			assert(second->isClass());
			
			if (first->isClassDecl() && second->isClassDecl()) {
				// Two declarations can be merged.
				// TODO: check these declarations match up,
				//       and return the most detailed one.
				return first;
			} else if ((first->isClassDef() && second->isClassDecl()) || (first->isClassDecl() && second->isClassDef())) {
				auto decl = first->isClassDecl() ? first : second;
				auto def = first->isClassDef() ? first : second;
				assert(decl->isClassDecl());
				assert(def->isClassDef());
				
				// A declaration and definition can be merged.
				// TODO: check they match up.
				return def;
			} else {
				assert(first->isClassDef() && second->isClassDef());
				// Two definitions cannot be merged.
				throw NonUnifiableTypeClashException(first->name());
			}
		}
		
		SEM::TypeInstance* MergeStructs(SEM::TypeInstance* first, SEM::TypeInstance* second) {
			assert(first->isStruct());
			assert(second->isStruct());
			
			if (first->isStructDecl() && second->isStructDecl()) {
				// Two declarations can be merged.
				return first;
			} else if ((first->isStructDef() && second->isStructDecl()) || (first->isStructDecl() && second->isStructDef())) {
				auto decl = first->isStructDecl() ? first : second;
				auto def = first->isStructDef() ? first : second;
				assert(decl->isStructDecl());
				assert(def->isStructDef());
				
				// A declaration and definition can be merged.
				// TODO: check they match up.
				return def;
			} else {
				assert(first->isStructDef() && second->isStructDef());
				// Two definitions cannot be merged.
				throw NonUnifiableTypeClashException(first->name());
			}
		}
		
		SEM::TypeInstance* MergeTypeInstances(SEM::TypeInstance* first, SEM::TypeInstance* second) {
			if (first->isClass() && second->isClass()) {
				return MergeClasses(first, second);
			} else if (first->isStruct() && second->isStruct()) {
				return MergeStructs(first, second);
			} else if (first->kind() == second->kind()) {
				// TODO: handle merging other kinds of type instances...
				return first;
			} else {
				throw NonUnifiableTypeClashException(first->name());
			}
		}
		
		SEM::TypeInstance::Kind MergeTypeInstanceKinds(const Name& fullTypeName, SEM::TypeInstance::Kind first, SEM::TypeInstance::Kind second) {
			switch (first) {
				case SEM::TypeInstance::CLASSDECL:
				{
					if (second == SEM::TypeInstance::CLASSDECL) {
						return SEM::TypeInstance::CLASSDECL;
					} else if (second == SEM::TypeInstance::CLASSDEF) {
						return SEM::TypeInstance::CLASSDEF;
					} else {
						throw NonUnifiableTypeClashException(fullTypeName);
					}
				}
				case SEM::TypeInstance::CLASSDEF:
				{
					if (second == SEM::TypeInstance::CLASSDECL) {
						return SEM::TypeInstance::CLASSDEF;
					} else {
						throw NonUnifiableTypeClashException(fullTypeName);
					}
				}
				case SEM::TypeInstance::STRUCTDECL:
				{
					if (second == SEM::TypeInstance::STRUCTDECL) {
						return SEM::TypeInstance::STRUCTDECL;
					} else if (second == SEM::TypeInstance::STRUCTDEF) {
						return SEM::TypeInstance::STRUCTDEF;
					} else {
						throw NonUnifiableTypeClashException(fullTypeName);
					}
				}
				case SEM::TypeInstance::STRUCTDEF:
				{
					if (second == SEM::TypeInstance::STRUCTDECL) {
						return SEM::TypeInstance::STRUCTDEF;
					} else {
						throw NonUnifiableTypeClashException(fullTypeName);
					}
				}
				default:
				{
					if (first == second) {
						return first;
					} else {
						throw NonUnifiableTypeClashException(fullTypeName);
					}
				}
			}
		}
		
	}
	
}


