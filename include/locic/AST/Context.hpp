#ifndef LOCIC_AST_CONTEXT_HPP
#define LOCIC_AST_CONTEXT_HPP

#include <memory>

namespace locic {
	
	class PrimitiveID;
	
	namespace AST {
		
		class FunctionType;
		class FunctionTypeData;
		class MethodSet;
		class Type;
		class TypeInstance;
		
		class Context {
			public:
				Context();
				~Context();
				
				FunctionType
				getFunctionType(FunctionTypeData functionType) const;
				
				const Type* getType(Type&& type) const;
				
				void setPrimitive(PrimitiveID primitiveID,
				                  const TypeInstance& typeInstance);
				
				const TypeInstance&
				getPrimitive(PrimitiveID primitiveID) const;
				
				const MethodSet*
				getMethodSet(MethodSet methodSet) const;
				
			private:
				// Non-copyable.
				Context(const Context&) = delete;
				Context& operator=(const Context&) = delete;
				
				std::unique_ptr<class ContextImpl> impl_;
				
		};
		
	}
	
}

#endif
