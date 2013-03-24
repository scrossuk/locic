#ifndef LOCIC_SEMANTICANALYSIS_CONVERTVALUE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTVALUE_HPP

#include <Locic/AST.hpp>
#include <Locic/SEM.hpp>
#include <Locic/SemanticAnalysis/Context.hpp>
#include <Locic/SemanticAnalysis/Exception.hpp>

namespace Locic {

	namespace SemanticAnalysis {
	
		SEM::Value* ConvertValue(Context& context, AST::Value* value);
		
		class BuiltInTypeMissingException: public Exception {
			public:
				inline BuiltInTypeMissingException(const std::string& typeName)
					: typeName_(typeName) { }
					
				inline std::string toString() const {
					return makeString("Built-in type '%s' is missing.", typeName_.c_str());
				}
				
			private:
				std::string typeName_;
				
		};
		
	}
	
}

#endif
