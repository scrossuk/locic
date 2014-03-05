#include <cstddef>
#include <cstdio>
#include <list>
#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertClassDef.hpp>
#include <locic/SemanticAnalysis/ConvertFunctionDef.hpp>
#include <locic/SemanticAnalysis/ConvertNamespace.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		void ConvertNamespace(Context& context) {
			Node& node = context.node();
			
			assert(node.isNamespace());
			
			for (StringMap<Node>::Range range = node.children().range(); !range.empty(); range.popFront()) {
				const Node& childNode = range.front().value();
				NodeContext newContext(context, range.front().key(), childNode);
				if(childNode.isFunction()){
					ConvertFunctionDef(newContext);
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

