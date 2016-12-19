#ifndef LOCIC_SEMANTICANALYSIS_GETMETHOD_HPP
#define LOCIC_SEMANTICANALYSIS_GETMETHOD_HPP

#include <locic/AST/ValueArray.hpp>

namespace locic {
	
	class String;
	
	namespace AST {
		
		class Type;
		class Value;
		
	}
	
	namespace Debug {
		
		class SourceLocation;
		
	}

	namespace SemanticAnalysis {
		
		class Context;
		
		AST::Value GetStaticMethod(Context& context, AST::Value value, const String& methodName, const Debug::SourceLocation& location);
		
		AST::Value GetMethod(Context& context, AST::Value value, const String& methodName, const Debug::SourceLocation& location);
		
		AST::Value GetTemplatedMethod(Context& context, AST::Value value, const String& methodName,
			AST::ValueArray templateArguments, const Debug::SourceLocation& location);
		
		AST::Value GetSpecialMethod(Context& context, AST::Value value, const String& methodName, const Debug::SourceLocation& location);
		
		AST::Value GetMethodWithoutResolution(Context& context,
			AST::Value value,
			const AST::Type* type,
			const String& methodName,
			const Debug::SourceLocation& location);
		
		AST::Value GetTemplatedMethodWithoutResolution(Context& context,
			AST::Value value,
			const AST::Type* type,
			const String& methodName,
			AST::ValueArray templateArguments,
			const Debug::SourceLocation& location);
		
	}
	
}

#endif
