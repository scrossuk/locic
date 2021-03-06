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
			                 AST::ValueArray templateArguments = {});
			
			const AST::Type*
			getVoidType();
			
			const AST::Type*
			getBoolType();
			
			const AST::Type*
			getIntType();
			
			const AST::Type*
			getSizeType();
			
			/**
			 * \brief Get abstracttypename_t.
			 */
			const AST::Type*
			getAbstractTypenameType();
			
			/**
			 * \brief Get typename_t<none_t>.
			 */
			const AST::Type*
			getNoneTypenameType();
			
			/**
			 * \brief Get typename_t<T>.
			 */
			const AST::Type*
			getTypenameType(const AST::Type* type);
			
			const AST::Type*
			getDestructibleInterfaceType();
			
			const AST::Type*
			getMovableInterfaceType(const AST::Type* type);
			
			const AST::Type*
			getRefType(const AST::Type* targetType);
			
			const AST::Type*
			getPointerType(const AST::Type* elementType);
		
			const AST::Type*
			getConstantStaticArrayType(const AST::Type* elementType,
			                           size_t arraySize,
			                           const Debug::SourceLocation& location);
		
			const AST::Type*
			getStaticArrayType(const AST::Type* elementType,
			                   AST::Value arraySize,
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
			
		};
		
	}
	
}

#endif
