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
void ThreadCache::Deallocte(void* ptr, size_t size)  //������
{
	size_t index=SizeClass::ListIndex(size);
	FreeList& freeList = _freeLists[index];
	freeList.Push(ptr);
	size_t num = SizeClass::NumMoveSize(size);
	if (freeList.Num()>=num)  //����һ�����������
	{
		ListTooLong(freeList,num,size);  //��FreeList�ҵ�̫��
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
	//ģ��ȡ�ڴ����Ĵ���
	size_t num = SizeClass::NumMoveSize(size);
	void *start = nullptr, *end = nullptr;
	size_t actualNum=centralCacheInst.FetchRangeObj(start, end, num, size);

	if (actualNum == 1)
	{
		return start;
	}
	else  //һ���Լ�һ������
	{
		size_t index = SizeClass::ListIndex(size);
		FreeList &list = _freeLists[index];
		list.PushRange(NextObj(start),end,actualNum-1);//���ֻ��һ���ͷ��أ�����ж���ǾͰ������ĸ�������
		return start;
	}
	
}

