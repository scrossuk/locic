#ifndef LOCIC_BUILDOPTIONS_HPP
#define LOCIC_BUILDOPTIONS_HPP

namespace locic{

	struct BuildOptions {
		bool unsafe;
		
		inline BuildOptions()
			: unsafe(false) { }
	};

}

#endif
