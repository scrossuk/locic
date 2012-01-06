#include <assert.h>
#include <map>
#include <string>

#include <Locic/StringMap.h>

typedef std::map<std::string, void*> MapType;
typedef MapType::iterator IteratorType;
typedef std::pair<std::string, void*> PairType;

extern "C" {

	void* Locic_StringMap_Alloc() {
		return new MapType();
	}
	
	void Locic_StringMap_Free(void* stringMap) {
		assert(stringMap != NULL);
		delete reinterpret_cast<MapType*>(stringMap);
	}
	
	void Locic_StringMap_Clear(void * stringMap){
		assert(stringMap != NULL);
		MapType* map = reinterpret_cast<MapType*>(stringMap);
		map->clear();
	}
	
	void* Locic_StringMap_Find(void* stringMap, const char* str) {
		assert(stringMap != NULL);
		MapType* map = reinterpret_cast<MapType*>(stringMap);
		IteratorType it = map->find(str);
		
		if(it == map->end()) {
			return NULL;
		} else {
			return it->second;
		}
	}
	
	// Return value is existing value in map, which if not NULL indicates the insert failed.
	void* Locic_StringMap_Insert(void* stringMap, const char* str, void* data) {
		assert(stringMap != NULL);
		MapType* map = reinterpret_cast<MapType*>(stringMap);
		std::pair<IteratorType, bool> ret = map->insert(PairType(str, data));
		
		if(!ret.second) {
			return ret.first->second;
		} else {
			// Indicates successful insert.
			return NULL;
		}
	}
	
	void* Locic_StringMap_Erase(void* stringMap, const char* str) {
		assert(stringMap != NULL);
		MapType* map = reinterpret_cast<MapType*>(stringMap);
		IteratorType it = map->find(str);
		
		if(it == map->end()) {
			return NULL;
		} else {
			void* val = it->second;
			map->erase(it);
			return val;
		}
	}
	
}

