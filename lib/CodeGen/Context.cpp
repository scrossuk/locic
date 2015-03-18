#include <locic/CodeGen/Context.hpp>
#include <locic/CodeGen/InternalContext.hpp>

namespace locic {
	
	namespace CodeGen {
		
		Context::Context(const StringHost& stringHost, const TargetOptions& targetOptions)
		: internalContext_(new InternalContext(stringHost, targetOptions)) { }
		
		Context::~Context() { }
		
		InternalContext& Context::internal() {
			return *internalContext_;
		}
		
		const InternalContext& Context::internal() const {
			return *internalContext_;
		}
		
	}
	
}

