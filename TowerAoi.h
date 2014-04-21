#ifndef __TOWERAOI_H
#define __TOWERAOI_H
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <vector>
#include <map>
#include <set>
using namespace std;

//Warning: duplicate defined with AllWorld solution. markbyxds 
//------------------------------------------------------------
#ifdef _MSC_VER
typedef signed __int64 int64;
typedef signed __int32 int32;
typedef signed __int16 int16;
typedef signed __int8 int8;

typedef unsigned __int64 uint64;
typedef unsigned __int32 uint32;
typedef unsigned __int16 uint16;
typedef unsigned __int8 uint8;

typedef volatile long atomic_t;
#else
#include <stdint.h>
typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;
typedef uint32_t DWORD;
typedef volatile int32 atomic_t;
#endif
#define ASSERT(x) assert(x)
struct Location {
	uint16 x;
	uint16 y;
	Location(uint16 x, uint16 y): x(x), y(y) {}
};
//------------------------------------------------------------

namespace TowerAoi
{
	struct Region {
		Location startPos;
		Location endPos;
		Region(uint16 startX, uint16 startY, uint16 endX, uint16 endY): startPos(startX,startY), endPos(endX,endY) {}
		Region(): startPos(0,0), endPos(0,0) {}
	};

	template<class T>
	class Tower
	{
	public:
		typedef set<T*> ObjectSet;
		typedef map<uint8, set<T*> > TypeToObjectSet;
		typedef vector<uint8> TypeVector;

		Tower(uint32 id, uint16 x, uint16 y): m_pos(x,y), m_id(id) {}
		~Tower() {}

		uint32 GetId() { return m_id; }

		bool Add(T* obj) {
			ASSERT(obj);
			pair<typename ObjectSet::iterator,bool> rt = m_objSet.insert(obj);
			printf(rt.second ? "obj %d added to tower %d\n" : "obj %d already in tower %d\n", obj->GetId(), GetId());
			m_typeToObjSet[obj->GetType()].insert(obj);
			return true;
		}

		bool Remove(T* obj) {
			ASSERT(obj);
			uint32 rt = m_objSet.erase(obj);
			printf(rt>0 ? "obj %d removed from tower %d\n" : "to remove obj %d not found in tower %d\n", obj->GetId(), GetId());
			typename TypeToObjectSet::iterator it = m_typeToObjSet.find(obj->GetType());
			if (it != m_typeToObjSet.end()) {
				it->second.erase(obj);
			}
			return true;
		}

		bool AddWatcher(T* obj) {
			pair<typename ObjectSet::iterator,bool> rt = m_typeToWatcherSet[obj->GetType()].insert(obj);
			printf(rt.second ? "watcher %d added to tower %d\n" : "watcher %d already in tower %d\n", obj->GetId(), GetId());
			return true;
		}

		bool RemoveWatcher(T* obj) {
			typename TypeToObjectSet::iterator it = m_typeToWatcherSet.find(obj->GetType());
			if (it != m_typeToWatcherSet.end()) {
				int rt = it->second.erase(obj);
				printf(rt>0 ? "watcher %d removed from tower %d\n" : 
					"to remove watcher %d not found in tower %d\n",  obj->GetId(), GetId());
			} else {
				printf("to remove watcher %d not found in tower %d\n", obj->GetId(), GetId());
			}
			return true;
		}

		const ObjectSet& GetObjects() { return m_objSet; }
		const TypeToObjectSet& GetWatchers() { return m_typeToWatcherSet; }

		void GetObjsByTypes(TypeToObjectSet& objs, const TypeVector& types) {
			ASSERT(m_typeToObjSet.size() == 0);
			for (TypeVector::const_iterator it = types.begin(); it != types.end(); it++) {
				typename TypeToObjectSet::iterator mapIt = m_typeToObjSet.find(*it);
				if (mapIt != m_typeToObjSet.end()) {
					typename TypeToObjectSet::iterator tmpIt = objs.find(*it);
					if (tmpIt == m_typeToObjSet.end()){
						objs[*it] = m_typeToObjSet[*it];
					}else {
						for (typename ObjectSet::iterator objIt = mapIt->second.begin(); objIt != mapIt->second.end(); objIt++) {
							objs[*it].insert(*objIt);
						}
					}
				}
			}
		}

