#ifndef LOCIC_SEM_CONTEXT_HPP
#define LOCIC_SEM_CONTEXT_HPP

#include <memory>
#include <set>

#include <locic/SEM/Type.hpp>

namespace locic {

	namespace SEM {
	
		class Namespace;
		
		class Context {
			public:
				Context();
				
				Type* getType(Type type);
				
				SEM::Namespace* rootNamespace();
				
			private:
				// Non-copyable.
				Context(const Context&) = delete;
				Context& operator=(const Context&) = delete;
				
				std::unique_ptr<SEM::Namespace> rootNamespace_;
				std::set<Type> types_;
				
		};
		
	}
	
}

#endif
