#ifndef LOCIC_SEMANTICANALYSIS_TYPEBUILDER_HPP
#define LOCIC_SEMANTICANALYSIS_TYPEBUILDER_HPP

#include <string>

namespace locic {
	
	namespace SEM {
		
		class FunctionType;
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		class TypeBuilder {
		public:
			TypeBuilder(Context& context);
			
			const SEM::Type*
			getBoolType();
			
			const SEM::Type*
			getSizeType();
			
			const SEM::Type*
			getTypenameType();
			
			const SEM::Type*
			getPointerType(const SEM::Type* elementType);
		
			const SEM::Type*
			getConstantStaticArrayType(const SEM::Type* elementType,
			                           size_t arraySize,
			                           const Debug::SourceLocation& location);
		
			const SEM::Type*
			getStaticArrayType(const SEM::Type* elementType,
			                   SEM::Value arraySize,
			                   const Debug::SourceLocation& location);
			
			const SEM::Type*
			getPrimitiveCallableType(SEM::FunctionType functionType,
			                         const std::string& prefix,
			                         const std::string& suffix);
			
			const SEM::Type*
			getTrivialFunctionPointerType(SEM::FunctionType functionType);
			
			const SEM::Type*
			getTemplatedFunctionPointerType(SEM::FunctionType functionType);
			
			const SEM::Type*
			getMethodFunctionPointerType(SEM::FunctionType functionType);
			
			const SEM::Type*
			getTemplatedMethodFunctionPointerType(SEM::FunctionType functionType);
			
			const SEM::Type*
			getVarArgFunctionPointerType(SEM::FunctionType functionType);
			
			const SEM::Type*
			getFunctionPointerType(SEM::FunctionType functionType);
			
			const SEM::Type*
			getTrivialMethodType(SEM::FunctionType functionType);
			
			const SEM::Type*
			getTemplatedMethodType(SEM::FunctionType functionType);
			
			const SEM::Type*
			getMethodType(SEM::FunctionType functionType);
			
			const SEM::Type*
			getInterfaceMethodType(SEM::FunctionType functionType);
			
			const SEM::Type*
			getStaticInterfaceMethodType(SEM::FunctionType functionType);
			
		private:
			Context& context_;
			const SEM::Type* cachedBoolType_;
			const SEM::Type* cachedTypenameType_;
			
		};
		
	}
	
}

#endif