		void GetWatchers(TypeToObjectSet& watchers, const TypeVector& types) {
			for (TypeVector::const_iterator it = types.begin(); it != types.end(); it++) {
				typename TypeToObjectSet::iterator mapIt = m_typeToWatcherSet.find(*it);
				if (mapIt != m_typeToWatcherSet.end()) {
					typename TypeToObjectSet::iterator tmpIt = watchers.find(*it);
					if (tmpIt == watchers.end()) {
						watchers[*it] = m_typeToWatcherSet[*it];
					}else {
						for (typename ObjectSet::iterator objIt = mapIt->second.begin(); objIt != mapIt->second.end(); ++objIt) {
							watchers[*it].insert(*objIt);
						}
					}
				}
			}
		}

		bool CheckConsistency(){
			ObjectSet allObjs;
			uint32 objSize = 0;
			for (typename TypeToObjectSet::const_iterator it = m_typeToObjSet.begin(); it != m_typeToObjSet.end(); it++) {
				objSize += it->second.size();
				for (typename ObjectSet::iterator setIt = it->second.begin(); setIt != it->second.end(); setIt++) {
					allObjs.insert(*setIt);
				}
			}
			ASSERT(allObjs.size() == objSize);
			ASSERT(objSize == m_objSet.size());
			for (typename ObjectSet::const_iterator it = m_objSet.begin(); it != m_objSet.end(); it++) {
				ASSERT(allObjs.find(*it) != allObjs.end());
			}
			return true;
		}

		Location GetPos() { return m_pos; }

	private:
		ObjectSet m_objSet;
		TypeToObjectSet m_typeToObjSet;
		TypeToObjectSet m_typeToWatcherSet;
		Location m_pos;
		uint32 m_id;
	};

	template<class T>
	class TowerAoi
	{
	public:
		typedef set<T*> ObjectSet;
		typedef map<uint8, set<T*> > TypeToObjectSet;
		typedef vector<uint8> TypeVector;

		typedef void (*FuncObjectAdded)(T* appearObj, const TypeToObjectSet& watchers);
		typedef void (*FuncObjectRemoved)(T* disappearObj, const TypeToObjectSet& watchers);
		typedef void (*FuncObjectMoved)(T* obj, const TypeToObjectSet& oldWatchers, const TypeToObjectSet& newWatchers);
		typedef void (*FuncWatcherMoved)(T* watcher, const ObjectSet& appearObjs, const ObjectSet& disappearObjs);

		TowerAoi(uint32 width, uint32 height, uint32 towerWidth, uint32 towerHeight):
		m_width(width), m_height(height), m_towerWidth(towerWidth), m_towerHeight(towerHeight)
		{
			m_towerX = (uint32)ceil((float)m_width/m_towerWidth);
			m_towerY = (uint32)ceil((float)m_height/m_towerHeight);
			m_towers = new Tower<T>**[m_towerX];
			for (uint32 x = 0; x < m_towerX; x++) {
				m_towers[x] = new Tower<T>*[m_towerY];
				for (uint32 y = 0; y < m_towerY; y++) {
					m_towers[x][y] = new Tower<T>(x*m_towerY+y, x, y);
				}
			}
		}
		~TowerAoi() {
			if (m_towers) {
				for (uint32 x = 0; x < m_towerX; x++) {
					for (uint32 y = 0; y < m_towerY; y++) {
						delete m_towers[x][y];
					}
					delete [] m_towers[x];
				}
				delete [] m_towers;
			}
		}

