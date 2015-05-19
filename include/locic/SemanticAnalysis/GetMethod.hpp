#ifndef LOCIC_SEMANTICANALYSIS_GETMETHOD_HPP
#define LOCIC_SEMANTICANALYSIS_GETMETHOD_HPP

#include <locic/SEM/ValueArray.hpp>

namespace locic {
	
	class String;
	
	namespace Debug {
		
		class SourceLocation;
		
	}
	
	namespace SEM {
		
		class Type;
		class Value;
		
	}

	namespace SemanticAnalysis {
		
		class Context;
		
		SEM::Value GetStaticMethod(Context& context, SEM::Value value, const String& methodName, const Debug::SourceLocation& location);
		
		SEM::Value GetMethod(Context& context, SEM::Value value, const String& methodName, const Debug::SourceLocation& location);
		
		SEM::Value GetTemplatedMethod(Context& context, SEM::Value value, const String& methodName,
			SEM::ValueArray templateArguments, const Debug::SourceLocation& location);
		
		SEM::Value GetSpecialMethod(Context& context, SEM::Value value, const String& methodName, const Debug::SourceLocation& location);
		
		SEM::Value GetMethodWithoutResolution(Context& context,
			SEM::Value value,
			const SEM::Type* type,
			const String& methodName,
			const Debug::SourceLocation& location);
		
		SEM::Value GetTemplatedMethodWithoutResolution(Context& context,
			SEM::Value value,
			const SEM::Type* type,
			const String& methodName,
			SEM::ValueArray templateArguments,
			const Debug::SourceLocation& location);
		
	}
	
}

#endif
