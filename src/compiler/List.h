#ifndef LOCIC_LIST_H
#define LOCIC_LIST_H

#include <vector>

namespace Locic{

	template <class T>
	class List{
		public:
			List(){ }

			List(T * t){
				mList.push_back(t);
			}

			List<T> * Add(T * t){
				mList.push_back(t);
				return this;
			}

			T * Get(unsigned int i){
				return mList[i];
			}

			unsigned int Size(){
				return mList.size();
			}

			bool IsEqual(List<T> * list){
				if(list->Size() != Size()){
					return false;
				}

				for(unsigned int i = 0; i < list->Size(); ++i){
					if(list->Get(i) != Get(i)){
						return false;
					}
				}

				return true;
			}

		private:
			std::vector<T *> mList;

	};

}

#endif
