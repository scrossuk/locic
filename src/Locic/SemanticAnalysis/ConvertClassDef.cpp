#include <cassert>
#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/ConvertFunctionDef.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		void ConvertClassDef(Context& context) {
			Node& node = context.node();
			
			assert(node.isTypeInstance());
			assert(node.getSEMTypeInstance()->isClassDef());
			
			for (StringMap<Node>::Range range = node.children().range(); !range.empty(); range.popFront()) {
				const Node& childNode = range.front().value();
				
				if (!childNode.isFunction()) continue;
				
				Context functionContext(context, range.front().key(), childNode);
				ConvertFunctionDef(functionContext);
			}
		}
		
	}
	
}

