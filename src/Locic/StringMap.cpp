#include <cassert>
#include <cstdlib>
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
	
	Locic_StringMapIterator Locic_StringMap_Begin(void * stringMap){
		assert(stringMap != NULL);
		MapType* map = reinterpret_cast<MapType*>(stringMap);
		return new IteratorType(map->begin());
	}

	void Locic_StringMap_Advance(void * iterator){
		assert(iterator != NULL);
		IteratorType* it = reinterpret_cast<IteratorType*>(iterator);
		(*it)++;
	}

	int Locic_StringMap_IsEnd(Locic_StringMap stringMap, Locic_StringMapIterator iterator){
		assert(stringMap != NULL);
		assert(iterator != NULL);
		MapType* map = reinterpret_cast<MapType*>(stringMap);
		IteratorType* it = reinterpret_cast<IteratorType*>(iterator);
		return (*it == map->end()) ? 1 : 0;
	}

	char * Locic_StringMap_GetStringKey(void * iterator){
		assert(iterator != NULL);
		IteratorType* it = reinterpret_cast<IteratorType*>(iterator);
		const std::string& str = (*it)->first;
		const std::size_t size = str.size();
		char * string = (char *) malloc(size + 1);
		str.copy(string, size);
		string[size] = '\0';
		return string;
	}

	void * Locic_StringMap_GetData(void * iterator){
		assert(iterator != NULL);
		IteratorType* it = reinterpret_cast<IteratorType*>(iterator);
		return (*it)->second;
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
	
	size_t Locic_StringMap_Size(void * stringMap){
		assert(stringMap != NULL);
		MapType* map = reinterpret_cast<MapType*>(stringMap);
		return map->size();
	}
	
}

