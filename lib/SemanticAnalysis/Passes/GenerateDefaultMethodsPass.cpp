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
				typeInstance.attachFunction(CreateDefaultAlignMaskDecl(context, &typeInstance, typeInstance.name() + context.getCString("__alignmask")));
			}
			
			// Add default __sizeof method.
			const bool hasDefaultSizeOf = HasDefaultSizeOf(context, &typeInstance);
			if (hasDefaultSizeOf) {
				typeInstance.attachFunction(CreateDefaultSizeOfDecl(context, &typeInstance, typeInstance.name() + context.getCString("__sizeof")));
			}
			
			// Add default __destroy method.
			const bool hasDefaultDestroy = HasDefaultDestroy(context, &typeInstance);
			if (hasDefaultDestroy) {
				typeInstance.attachFunction(CreateDefaultDestroyDecl(context, &typeInstance, typeInstance.name() + context.getCString("__destroy")));
			}
			
			// Add default __moveto method.
			const bool hasDefaultMove = HasDefaultMove(context, &typeInstance);
			if (hasDefaultMove) {
				typeInstance.attachFunction(CreateDefaultMoveDecl(context, &typeInstance, typeInstance.name() + context.getCString("__moveto")));
			}
			
			// Add default __setdead method.
			const bool hasDefaultSetDead = HasDefaultSetDead(context, &typeInstance);
			if (hasDefaultSetDead) {
				typeInstance.attachFunction(CreateDefaultSetDeadDecl(context, &typeInstance, typeInstance.name() + context.getCString("__setdead")));
			}
			
			// Add default __islive method.
			const bool hasDefaultIsLive = HasDefaultIsLive(context, &typeInstance);
			if (hasDefaultIsLive) {
				typeInstance.attachFunction(CreateDefaultIsLiveDecl(context, &typeInstance, typeInstance.name() + context.getCString("__islive")));
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
					typeInstance.attachFunction(std::move(methodDecl));
				}
				
				if (!typeInstance.isException()) {
					// Add default implicit copy if available.
					if (HasDefaultImplicitCopy(context, &typeInstance)) {
						typeInstance.attachFunction(CreateDefaultImplicitCopyDecl(context, &typeInstance, typeInstance.name() + context.getCString("implicitcopy")));
					}
					
					// Add default compare for datatypes if available.
					if (HasDefaultCompare(context, &typeInstance)) {
						typeInstance.attachFunction(CreateDefaultCompareDecl(context, &typeInstance, typeInstance.name() + context.getCString("compare")));
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
			auto& semNamespace = context.scopeStack().back().nameSpace();
			GenerateNamespaceDefaultMethods(context, semNamespace);
			
			// All methods are now known so we can start producing method sets.
			context.setMethodSetsComplete();
		}
		
	}
	
}
