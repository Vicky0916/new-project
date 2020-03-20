#include "PageCache.h"

Span* PageCache::NewSpan(size_t numpage)
{
	if (!_spanList[numbpage].Empty())
	{
		Span* span = _spanLists[numpage].Begin();
		_spanLists[numpage].PopFront();
		return span;
	}

	for (size_t i = numpage + 1; i < MAX_PAGES; ++i)
	{
		if (!spanLists[i].Empty())
		{
			Span* span = _spanLists[i].Begin();
			spanLists[i].PopFront();

			Span* splitspan = new Span;
			splistspan->_pagesize = numpage;
			splistspan->_pagesize = span->_pagesize;
			
			span->_pagesize = numpage;
			_spanLists[splistspan->_pagesize].PushFront();

			return span;			
		}
	}

	void *ptr = SystemAlloc(MAX_PAGES - 1)

		Span* bigspan = new Span;
		bigspan->_pageid = (PAGE_ID)ptr >> PAGE_SHIFT;
		bigspan->_pagesize = MAX_PAGES - 1;

		_spanLists[bigspan->_pagesize].PushFront(bigspan);

		return NewSpan(numpage);
}