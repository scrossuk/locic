#ifndef LOCIC_IF_H
#define LOCIC_IF_H

#include <string>
#include "Tree.h"
#include "Type.h"

namespace Locic{

	class If: public Tree{
		public:
			inline If(Tree * condition, Tree * ifTrue, Tree * ifFalse){
				mCondition = condition;
				mIfTrue = ifTrue;
				mIfFalse = ifFalse;
			}

			inline TreeResult Generate(Resolver * resolver){
				TreeResult condRes = mCondition->Generate(resolver);
				TreeResult ifTrueRes = mIfTrue->Generate(resolver);
				TreeResult ifFalseRes = mIfFalse->Generate(resolver);

				std::string s = "(Loci_TestBool(";
				s += condRes.str;
				s += ")) ? (";
				s += ifTrueRes.str;
				s += ") : (";
				s += ifFalseRes.str;
				s += ")";
				return TreeResult(ifTrueRes.type, s);
			}

		protected:
			Tree * mCondition;
			Tree * mIfTrue;
			Tree * mIfFalse;

	};

}

#endif
