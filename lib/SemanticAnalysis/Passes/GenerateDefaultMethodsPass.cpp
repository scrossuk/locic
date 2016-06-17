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
			
			DefaultMethods defaultMethods(context);
			
			// Add default __alignmask method.
			const bool hasDefaultAlignMask = defaultMethods.hasDefaultAlignMask(&typeInstance);
			if (hasDefaultAlignMask) {
				typeInstance.attachFunction(defaultMethods.createDefaultAlignMaskDecl(&typeInstance,
				                                                                      typeInstance.name() + context.getCString("__alignmask")));
			}
			
			// Add default __sizeof method.
			const bool hasDefaultSizeOf = defaultMethods.hasDefaultSizeOf(&typeInstance);
			if (hasDefaultSizeOf) {
				typeInstance.attachFunction(defaultMethods.createDefaultSizeOfDecl(&typeInstance,
				                                                                   typeInstance.name() + context.getCString("__sizeof")));
			}
			
			// Add default __destroy method.
			const bool hasDefaultDestroy = defaultMethods.hasDefaultDestroy(&typeInstance);
			if (hasDefaultDestroy) {
				typeInstance.attachFunction(defaultMethods.createDefaultDestroyDecl(&typeInstance,
				                                                                    typeInstance.name() + context.getCString("__destroy")));
			}
			
			// Add default __moveto method.
			const bool hasDefaultMove = defaultMethods.hasDefaultMove(&typeInstance);
			if (hasDefaultMove) {
				typeInstance.attachFunction(defaultMethods.createDefaultMoveDecl(&typeInstance,
				                                                                 typeInstance.name() + context.getCString("__moveto")));
			}
			
			// Add default __setdead method.
			const bool hasDefaultSetDead = defaultMethods.hasDefaultSetDead(&typeInstance);
			if (hasDefaultSetDead) {
				typeInstance.attachFunction(defaultMethods.createDefaultSetDeadDecl(&typeInstance,
				                                                                    typeInstance.name() + context.getCString("__setdead")));
			}
			
			// Add default __islive method.
			const bool hasDefaultIsLive = defaultMethods.hasDefaultIsLive(&typeInstance);
			if (hasDefaultIsLive) {
				typeInstance.attachFunction(defaultMethods.createDefaultIsLiveDecl(&typeInstance,
				                                                                   typeInstance.name() + context.getCString("__islive")));
			}
			
			// All non-class types can also get various other default methods implicitly
			// (which must be specified explicitly for classes).
			if (!typeInstance.isClass()) {
				// Add default constructor.
				if (defaultMethods.hasDefaultConstructor(&typeInstance)) {
					// Add constructor for exception types using initializer;
					// for other types just add a default constructor.
					auto methodDecl =
						typeInstance.isException() ?
							CreateExceptionConstructorDecl(context, &typeInstance) :
							defaultMethods.createDefaultConstructorDecl(&typeInstance,
							                                            typeInstance.name() + context.getCString("create"));
					typeInstance.attachFunction(std::move(methodDecl));
				}
				
				if (!typeInstance.isException()) {
					// Add default implicit copy if available.
					if (defaultMethods.hasDefaultImplicitCopy(&typeInstance)) {
						typeInstance.attachFunction(defaultMethods.createDefaultImplicitCopyDecl(&typeInstance,
						                                                                         typeInstance.name() + context.getCString("implicitcopy")));
					}
					
					// Add default compare for datatypes if available.
					if (defaultMethods.hasDefaultCompare(&typeInstance)) {
						typeInstance.attachFunction(defaultMethods.createDefaultCompareDecl(&typeInstance,
						                                                                    typeInstance.name() + context.getCString("compare")));
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
