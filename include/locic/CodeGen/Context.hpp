#ifndef LOCIC_CODEGEN_CONTEXT_HPP
#define LOCIC_CODEGEN_CONTEXT_HPP

#include <memory>

namespace locic {
	
	class StringHost;
	
	namespace CodeGen {
		
		class InternalContext;
		
		class Context {
			public:
				Context(const StringHost& stringHost);
				~Context();
				
				InternalContext& internal();
				
				const InternalContext& internal() const;
				
			private:
				std::unique_ptr<InternalContext> internalContext_;
				
		};
		
	}
	
}

#endif
