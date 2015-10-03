#ifndef LOCIC_CODEGEN_PRIMITIVEMAP_HPP
#define LOCIC_CODEGEN_PRIMITIVEMAP_HPP

#include <memory>
#include <unordered_map>

#include <locic/Support/PrimitiveID.hpp>

namespace locic {
	
	namespace SEM {
		
		class TypeInstance;
		
	}
	
	namespace CodeGen {
		
		class Primitive;
		
		class PrimitiveMap {
		public:
			PrimitiveMap();
			~PrimitiveMap();
			
			const Primitive& getPrimitive(const SEM::TypeInstance& typeInstance) const;
			
		private:
			// TODO: avoid heap allocations here.
			mutable std::unordered_map<const SEM::TypeInstance*, std::unique_ptr<Primitive>> primitives_;
			
		};
		
	}
	
}

#endif
