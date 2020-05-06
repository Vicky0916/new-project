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
	// page cache ��ȡһ��span
	size_t numpage = SizeClass::NumMovePage(size);
	Span* span = pageCacheInst.NewSpan(numpage);
	//spanlist.PushFront(span);
	//Span* span = pageCacheInst.NewSpan(numpage);
	//��span�����гɶ�Ӧ��С�ҵ�span��freelist��

	char* start = (char*)(span->_pageid<<12);  //���span����ʼλ�úͽ���λ�ã��ֳɺܶ��size��С
	char *end = start + (span->_pagesize << 12);
	while (start < end)
	{
		char* obj = start;
		start += size;

		span->_freeList.Push(obj);   //�ҵ�span�� freelist
	}
	span->_objsize = size;
	spanlist.PushFront(span);
	return span;
}
size_t CentralCache::FetchRangeObj(void *&start, void*& end, size_t num, size_t size)//��ȡһ�ζ���
{
	//����Cache����SpanΪ��λ�����
	//��ȡһ���ж����Span
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
			//ֱ��ȡ
		}
		else
		{
			it = it->_next;
		}
	}

	//�ߵ�����˵��span����û�У�Ӧ����pagecacheȡ
	//�����������
	//Ҫʮ�����ж�ʮ��
	//Ҫʮ����ֻ�����
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
		if (span->_usecount == 0)  //���span�Ѿ�ȫ��������,Ҫ����pagecache,�����ڴ���Ƭ
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