#ifndef LOCIC_SEM_MODULE_HPP
#define LOCIC_SEM_MODULE_HPP

#include <memory>
#include <string>

namespace locic {
	
	namespace AST {
		
		class Namespace;
		
	}
	
	namespace SEM {
		
		class Context;
		
		class Module {
			public:
				Module(Context& context);
				~Module();
				
				Module(Module&&) = default;
				Module& operator=(Module&&) = default;
				
				Context& context();
				const Context& context() const;
				
				AST::Namespace& rootNamespace();
				const AST::Namespace& rootNamespace() const;
				
				std::string toString() const;
				
			private:
				Context& context_;
				std::unique_ptr<AST::Namespace> rootNamespace_;
				
		};
		
	}
	
}

#endif
