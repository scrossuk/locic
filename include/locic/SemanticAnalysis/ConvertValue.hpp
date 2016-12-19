#ifndef LOCIC_SEMANTICANALYSIS_CONVERTVALUE_HPP
#define LOCIC_SEMANTICANALYSIS_CONVERTVALUE_HPP

#include <locic/AST.hpp>
#include <locic/SEM.hpp>
#include <locic/SemanticAnalysis/Context.hpp>
#include <locic/SemanticAnalysis/Exception.hpp>
#include <locic/Support/String.hpp>

namespace locic {

	namespace SemanticAnalysis {
	
		SEM::Value ConvertValue(Context& context, const AST::Node<AST::ValueDecl>& value);
		
		class BuiltInTypeMissingException final: public Exception {
			public:
				inline BuiltInTypeMissingException(const String& typeName)
					: typeName_(typeName) { }
					
				inline std::string toString() const {
					return makeString("Built-in type '%s' is missing.", typeName_.c_str());
				}
				
			private:
				String typeName_;
				
		};
		
	}
	
}

#endif
