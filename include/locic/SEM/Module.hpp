#ifndef LOCIC_SEM_MODULE_HPP
#define LOCIC_SEM_MODULE_HPP

#include <memory>
#include <string>

namespace locic {
	
	namespace AST {
		
		class Context;
		class Namespace;
		
	}
	
	namespace SEM {
		
		class Module {
			public:
				Module(AST::Context& context);
				~Module();
				
				Module(Module&&) = default;
				Module& operator=(Module&&) = default;
				
				AST::Context& context();
				const AST::Context& context() const;
				
				AST::Namespace& rootNamespace();
				const AST::Namespace& rootNamespace() const;
				
				std::string toString() const;
				
			private:
				AST::Context& context_;
				std::unique_ptr<AST::Namespace> rootNamespace_;
				
		};
		
	}
	
}

#endif
