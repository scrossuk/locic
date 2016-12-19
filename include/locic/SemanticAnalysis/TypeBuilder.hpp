#ifndef LOCIC_SEMANTICANALYSIS_TYPEBUILDER_HPP
#define LOCIC_SEMANTICANALYSIS_TYPEBUILDER_HPP

#include <string>

namespace locic {
	
	class PrimitiveID;
	
	namespace AST {
		
		class FunctionType;
		class Type;
		
	}
	
	namespace SemanticAnalysis {
		
		class Context;
		
		class TypeBuilder {
		public:
			TypeBuilder(Context& context);
			
			const AST::Type*
			getPrimitiveType(PrimitiveID primitiveID,
			                 SEM::ValueArray templateArguments = {});
			
			const AST::Type*
			getVoidType();
			
			const AST::Type*
			getBoolType();
			
			const AST::Type*
			getIntType();
			
			const AST::Type*
			getSizeType();
			
			const AST::Type*
			getTypenameType();
			
			const AST::Type*
			getMovableInterfaceType();
			
			const AST::Type*
			getPointerType(const AST::Type* elementType);
		
			const AST::Type*
			getConstantStaticArrayType(const AST::Type* elementType,
			                           size_t arraySize,
			                           const Debug::SourceLocation& location);
		
			const AST::Type*
			getStaticArrayType(const AST::Type* elementType,
			                   SEM::Value arraySize,
			                   const Debug::SourceLocation& location);
			
			const AST::Type*
			getPrimitiveCallableType(AST::FunctionType functionType,
			                         const char* functionTypeName);
			
			const AST::Type*
			getTrivialFunctionPointerType(AST::FunctionType functionType);
			
			const AST::Type*
			getTemplatedFunctionPointerType(AST::FunctionType functionType);
			
			const AST::Type*
			getMethodFunctionPointerType(AST::FunctionType functionType);
			
			const AST::Type*
			getTemplatedMethodFunctionPointerType(AST::FunctionType functionType);
			
			const AST::Type*
			getVarArgFunctionPointerType(AST::FunctionType functionType);
			
			const AST::Type*
			getFunctionPointerType(AST::FunctionType functionType);
			
			const AST::Type*
			getTrivialMethodType(AST::FunctionType functionType);
			
			const AST::Type*
			getTemplatedMethodType(AST::FunctionType functionType);
			
			const AST::Type*
			getMethodType(AST::FunctionType functionType);
			
			const AST::Type*
			getInterfaceMethodType(AST::FunctionType functionType);
			
			const AST::Type*
			getStaticInterfaceMethodType(AST::FunctionType functionType);
			
		private:
			Context& context_;
			const AST::Type* cachedVoidType_;
			const AST::Type* cachedBoolType_;
			const AST::Type* cachedIntType_;
			const AST::Type* cachedSizeType_;
			const AST::Type* cachedTypenameType_;
			const AST::Type* cachedMovableType_;
			
		};
		
	}
	
}

#endif
