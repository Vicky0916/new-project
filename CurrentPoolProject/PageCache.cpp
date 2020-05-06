#include "PageCache.h"

//向系统申请内存挂到自由链表
void  PageCache::SystemAlloc(size_t numpage)
{

}

Span* PageCache::NewSpan(size_t numpage)
{
	if (!_spanLists[numpage].Empty())
	{
		Span* span = _spanLists[numpage].Begin();
		_spanLists[numpage].PopFront();
		return span;
	}

	for (size_t i = numpage + 1; i < MAX_PAGES; ++i)
	{
		//分裂的过程
		if (!_spanLists[i].Empty())
		{
			Span* span = _spanLists[i].Begin();
			_spanLists[i].PopFront();

			Span* splistspan = new Span;
			splistspan->_pageid = span->_pageid + span->_pagesize - numpage;
			splistspan->_pagesize = numpage;
			for (PAGE_ID i = 0; i < numpage; ++i)
			{
				_idSpanMap[splistspan->_pageid + i] = splistspan;
			}
			span->_pagesize -= numpage;
			_spanLists[span->_pagesize].PushFront(span);
			return splistspan;
		}
	}

	void *ptr = SystemAlloc(MAX_PAGES - 1);

		Span* bigspan = new Span;
		bigspan->_pageid = (PAGE_ID)ptr >> PAGE_SHIFT;
		bigspan->_pagesize = MAX_PAGES - 1;

		for (PAGE_ID i = 0; i < bigspan->_pagesize; ++i)
		{
			//_idSpanMap.insert(std::make_pair(bigspan->_pageid + i, bigspan));
			_idSpanMap[bigspan->_pageid + i] = bigspan;
		}
		_spanLists[bigspan->_pagesize].PushFront(bigspan);

		return NewSpan(numpage);
}
void PageCache::ReleaseSpanToPageCache(Span* span)
{
	//页的合并
	while (1)
	{
		PAGE_ID prevPageid = span->_pageid - 1;
		auto pit = _idSpanMap.find(prevPageid);
		//前面的页不存在
		if (pit == _idSpanMap.end())
		{
			break;
		}
		Span* prevSpan = pit->second;
		if (prevSpan->_usecount != 0)
		{
			break;
		}
		span->_pageid = prevSpan->_pageid;
		span->_pagesize += span->_pagesize;
		for (PAGE_ID i = 0; i < prevSpan->_pagesize; i++)
		{
			_idSpanMap[prevSpan->_pageid + i] = span;
		}

		_spanLists[prevSpan->_pagesize].Erase(prevSpan);
		delete prevSpan;
	}
	//向后合并
	while (1)
	{
		PAGE_ID nextPageId = span->_pageid + span->_pagesize;
		auto nextIt = _idSpanMap.find(nextPageId);
		if (nextIt == _idSpanMap.end())
		{
			break;
		}
		Span* nextSpan = nextIt->second;
		if (nextSpan->_usecount != 0)
		{
			break;
		}
		span->_pagesize += nextSpan->_pagesize;
		for (PAGE_ID i = 0; i < nextSpan->_pagesize; ++i)
		{
			_idSpanMap[nextSpan->_pageid + i] = span;
		}
		_spanLists[nextSpan->_pagesize].Erase(nextSpan);
		delete nextSpan;
	}
	_spanLists[span->_pagesize].PushFront(span);  //挂起来
}
Span* PageCache::GetIdToSpan(PAGE_ID id)
{
	auto it = _idSpanMap.find(id);
	if (it != _idSpanMap.end())
	{
		return it->second;
	}
	else
	{
		return nullptr;
	}
}