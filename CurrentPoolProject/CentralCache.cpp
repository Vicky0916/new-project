#include "CentralCache.h"


Span* CentralCache::GetOneSpan(size_t size)
{
	size_t index = SizeClass::ListIndex(size);
	SpanList& spanlist = _spanlists[index];
	Span* it = spanlist.Begin();
	while (it != spanlist.End())
	{
		if (!it->freeList.Empty())
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

	Span* span = pageCacheInst.NewSpan(numpage);
	//把span对象切成对应大小挂到span的freelist中

	char* start = (char*)(span->_pageid<<12);
	char *end = start + (span->_pagesize << 12);
	while (start < end)
	{
		char* obj = start;
		start += size;

		span->_freeList.Push(obj);
	}
	spanlist.PushFront(span);
	return span;
}
size_t CentralCache::FetchRangeObj(void *&start, void*& end, size_t n, size_t b)//获取一段对象
{
	//中心Cache是以Span为单位管理的
	//获取一个有对象的Span
	Span* span=GetOneSpan(size);
	FreeList& freelist = span->_freeList;
	size_t actualNum = freelist.PopRange(start, end, num);
	size_t size = SizeClass::ListIndex(size);
	SpanList& spanlist = _spanlists[index];
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