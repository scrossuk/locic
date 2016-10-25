#ifndef LOCIC_CODEGEN_BUILDOPTIONS_HPP
#define LOCIC_CODEGEN_BUILDOPTIONS_HPP

namespace locic{
	
	namespace CodeGen {
		
		class BuildOptions {
		public:
			bool unsafe;
			bool zeroAllAllocas;
			
			BuildOptions()
			: unsafe(false),
			zeroAllAllocas(false) { }
		};
		
	}
	
}

#endif
