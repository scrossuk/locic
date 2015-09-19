#include <locic/AST.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertException.hpp>
#include <locic/SemanticAnalysis/DefaultMethods.hpp>
#include <locic/SemanticAnalysis/ScopeStack.hpp>

namespace locic {
	
	namespace SemanticAnalysis {
		
		void GenerateTypeDefaultMethods(Context& context, SEM::TypeInstance& typeInstance) {
			if (typeInstance.isInterface() || typeInstance.isPrimitive()) {
				// Skip interfaces and primitives since default
				// method generation doesn't apply to them.
				return;
			}
			
			if (typeInstance.isOpaqueStruct()) {
				// Opaque structs don't have any methods.
				return;
			}
			
			// Add default __alignmask method.
			const bool hasDefaultAlignMask = HasDefaultAlignMask(context, &typeInstance);
			if (hasDefaultAlignMask) {
				auto methodDecl = CreateDefaultAlignMaskDecl(context, &typeInstance, typeInstance.name() + context.getCString("__alignmask"));
				typeInstance.functions().insert(std::make_pair(context.getCString("__alignmask"), std::move(methodDecl)));
			}
			
			// Add default __sizeof method.
			const bool hasDefaultSizeOf = HasDefaultSizeOf(context, &typeInstance);
			if (hasDefaultSizeOf) {
				auto methodDecl = CreateDefaultSizeOfDecl(context, &typeInstance, typeInstance.name() + context.getCString("__sizeof"));
				typeInstance.functions().insert(std::make_pair(context.getCString("__sizeof"), std::move(methodDecl)));
			}
			
			// Add default __destroy method.
			const bool hasDefaultDestroy = HasDefaultDestroy(context, &typeInstance);
			if (hasDefaultDestroy) {
				auto methodDecl = CreateDefaultDestroyDecl(context, &typeInstance, typeInstance.name() + context.getCString("__destroy"));
				typeInstance.functions().insert(std::make_pair(context.getCString("__destroy"), std::move(methodDecl)));
			}
			
			// Add default __moveto method.
			const bool hasDefaultMove = HasDefaultMove(context, &typeInstance);
			if (hasDefaultMove) {
				auto methodDecl = CreateDefaultMoveDecl(context, &typeInstance, typeInstance.name() + context.getCString("__moveto"));
				typeInstance.functions().insert(std::make_pair(context.getCString("__moveto"), std::move(methodDecl)));
			}
			
			// Add default __setdead method.
			const bool hasDefaultSetDead = HasDefaultSetDead(context, &typeInstance);
			if (hasDefaultSetDead) {
				auto methodDecl = CreateDefaultSetDeadDecl(context, &typeInstance, typeInstance.name() + context.getCString("__setdead"));
				typeInstance.functions().insert(std::make_pair(context.getCString("__setdead"), std::move(methodDecl)));
			}
			
			// Add default __islive method.
			const bool hasDefaultIsLive = HasDefaultIsLive(context, &typeInstance);
			if (hasDefaultIsLive) {
				auto methodDecl = CreateDefaultIsLiveDecl(context, &typeInstance, typeInstance.name() + context.getCString("__islive"));
				typeInstance.functions().insert(std::make_pair(context.getCString("__islive"), std::move(methodDecl)));
			}
			
			// All non-class types can also get various other default methods implicitly
			// (which must be specified explicitly for classes).
			if (!typeInstance.isClass()) {
				// Add default constructor.
				if (HasDefaultConstructor(context, &typeInstance)) {
					// Add constructor for exception types using initializer;
					// for other types just add a default constructor.
					auto methodDecl =
						typeInstance.isException() ?
							CreateExceptionConstructorDecl(context, &typeInstance) :
							CreateDefaultConstructorDecl(context, &typeInstance, typeInstance.name() + context.getCString("create"));
					typeInstance.functions().insert(std::make_pair(context.getCString("create"), std::move(methodDecl)));
				}
				
				if (!typeInstance.isException()) {
					// Add default implicit copy if available.
					if (HasDefaultImplicitCopy(context, &typeInstance)) {
						auto methodDecl = CreateDefaultImplicitCopyDecl(context, &typeInstance, typeInstance.name() + context.getCString("implicitcopy"));
						typeInstance.functions().insert(std::make_pair(context.getCString("implicitcopy"), std::move(methodDecl)));
					}
					
					// Add default compare for datatypes if available.
					if (HasDefaultCompare(context, &typeInstance)) {
						auto methodDecl = CreateDefaultCompareDecl(context, &typeInstance, typeInstance.name() + context.getCString("compare"));
						typeInstance.functions().insert(std::make_pair(context.getCString("compare"), std::move(methodDecl)));
					}
				}
			}
		}
		
		void GenerateNamespaceDefaultMethods(Context& context, SEM::Namespace& nameSpace) {
			for (const auto& itemPair: nameSpace.items()) {
				const auto& item = itemPair.second;
				if (item.isNamespace()) {
					GenerateNamespaceDefaultMethods(context, item.nameSpace());
				} else if (item.isTypeInstance()) {
					GenerateTypeDefaultMethods(context, item.typeInstance());
				}
			}
		}
		
		void GenerateDefaultMethodsPass(Context& context) {
			const auto semNamespace = context.scopeStack().back().nameSpace();
			GenerateNamespaceDefaultMethods(context, *semNamespace);
			
			// All methods are now known so we can start producing method sets.
			context.setMethodSetsComplete();
		}
		
	}
	
}