		void GetObjectsByPos(ObjectSet& objSet, const Location& pos, uint8 range) {
			if (!CheckPos(pos) || !CheckRange(range)) {
				printf("pos(%d,%d) or range %d invalid\n", pos.x, pos.y, range);
				return;
			}
			Location towerPos = TranslatePos(pos);
			Region region = GetRegion(towerPos, range);
			for (uint16 x = region.startPos.x; x <= region.endPos.x; x++) {
				for (uint16 y = region.startPos.y; y <= region.endPos.y; y++) {
					const ObjectSet& objs = m_towers[x][y]->GetObjects();
					for (typename ObjectSet::const_iterator it = objs.begin(); it != objs.end(); it++) {
						objSet.insert(*it);
					}
				}
			}
		}

		void GetObjectsByTypes(TypeToObjectSet& typeToObjSet, const Location& pos, uint8 range, const TypeVector& types) {
			if (!CheckPos(pos) || !CheckRange(range)) {
				printf("pos(%d,%d) or range %d invalid\n", pos.x, pos.y, range);
				return;
			}
			Location towerPos = TranslatePos(pos);
			Region region = GetRegion(towerPos, range);
			for (uint16 x = region.startPos.x; x <= region.endPos.x; x++) {
				for (uint16 y = region.startPos.y; y <= region.endPos.y; y++) {
					m_towers[x][y]->GetObjsByTypes(typeToObjSet, types);
				}
			}
		}

		bool AddObject(T* obj, const Location& pos) {
			if (!CheckPos(pos)) return false;
			Location towerPos = TranslatePos(pos);
			bool rt = m_towers[towerPos.x][towerPos.y]->Add(obj);
			if (rt)
				m_funcObjAdded(obj, m_towers[towerPos.x][towerPos.y]->GetWatchers());
			return rt;
		}

		bool RemoveObject(T* obj, const Location& pos) {
			if (!CheckPos(pos)) return false;
			Location towerPos = TranslatePos(pos);
			bool rt = m_towers[towerPos.x][towerPos.y]->Remove(obj);
			if (rt) 
				m_funcObjRemoved(obj, m_towers[towerPos.x][towerPos.y]->GetWatchers());
			return rt;
		}

		bool UpdateObject(T* obj, const Location& oldPos, const Location& newPos) {
			if (!CheckPos(oldPos) || !CheckPos(newPos)) return false;
			Location oldTower = TranslatePos(oldPos);
			Location newTower = TranslatePos(newPos);
			if (oldTower.x!=newTower.x || oldTower.y!=newTower.y) {
				m_towers[oldTower.x][oldTower.y]->Remove(obj);
				m_towers[newTower.x][newTower.y]->Add(obj);
			}
			m_funcObjMoved(obj, m_towers[oldTower.x][oldTower.y]->GetWatchers(), m_towers[newTower.x][newTower.y]->GetWatchers());
			return true;
		}

		bool GetWatchers(TypeToObjectSet& watchers, const Location& pos, const TypeVector& types) {
			if(!CheckPos(pos)) return false;
			Location towerPos = TranslatePos(pos);
			m_towers[towerPos.x][towerPos.y]->GetWatchers(watchers, types);
			return true;
		}

		bool AddWatcher(T* watcher, const Location& pos, uint8 range){
			if (!CheckPos(pos) || !CheckRange(range)) return false;
			Location towerPos = TranslatePos(pos);
			Region region = GetRegion(towerPos, range);
			ObjectSet addedObjs;
			ObjectSet removedObjs;
			for (uint16 x = region.startPos.x; x <= region.endPos.x; x++) {
				for (uint16 y = region.startPos.y; y <= region.endPos.y; y++) {
					m_towers[x][y]->AddWatcher(watcher);
					const ObjectSet& objSet = m_towers[x][y]->GetObjects();
					for (typename ObjectSet::const_iterator objIt = objSet.begin(); objIt != objSet.end(); objIt++) {
						addedObjs.insert(*objIt);
					}
				}
			}
			//call back
			m_funcWatcherMoved(watcher, addedObjs, removedObjs);
			return true;
		}

