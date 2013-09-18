#ifndef LOCIC_SEMANTICANALYSIS_METHODPATTERN_HPP
#define LOCIC_SEMANTICANALYSIS_METHODPATTERN_HPP

#include <string>
#include <vector>
#include <Locic/SemanticAnalysis/Node.hpp>

namespace Locic {

	namespace SemanticAnalysis {
		
		static const bool IS_STATIC = true;
		static const bool IS_NOT_STATIC = false;
		
		static const bool CHECK_PARAM_COUNT = true;
		static const bool DO_NOT_CHECK_PARAM_COUNT = false;
		
		struct MethodPattern {
			std::string name;
			bool isStatic;
			bool checkParamCount;
			size_t numParameters;
			
			inline MethodPattern(const std::string& n, bool isS, bool cPC, size_t nP)
				: name(n), isStatic(isS), checkParamCount(cPC), numParameters(nP) {
				assert(n.size() > 0 && "Can't use empty method name in pattern.");
			}
		};
		
		// Standard patterns.
		MethodPattern DefaultConstructorPattern();
		MethodPattern NullConstructorPattern();
		MethodPattern ImplicitCopyPattern();
		MethodPattern OpAddressPattern();
		MethodPattern OpAssignPattern();
		MethodPattern OpDissolvePattern();
		MethodPattern OpMovePattern();
		MethodPattern DestructorPattern();
		
		std::vector<MethodPattern> GetStandardPatterns();
	
		Node FindMethodPattern(const MethodPattern& pattern, const Node& typeNode);
		
	}
	
}

#endif
