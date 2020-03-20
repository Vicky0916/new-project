#pragma once
#include "Common.h"  
 
class CentralCache
{
public:
	//�����Ļ����л�ȡһ�������Ķ����thread cache
	size_t FetchRangeObj(void *&start, void*& end, size_t n, size_t b);
	//��һ�������Ķ����ͷŵ�span���
	void ReleaseListToSpans (void *start, size_t size);
	//��spanlist���ߴ�page cache��ȡһ��span 
	Span *GetOneSpan(SpanList* list, size_t size); 
private:
	SpanList _spanLists[NFREE_LIST];

};

static CentralCache centralCacheInst;    //
