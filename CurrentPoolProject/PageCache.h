#pragma once
#include "Common.h"

class PageCache
{
public:
	Span* NewSpan(size_t numpage);
	Span* GetIdToSpan(PAGE_ID id);
	void ReleaseSpanToPageCache(Span* span);
	//��ϵͳ�����ڴ�ҵ���������
	//void SystemAlloc(size_t k);
private:
	SpanList _spanLists[MAX_PAGES];
	std::map<PAGE_ID, Span*> _idSpanMap;

};

static PageCache pageCacheInst;
