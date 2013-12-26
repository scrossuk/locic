#include <cassert>
#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/ConvertFunctionDef.hpp>

namespace locic {

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