		bool RemoveWatcher(T* watcher, const Location& pos, uint8 range) { //watcher pos/range should recorded and verified internal. markbyxds 
			if (!CheckPos(pos) || !CheckRange(range)) return false;
			Location towerPos = TranslatePos(pos);
			Region region = GetRegion(towerPos, range);
			ObjectSet addedObjs;
			ObjectSet removedObjs;
			for (uint16 x = region.startPos.x; x <= region.endPos.x; x++) {
				for (uint16 y = region.startPos.y; y <= region.endPos.y; y++) {
					m_towers[x][y]->RemoveWatcher(watcher);
					const ObjectSet& objSet = m_towers[x][y]->GetObjects();
					for (typename ObjectSet::const_iterator objIt = objSet.begin(); objIt != objSet.end(); objIt++) {
						removedObjs.insert(*objIt);
					}
				}
			}
			//call back
			m_funcWatcherMoved(watcher, addedObjs, removedObjs);
			return true;
		}

		bool UpdateWatcher(T* watcher, const Location& oldPos, uint8 oldRange, const Location& newPos, uint8 newRange) {
			if (!CheckPos(oldPos) || !CheckPos(newPos) || !CheckRange(oldRange) || !CheckRange(newRange))
				return false;
			Location oldTower = TranslatePos(oldPos);
			Location newTower = TranslatePos(newPos);
			if (oldTower.x==newTower.x && oldTower.y==newTower.y && oldRange==newRange) {
				return true;
			}
			//key: 0->addedTowers 1->removedTowers 2->unchangeTowers
			map<uint8,vector<Tower<T>*> > towers;
			GetChangedTowers(towers, oldTower, newTower, oldRange, newRange);
			//add watcher to towers; get addedObjs for watcher
			ObjectSet addedObjs;
			for (typename vector<Tower<T>*>::const_iterator it = towers[0].begin(); it != towers[0].end(); it++) {
				(*it)->AddWatcher(watcher);
				const ObjectSet& objSet = (*it)->GetObjects();
				for (typename ObjectSet::const_iterator objIt = objSet.begin(); objIt != objSet.end(); objIt++) {
					addedObjs.insert(*objIt);
				}
			}
			//remove watcher from towers; get removedObjs for watcher
			ObjectSet removedObjs;
			for (typename vector<Tower<T>*>::const_iterator it = towers[1].begin(); it != towers[1].end(); it++) {
				(*it)->RemoveWatcher(watcher);
				const ObjectSet& objSet = (*it)->GetObjects();
				for (typename ObjectSet::const_iterator objIt = objSet.begin(); objIt != objSet.end(); objIt++) {
					removedObjs.insert(*objIt);
				}
			}
			//call back
			m_funcWatcherMoved(watcher, addedObjs, removedObjs);
			return true;
		}

		void RegisterCallBackFunc(FuncObjectAdded func1, FuncObjectRemoved func2, FuncObjectMoved func3, FuncWatcherMoved func4) {
			m_funcObjAdded = func1;
			m_funcObjRemoved = func2;
			m_funcObjMoved = func3;
			m_funcWatcherMoved = func4;
		}

		bool CheckConsistency() {
			map<T*, set<Tower<T>*> > watcherToTower;
			for (uint32 x = 0; x < m_towerX; x++) {
				for (uint32 y = 0; y < m_towerY; y++) {
					ASSERT(m_towers[x][y]->CheckConsistency());
					const TypeToObjectSet& watcherMap = m_towers[x][y]->GetWatchers();
					for (typename TypeToObjectSet::const_iterator mapIt = watcherMap.begin(); mapIt != watcherMap.end(); mapIt++) {
						const ObjectSet& watcherSet = mapIt->second;
						for (typename ObjectSet::const_iterator it = watcherSet.begin(); it != watcherSet.end(); it++) {
							watcherToTower[*it].insert(m_towers[x][y]);
						}
					}					
				}
			}
			for (typename map<T*, set<Tower<T>*> >::iterator it = watcherToTower.begin(); it != watcherToTower.end(); it++) {
				ASSERT(it->second.size() == 49); //markbyxds 
				Location towerPos = TranslatePos(it->first->m_pos);
				Region region = GetRegion(towerPos, 3); // markbyxds 
				for (typename set<Tower<T>*>::iterator towerIt = it->second.begin(); towerIt != it->second.end(); towerIt++) {
					ASSERT(IsInRect((*towerIt)->GetPos(), region.startPos, region.endPos));
				}
			}
			return true;
		}

