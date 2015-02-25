#ifndef LOCIC_EXCEPTION_HPP
#define LOCIC_EXCEPTION_HPP

#include <string>

namespace locic{

	class Exception {
		public:
			virtual std::string toString() const = 0;
		
	};

}

#endif
