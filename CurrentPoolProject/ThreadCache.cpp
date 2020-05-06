#pragma once
#include "CentralCache.h"
#include "ThreadCache.h"
void* ThreadCache::Allocte(size_t size)
{
	size_t index = SizeClass::ListIndex(size);
	FreeList& freeList = _freeLists[index];
	if (!freeList.Empty())
	{
		return freeList.Pop();
	}
	else
	{
		return FetchFromCentralCache(SizeClass::RoundUp(size));
	}
	
}
void ThreadCache::Deallocte(void* ptr, size_t size)  //还给我
{
	size_t index=SizeClass::ListIndex(size);
	FreeList& freeList = _freeLists[index];
	freeList.Push(ptr);
	size_t num = SizeClass::NumMoveSize(size);
	if (freeList.Num()>=num)  //大于一次批量申请的
	{
		ListTooLong(freeList,num,size);  //在FreeList挂的太多
	}
	 
}
void ThreadCache::ListTooLong(FreeList& freeList, size_t num,size_t size)
{
	void* start = nullptr, *end = nullptr;
	freeList.PopRange(start, end, num);
	NextObj(end) = nullptr;
	centralCacheInst.ReleaseListToSpans(start );

}
void* ThreadCache::FetchFromCentralCache(size_t size)
{
	//模拟取内存对象的代码
	size_t num = SizeClass::NumMoveSize(size);
	void *start = nullptr, *end = nullptr;
	size_t actualNum=centralCacheInst.FetchRangeObj(start, end, num, size);

	if (actualNum == 1)
	{
		return start;
	}
	else  //一个以及一个以上
	{
		size_t index = SizeClass::ListIndex(size);
		FreeList &list = _freeLists[index];
		list.PushRange(NextObj(start),end,actualNum-1);//如果只有一个就返回，如果有多个那就把其他的给挂起来
		return start;
	}
	
}

