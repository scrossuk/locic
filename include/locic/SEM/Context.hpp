#ifndef LOCIC_SEM_CONTEXT_HPP
#define LOCIC_SEM_CONTEXT_HPP

#include <memory>

namespace locic {

	namespace SEM {
	
		class Namespace;
		class Type;
		
		class Context {
			public:
				Context();
				~Context();
				
				const Type* getType(Type type) const;
				
				Namespace* rootNamespace();
				
			private:
				// Non-copyable.
				Context(const Context&) = delete;
				Context& operator=(const Context&) = delete;
				
				std::unique_ptr<class ContextImpl> impl_;
				
		};
		
	}
	
}

#endif