	private:
		void GetChangedTowers(map<uint8,vector<Tower<T>*> >& towers, const Location& oldPos, const Location& newPos, uint8 oldRange, uint8 newRange) {
			Region oldRegion = GetRegion(oldPos, oldRange);
			Region newRegion = GetRegion(newPos, newRange);
			//key: 0->addedTowers 1->removedTowers 2->unchangeTowers
			for (uint16 x = oldRegion.startPos.x; x <= oldRegion.endPos.x; x++) {
				for (uint16 y = oldRegion.startPos.y; y <= oldRegion.endPos.y; y++) {
					if (IsInRect(Location(x,y), newRegion.startPos, newRegion.endPos)) {
						towers[2].push_back(m_towers[x][y]);
					}else {
						towers[1].push_back(m_towers[x][y]);
					}
				}
			}
			for (uint16 x = newRegion.startPos.x; x <= newRegion.endPos.x; x++) {
				for (uint16 y = newRegion.startPos.y; y <= newRegion.endPos.y; y++) {
					if (!IsInRect(Location(x,y), oldRegion.startPos, oldRegion.endPos)) {
						towers[0].push_back(m_towers[x][y]);
					}
				}
			}
		}

		bool CheckPos(const Location& pos) const{
			if (pos.x<0 || pos.y<0 || pos.x>=m_width || pos.y>=m_height) {
				printf("pos(%d,%d) is invalid\n", pos.x, pos.y);
				return false;
			}
			return true;
		}

		bool CheckRange(uint8 range) const{
			if (range<=0 || range>RANGE_LIMIT) {
				printf("range %d is invalid\n", range);
				return false;
			}
			return true;
		}

		bool IsInRect(const Location& pos, const Location& startPos, const Location& endPos) const{
			return pos.x>=startPos.x && pos.x<=endPos.x && pos.y>=startPos.y && pos.y<=endPos.y;
		}

		Location TranslatePos(const Location& pos) const{
			return Location(floor((float)pos.x/m_towerWidth),floor((float)pos.y/m_towerHeight));
		}

		Region GetRegion(const Location& pos, uint8 range) {
			Region region;
			if (pos.x - range < 0) {
				region.startPos.x = 0;
				region.endPos.x = 2*range;
			}else if (pos.x + range >= m_towerX) {
				region.endPos.x = m_towerX - 1;
				region.startPos.x = m_towerX - 1 - 2*range;
			}else {
				region.startPos.x = pos.x - range;
				region.endPos.x = pos.x + range;
			}
			if (pos.y - range < 0) {
				region.startPos.y = 0;
				region.endPos.y = 2*range;
			}else if (pos.y + range >= m_towerY) {
				region.endPos.y = m_towerY - 1;
				region.startPos.y = m_towerY - 1 - 2*range;
			}else {
				region.startPos.y = pos.y - range;
				region.endPos.y = pos.y + range;
			}
			region.startPos.x = region.startPos.x>=0?region.startPos.x:0;
			region.endPos.x = region.endPos.x<m_towerX?region.endPos.x:m_towerX-1;
			region.startPos.y = region.startPos.y>=0?region.startPos.y:0;
			region.endPos.y = region.endPos.y<m_towerY?region.endPos.y:m_towerY-1;
			return region;
		}

	private:
		uint32 m_width;
		uint32 m_height;
		uint32 m_towerWidth;
		uint32 m_towerHeight;
		uint32 m_towerX;
		uint32 m_towerY;
		Tower<T>*** m_towers;
		static const uint8 RANGE_LIMIT = 5;

		FuncObjectAdded m_funcObjAdded;
		FuncObjectRemoved m_funcObjRemoved;
		FuncObjectMoved m_funcObjMoved;
		FuncWatcherMoved m_funcWatcherMoved;
	};
}

#endif