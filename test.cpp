#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "TowerAoi.h"
using namespace std;

static const uint32 WATCHER_NUM = 10;
static const uint32 OBJECT_NUM = 50;
static const uint32 MAP_X = 1000;
static const uint32 MAP_Y = 1000;
static const uint32 TOWER_X = 100;
static const uint32 TOWER_Y = 100;
static const uint8 TYPE_NUM = 3;
static const uint8 RANGE_TOWER = 3;

void Log(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	printf("\n");
	va_end(ap);
};

struct Object {
	uint32 m_id;
	uint8 m_type;
	Location m_pos;
	bool m_isWatcher;

	Object(uint32 id, uint8 type): m_id(id), m_type(type), m_pos(0,0), m_isWatcher(false) {}
	uint32 GetId() { return m_id; }
	uint8 GetType() { return m_type; }
	void SetPos(const Location& pos) { m_pos = pos; }

};

Location GetRandLocaion()
{
	return Location(rand()%MAP_X, rand()%MAP_Y);
}

void ObjectAdded(Object* appearObj, const map<uint8, set<Object*> >& watchers)
{
	for (map<uint8, set<Object*> >::const_iterator it = watchers.begin(); it != watchers.end(); it++) {
		const set<Object*>& objSet = it->second;
		for (set<Object*>::const_iterator objIt = objSet.begin(); objIt != objSet.end(); ++objIt) {
			printf("%d: i saw %d apear\n", (*objIt)->GetId(), appearObj->GetId());
		}
	}
}

void ObjectRemoved(Object* disappearObj, const map<uint8, set<Object*> >& watchers)
{
	for (map<uint8, set<Object*> >::const_iterator it = watchers.begin(); it != watchers.end(); it++) {
		const set<Object*>& objSet = it->second;
		for (set<Object*>::const_iterator objIt = objSet.begin(); objIt != objSet.end(); ++objIt) {
			printf("%d: i saw %d disapear\n", (*objIt)->GetId(), disappearObj->GetId());
		}
	}
}

void ObjectMoved(Object* obj, const map<uint8, set<Object*> >& oldWatchers, const map<uint8, set<Object*> >& newWatchers)
{
	for (map<uint8, set<Object*> >::const_iterator it = oldWatchers.begin(); it != oldWatchers.end(); it++) {
		const set<Object*>& objSet = it->second;
		for (set<Object*>::const_iterator objIt = objSet.begin(); objIt != objSet.end(); ++objIt) {
			printf("%d: i saw %d move start\n", (*objIt)->GetId(), obj->GetId());
		}
	}
	for (map<uint8, set<Object*> >::const_iterator it = newWatchers.begin(); it != newWatchers.end(); it++) {
		const set<Object*>& objSet = it->second;
		for (set<Object*>::const_iterator objIt = objSet.begin(); objIt != objSet.end(); ++objIt) {
			printf("%d: i saw %d move end\n", (*objIt)->GetId(), obj->GetId());
		}
	}
}

void WatcherMoved(Object* watcher, const set<Object*>& appearObjs, const set<Object*>& disappearObjs)
{
	for (set<Object*>::const_iterator objIt = disappearObjs.begin(); objIt != disappearObjs.end(); ++objIt) {
		printf("%d: i saw %d disapear when i move\n", watcher->GetId(), (*objIt)->GetId(), (*objIt)->GetType());
	}
	for (set<Object*>::const_iterator objIt = appearObjs.begin(); objIt != appearObjs.end(); ++objIt) {
		printf("%d: i saw %d apear when i move\n", watcher->GetId(), (*objIt)->GetId());
	}
}

void SplitString(vector<string>& strVector, const char* str)
{
	ASSERT(strlen(str) < 1024);
	static char buff[1024] = {0};
	const char* ptr = str;
	char* buffPtr = buff;
	for (const char* ptr = str; true; ptr++) {
		if (*ptr=='\n' || *ptr==' ' || *ptr=='\t' || *ptr=='\0') {
			*buffPtr = '\0';
			if (strlen(buff) != 0) {
				strVector.push_back(string(buff));
			}
			if (*ptr == '\0') break;
			else buffPtr = buff;
		}else {
			*buffPtr = *ptr;
			buffPtr++;
		}
	}
}

void AddRandomObject(map<uint32,Object*>& objs, TowerAoi::TowerAoi<Object>* aoi)
{
	uint32 maxId = 0;
	for (map<uint32,Object*>::iterator it = objs.begin(); it != objs.end(); it++) {
		if (it->first > maxId)
			maxId = it->first;
	}
	Object* obj = new Object(maxId + 1, rand()%TYPE_NUM);
	objs[obj->GetId()] = obj;
	Location pos = GetRandLocaion();
	obj->SetPos(pos);
	aoi->AddObject(obj, pos);
	if (rand()%2 == 1) {
		obj->m_isWatcher = true;
		aoi->AddWatcher(obj, pos, RANGE_TOWER);
	}
}

Object* GetRandomObject(const map<uint32,Object*>& objs)
{
	uint32 randIndex = rand()%objs.size();
	uint32 tmpIndex = 0;
	Object* obj = NULL;
	map<uint32, Object*>::const_iterator it = objs.begin();
	for (; it != objs.end(); it++) {
		if (tmpIndex == randIndex) {
			obj = it->second;
			break;
		}
	}
	return obj;
}

