#ifndef LOCIC_BUILDOPTIONS_HPP
#define LOCIC_BUILDOPTIONS_HPP

namespace locic{

	class BuildOptions {
	public:
		bool unsafe;
		
		BuildOptions()
			: unsafe(false) { }
	};

}

#endif
