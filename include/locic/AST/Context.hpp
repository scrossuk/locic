#ifndef LOCIC_AST_CONTEXT_HPP
#define LOCIC_AST_CONTEXT_HPP

#include <memory>

namespace locic {
	
	class PrimitiveID;
	
	namespace SEM {
		
		class TypeInstance;
		
	}
	
	namespace AST {
		
		class FunctionType;
		class FunctionTypeData;
		class Type;
		
		class Context {
			public:
				Context();
				~Context();
				
				FunctionType
				getFunctionType(FunctionTypeData functionType) const;
				
				const Type* getType(Type&& type) const;
				
				void setPrimitive(PrimitiveID primitiveID,
				                  const SEM::TypeInstance& typeInstance);
				
				const SEM::TypeInstance&
				getPrimitive(PrimitiveID primitiveID) const;
				
			private:
				// Non-copyable.
				Context(const Context&) = delete;
				Context& operator=(const Context&) = delete;
				
				std::unique_ptr<class ContextImpl> impl_;
				
		};
		
	}
	
}

#endif
