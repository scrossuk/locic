#include <cstddef>
#include <cstdio>
#include <list>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertClassDef.hpp>
#include <Locic/SemanticAnalysis/ConvertFunctionDef.hpp>
#include <Locic/SemanticAnalysis/ConvertNamespace.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		void ConvertNamespace(Context& context) {
			Node& node = context.node();
			
			assert(node.isNamespace());
			
			for(StringMap<Node>::Range range = node.children().range(); !range.empty(); range.popFront()){
				const Node& childNode = range.front().value();
				Context newContext(context, range.front().key(), childNode);
				if(childNode.isFunction()){
					if(childNode.getASTFunction()->scope != NULL){
						ConvertFunctionDef(newContext);
					}
				}else if(childNode.isNamespace()){
					ConvertNamespace(newContext);
				}else if(childNode.isTypeInstance()){
					if(childNode.getSEMTypeInstance()->isClassDef()){
						ConvertClassDef(newContext);
					}
				}
			}
		}
		
	}
	
}

