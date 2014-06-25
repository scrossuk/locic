#ifndef LLVMABI_CONTEXT_HPP
#define LLVMABI_CONTEXT_HPP

#include <set>

#include <llvm-abi/Type.hpp>

namespace llvm_abi {

	/**
	 * \brief ABI Context
	 */
	class Context {
		public:
			Context();
			
			Type* getType(Type type);
			
		private:
			// Non-copyable.
			Context(const Context&) = delete;
			Context& operator=(const Context&) = delete;
			
			std::set<Type> types_;
			
	};
	
}

#endif
