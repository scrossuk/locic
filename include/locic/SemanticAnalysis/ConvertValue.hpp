#ifndef LOCIC_SEMANTICANALYSIS_CONVERTVALUE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTVALUE_HPP

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Value* ConvertValue(Context& context, AST::Node<AST::Value> value);
		
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
