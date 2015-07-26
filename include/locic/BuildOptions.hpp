#ifndef LOCIC_BUILDOPTIONS_HPP
#define LOCIC_BUILDOPTIONS_HPP

namespace locic{

	class BuildOptions {
	public:
		bool unsafe;
		bool zeroAllAllocas;
		
		BuildOptions()
		: unsafe(false),
		zeroAllAllocas(false) { }
	};

}

#endif
