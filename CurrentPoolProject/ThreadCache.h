#pragma once

#include "Common.h"
class ThreadCache
{
public:
	   //�����ڴ���ͷ��ڴ�
	    void* Allocte(size_t size);
		void Deallocte(void* ptr, size_t size);
		//�����Ļ����ȡ����
		void* FetchFromCentralCache(size_t index);
		void ListTooLong(FreeList& freelist, size_t num,size_t size);//��������������ĳ���̫���Ҫ�ͷŸ����Ļ���
private:
	FreeList _freeLists[NFREE_LIST];
	//ThreadCache* _next;
	//int threadid;
};
// ThreadCache _head;