#pragma once

#include "Common.h"
class ThreadCache
{
public:
	   //申请内存和释放内存
	    void* Allocte(size_t size);
		void Deallocte(void* ptr, size_t size);
		//从中心缓存获取对象
		void* FetchFromCentralCache(size_t index);
		void ListTooLong(FreeList& freelist, size_t num,size_t size);//如果自由链表对象的长度太多就要释放给中心缓存
private:
	FreeList _freeLists[NFREE_LIST];
	//ThreadCache* _next;
	//int threadid;
};
// ThreadCache _head;