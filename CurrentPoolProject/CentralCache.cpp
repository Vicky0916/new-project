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
	// page cache ��ȡһ��span
	size_t numpage = SizeClass::NumMovePage(size);
	Span* span = pageCacheInst.NewSpan(numpage);

	Span* span = pageCacheInst.NewSpan(numpage);
	//��span�����гɶ�Ӧ��С�ҵ�span��freelist��

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
size_t CentralCache::FetchRangeObj(void *&start, void*& end, size_t n, size_t b)//��ȡһ�ζ���
{
	//����Cache����SpanΪ��λ�����
	//��ȡһ���ж����Span
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