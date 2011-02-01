#ifndef LOCIC_CLASS_H
#define LOCIC_CLASS_H

#include <string>
#include "Type.h"

namespace Locic{

	class Class{
		public:
			virtual std::string Name() = 0;

			virtual void FillType(Type *) = 0;

			virtual void Generate() = 0;

	};

}

#endif