int main(int argc, char** argv)
{
	//constuct aoi instance
	TowerAoi::TowerAoi<Object>* aoi = new TowerAoi::TowerAoi<Object>(MAP_X, MAP_Y, TOWER_X, TOWER_Y);

	//register callback functions
	aoi->RegisterCallBackFunc(ObjectAdded, ObjectRemoved, ObjectMoved, WatcherMoved);

	srand(time(NULL));
	map<uint32, Object*> objs;
	for (uint32 i = 1; i <= OBJECT_NUM; i++) {
		//construct obj
		Object* obj = new Object(i, rand()%TYPE_NUM);
		objs[i] = obj;
		Location pos = GetRandLocaion();
		obj->SetPos(pos);
		//add as watcher
		if (i < WATCHER_NUM) {
			obj->m_isWatcher = true;
			aoi->AddWatcher(obj, pos, RANGE_TOWER);
			aoi->CheckConsistency();
		}
		//add as normal object
		aoi->AddObject(obj, pos);
		aoi->CheckConsistency();
	}

	//interactive test
	char command[1024] = {0};
	vector<string> strVec;
#define PROMPT {printf("invalid command.\n"); continue;}
	while (true) {
		strVec.clear();
		printf("**************************************\n");
		printf("* obj id 1~%d\n", objs.size());
		printf("* s objId --show object info: id, type, pos\n");
		printf("* m objId endx endy --move obj to pos(endx,endy)\n");
		printf("* c --check data consistency\n");
		printf("* a --add new object\n");
		printf("* d objId --remove object\n");
		printf("* p opNum --random operate aoi opNum times\n");
		printf("* Q --quit interactive mode\n");
		printf("**************************************\n");
		//scanf("%s", command);
		//gets(command);
		if (!fgets(command, 1024, stdin)) PROMPT;
		SplitString(strVec, command);
		if (strVec.size() < 1) PROMPT;
		if (strVec[0].compare("s") == 0){
			if (strVec.size() < 2) PROMPT;
			uint32 objId = atoi(strVec[1].c_str());
			map<uint32, Object*>::iterator it = objs.find(objId);
			if (it == objs.end()) PROMPT;
			Object* obj = objs[objId];
			printf("obj: id(%d) type(%d) pos(%d,%d) isWatcher(%d)\n", obj->GetId(), obj->GetType(), 
				obj->m_pos.x, obj->m_pos.y, obj->m_isWatcher?1:0);
		}else if (strVec[0].compare("m") == 0){
			if (strVec.size() < 4) PROMPT;
			uint32 objId = atoi(strVec[1].c_str());
			map<uint32, Object*>::iterator it = objs.find(objId);
			if (it == objs.end()) PROMPT;
			Object* obj = objs[objId];
			Location pos(0,0);
			pos.x = atoi(strVec[2].c_str());
			pos.y = atoi(strVec[3].c_str());
			if (aoi->UpdateObject(obj, obj->m_pos, pos)) {
				if (obj->m_isWatcher)
					aoi->UpdateWatcher(obj, obj->m_pos, RANGE_TOWER, pos, RANGE_TOWER);
				obj->SetPos(pos);
			}
			aoi->CheckConsistency();
		}else if (strVec[0].compare("a") == 0){
			AddRandomObject(objs, aoi);
			aoi->CheckConsistency();
		}else if (strVec[0].compare("d") == 0){
			if (strVec.size() < 2) PROMPT;
			uint32 objId = atoi(strVec[1].c_str());
			map<uint32, Object*>::iterator it = objs.find(objId);
			if (it == objs.end()) PROMPT;
			Object* obj = objs[objId];
			aoi->RemoveObject(obj, obj->m_pos);
			if (obj->m_isWatcher)
				aoi->RemoveWatcher(obj, obj->m_pos, RANGE_TOWER);
			delete obj;
			objs.erase(it);
			aoi->CheckConsistency();
		}else if (strVec[0].compare("p") == 0){
			if (strVec.size() < 2) PROMPT;
			uint32 opNum = atoi(strVec[1].c_str());
			for (uint32 num = 0; num < opNum; num++) {
				uint8 op = rand()%3;
				if (op == 0) { // add object
					AddRandomObject(objs, aoi);
				}else if (op == 1) { //delete object
					Object* obj = GetRandomObject(objs);
					if (obj) {
						aoi->RemoveObject(obj, obj->m_pos);
						if (obj->m_isWatcher)
							aoi->RemoveWatcher(obj, obj->m_pos, RANGE_TOWER);
						delete obj;
						objs.erase(obj->GetId());
					}
				}else { //move object
					Object* obj = GetRandomObject(objs);
					if (obj) {
						Location pos = GetRandLocaion();
						if (aoi->UpdateObject(obj, obj->m_pos, pos)) {
							if (obj->m_isWatcher)
								aoi->UpdateWatcher(obj, obj->m_pos, RANGE_TOWER, pos, RANGE_TOWER);
							obj->SetPos(pos);
						}
					}
				}
			}
			aoi->CheckConsistency();
		}else if (strVec[0].compare("c") == 0){
			aoi->CheckConsistency();
			printf("data is all right\n");
		}else if (strVec[0].compare("Q") == 0){
			break;
		}
	}

	//clear
	for (map<uint32, Object*>::iterator it = objs.begin(); it != objs.end(); it++){
		delete it->second;
	}
	delete aoi;

	printf("press enter to continue...");
	getchar();
	return 0;
}
