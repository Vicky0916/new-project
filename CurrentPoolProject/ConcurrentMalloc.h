#pragma once

#include "ThreadCache.h"
#include "PageCache.h"
void* ConcurrentMalloc(size_t size)
{
	if (pThreaCache == nullptr)
	{
		pthreadCache = new ThreadCache;
		cout << std::this_thread::get_id() << "->" << pthreadCache << endl;
	}
	if (size <= MAX_SIZE)  //дк1-64kb
	{
		return pThreaCache->Allocte(size);
	}
	else if (size <= (MAX_PAGES - 1)<<PAGE_SHIFT)  //64-128*4kb
	{
		size_t align_size = SizeClass::_RoundUp(size, 1 << PAGE_SHIFT);
		size_t pagenum = (align_size >> PAGE_SHIFT);
		Span* span = pageCacheInst.NewSpan(pagenum);
		span->_objsize  = align_size;
		void* ptr = (void*)(span->_pageid << PAGE_SHIFT);
		return ptr;
	}
	else   //[128*4kb]
	{
		size_t align_size = SizeClass::_RoundUp(size, 1 << PAGE_SHIFT);
		size_t pagenum = (align_size >> PAGE_SHIFT);
		SystemAlloc(pagenum);
	}
}

void ConcurrentFree(void* ptr)
{
	size_t pageid = (PAGE_ID)ptr >> PAGE_SHIFT;
	Span* span = pageCacheInst.GetIdToSpan(pageid);
	size_t size = span->_objsize;
	if (span == nullptr)
	{
		SystemFree(ptr);
		return;
	}
	if (size <= MAX_SIZE)
	{
		return pThreaCache->Allocte(size);
	}
	else if (size <= (MAX_PAGES - 1) << PAGE_SHIFT)
	{
		pageCacheInst.ReleaseSpanToPageCache(span);
	}
	
}