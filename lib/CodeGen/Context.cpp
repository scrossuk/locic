#include <locic/CodeGen/Context.hpp>
#include <locic/CodeGen/InternalContext.hpp>

namespace locic {

	namespace CodeGen {
		
		Context::Context()
		: internalContext_(new InternalContext()) { }
		
		Context::~Context() { }
		
		InternalContext& Context::internal() {
			return *internalContext_;
		}
		
		const InternalContext& Context::internal() const {
			return *internalContext_;
		}
		
	}
	
}

