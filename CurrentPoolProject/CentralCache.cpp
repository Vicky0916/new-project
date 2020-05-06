#include "CentralCache.h"
#include "PageCache.h"


Span* CentralCache::GetOneSpan(size_t size)
{
	size_t index = SizeClass::ListIndex(size);
	SpanList& spanlist = _spanLists[index];
	Span* it = spanlist.Begin();
	while (it != spanlist.End())
	{
		if (!it->_freeList.Empty())
		{
			return it;
		}
		else
		{
			it = it->_next;
		}
	}
	// page cache 获取一个span
	size_t numpage = SizeClass::NumMovePage(size);
	Span* span = pageCacheInst.NewSpan(numpage);
	//spanlist.PushFront(span);
	//Span* span = pageCacheInst.NewSpan(numpage);
	//把span对象切成对应大小挂到span的freelist中

	char* start = (char*)(span->_pageid<<12);  //算出span的起始位置和结束位置，分成很多个size大小
	char *end = start + (span->_pagesize << 12);
	while (start < end)
	{
		char* obj = start;
		start += size;

		span->_freeList.Push(obj);   //挂到span的 freelist
	}
	span->_objsize = size;
	spanlist.PushFront(span);
	return span;
}
size_t CentralCache::FetchRangeObj(void *&start, void*& end, size_t num, size_t size)//获取一段对象
{
	//中心Cache是以Span为单位管理的
	//获取一个有对象的Span
	Span* span=GetOneSpan(size);
	FreeList& freelist = span->_freeList;
	size_t actualNum = freelist.PopRange(start, end, num);
	span->_usecount += actualNum;
	size_t size = SizeClass::ListIndex(size);
	SpanList& spanlist = _spanLists[index];
	Span* it = spanlist.Begin();
	while(it != spanlist.End())
	{
		if (!it->_freeList.Empty())
		{
			//直接取
		}
		else
		{
			it = it->_next;
		}
	}

	//走到这里说明span里面没有，应该向pagecache取
	//分两种情况：
	//要十个你有二十个
	//要十个你只有五个
}

void CentralCache::ReleaseListToSpans(void* start,size_t size)
{
	size_t index = SizeClass::ListIndex(size);
	SpanList& spanlist = _spanLists[index];
	spanlist.Lock();
	while (start)
	{
		void* next = NextObj(start);
		PAGE_ID id = (PAGE_ID)start >> PAGE_SHIFT;
		Span* span = pageCacheInst.GetIdToSpan(id);
		span->_freeList.Push(start);
		span->_usecount--; 
		if (span->_usecount == 0)  //这个span已经全部回来了,要还给pagecache,减少内存碎片
		{
			size_t index = SizeClass::ListIndex(span->_objsize);
			_spanLists[index].Erase(span);
			span->_freeList.Clear();
			pageCacheInst.ReleaseSpanToPageCache(span);
		}
		start = next;
	}
	spanlist.UnLock();
}