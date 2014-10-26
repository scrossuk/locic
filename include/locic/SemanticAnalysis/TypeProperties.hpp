#ifndef LOCIC_SEMANTICANALYSIS_TYPEPROPERTIES_HPP
#define LOCIC_SEMANTICANALYSIS_TYPEPROPERTIES_HPP

#include <string>
#include <vector>

#include <locic/SEM.hpp>

namespace locic {

	namespace SemanticAnalysis {
		
		SEM::Value* GetStaticMethod(Context& context, SEM::Value* value, const std::string& methodName, const Debug::SourceLocation& location);
		
		SEM::Value* GetMethod(Context& context, SEM::Value* value, const std::string& methodName, const Debug::SourceLocation& location);
		
		SEM::Value* GetTemplatedMethod(Context& context, SEM::Value* value, const std::string& methodName,
		                               const std::vector<const SEM::Type*>& templateArguments, const Debug::SourceLocation& location);
		
		SEM::Value* GetSpecialMethod(Context& context, SEM::Value* value, const std::string& methodName, const Debug::SourceLocation& location);
		
		SEM::Value* GetMethodWithoutResolution(Context& context, SEM::Value* value, const SEM::Type* type,
		                                       const std::string& methodName, const Debug::SourceLocation& location);
		
		SEM::Value* GetTemplatedMethodWithoutResolution(Context& context, SEM::Value* value, const SEM::Type* type,
		                                                const std::string& methodName, const std::vector<const SEM::Type*>& templateArguments,
		                                                const Debug::SourceLocation& location);
		
		SEM::Value* CallValue(Context& context, SEM::Value* value, const std::vector<SEM::Value*>& args, const Debug::SourceLocation& location);
		
		bool supportsNullConstruction(Context& context, const SEM::Type* type);
		
		bool supportsImplicitCast(const SEM::Type* type);
		
		bool supportsImplicitCopy(Context& context, const SEM::Type* type);
		
		bool supportsExplicitCopy(Context& context, const SEM::Type* type);
		
		bool supportsNoExceptImplicitCopy(Context& context, const SEM::Type* type);
		
		bool supportsNoExceptExplicitCopy(Context& context, const SEM::Type* type);
		
		bool supportsCompare(Context& context, const SEM::Type* type);
		
		bool supportsMove(Context& context, const SEM::Type* const rawType);
		
	}
	
}

#endif
