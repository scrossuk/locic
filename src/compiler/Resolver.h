#ifndef LOCIC_RESOLVER_H
#define LOCIC_RESOLVER_H

#include <vector>
#include <map>
#include <string>

namespace Locic{

	class InternalClass;

	class Resolver{
		public:
			inline Resolver(InternalClass * thisInternalClass){
				mThisInternalClass = thisInternalClass;
				mNextID = 1;
				PushScope();
			}

			inline InternalClass * This(){
				return mThisInternalClass;
			}

			inline void PushScope(){
				std::map<std::string, Type *> map;
				std::map<std::string, unsigned int> idMap;
				mScopes.push_back(map);
				mScopeIDs.push_back(idMap);
			}

			inline void PopScope(){
				mScopes.pop_back();
				mScopeIDs.pop_back();
			}

			inline unsigned int GetNumVars(){
				return mNextID - 1;
			}

			inline unsigned int DeclareVar(const std::string& name, Type * type){
				if(FindVar(name) != 0){
					throw "Resolver -- Local var already exists";
				}
				(mScopes.back())[name] = type;
				(mScopeIDs.back())[name] = mNextID;
				return mNextID++;
			}

			inline Type * GetVar(const std::string& name){
				Type * type = FindVar(name);

				if(type == 0){
					throw std::string("Resolver -- Couldn't find local var: ") + name;
				}else{
					return type;
				}
			}

			inline Type * FindVar(const std::string& name){
				std::vector< std::map<std::string, Type *> >::reverse_iterator iter;
				for(iter = mScopes.rbegin(); iter != mScopes.rend(); ++iter){
					Type * type = (*iter)[name];
					if(type != 0){
						return type;
					}
				}

				return 0;
			}

			inline unsigned int GetVarID(const std::string& name){
				std::vector< std::map<std::string, unsigned int> >::reverse_iterator iter;
				for(iter = mScopeIDs.rbegin(); iter != mScopeIDs.rend(); ++iter){
					unsigned int id = (*iter)[name];
					if(id != 0){
						return id;
					}
				}

				return 0;
			}

		private:
			InternalClass * mThisInternalClass;

			unsigned int mNextID;

			std::vector< std::map<std::string, Type *> > mScopes;

			std::vector< std::map<std::string, unsigned int> > mScopeIDs;

	};

}

#endif
