#pragma once
#include <iostream>
#include <assert.h>
using namespace std;

const size_t MAX_SIZE = 64 * 1024;
const size_t NFREE_LIST = MAX_SIZE / 8;

inline void*& NextObj(void* obj)    //ȡ��һ�����󣬿����ĸ��ֽڻ��߰˸��ֽ�
{
	return *((void**)obj);   //���ú����������ĵģ������������ʱƵ�����õ�
}

class FreeList
{
public:
	void Push(void *obj)
	{
		//ͷ��
		NextObj(obj) = _freelist;
		_freelist = obj;
	}
	void PushRange(void* head, void* tail)
	{
		NextObj(tail) = _freelist;
		_freelist = head;
	}
	void *Pop()
	{
		//ͷɾ
		void* obj = _freelist;
		_freelist = NextObj(obj);
		return obj;
	}
	void PushRange(void * head, void* tail)
	{
		NextObj(tail) = _freelist;
		_freelist = head;
	}
	bool Empty()
	{
		return _freelist == nullptr; 
	}

	
private:
	void * _freelist=nullptr;

};

class SizeClass
{
public:
	static size_t ListIndex(size_t size,int align_shift)
	{
		return (size + (1 << align_shift) - 1) >> align_shift - 1;
	}

	static size_t NumMoveSize(size_t size)
	{
		if (size == 0)
			return 0;

		int num = MAX_SIZE / size;
		if (num < 2)
			num = 2;

		if (num > 512)
			num = 512;

		return num;
	}

	//������[1%,10%]���ҵ��ڴ��������
	//[1,128]  8byte����   freelist[0,16]
	//[129,1024]  16byte���룬freelist[16,72]
	//[1025,8*1024] 128byte���룬freelist[72,128]
	//[8*1024+1,64*1024] 1024byte���룬freelist[128,184] 
	static size_t _RoundUp(size_t size,int alignment)   //���϶��룬���������5���Ҿ͸���8
	{//�ڶ�������Ϊ������
		return (size + alignment - 1)&(~(alignment - 1));
	}

	static inline size_t RoundUp(size_t size)
	{
		assert(size <= MAX_SIZE);
		if (size <= 128)
		{
			return _RoundUp(size, 8);
		}
		else if (size <= 1024)
		{
			return _RoundUp(size, 16);
		}
		else if (size <= 8192)
		{
			return _RoundUp(size, 128);
		}
		else if (size <= 65536)
		{
			return _RoundUp(size, 1024); 
		}
	}
	static size_t _ListIndex(size_t size, int align_shift)
	{
		return ((size + (1 << align_shift) - 1) >> align_shift) - 1;
	}

	static size_t ListIndex(size_t size,int align_shift)
	{
		assert(size <= MAX_SIZE);

		//ÿ�������ж��ٸ���
		static int group_array[4] = { 16,56,56,56  };
		if (size <= 128)
		{
			return _ListIndex(size, 3);
		}
		else if (size <= 1024)
		{
			return _ListIndex(size - 128, 4) + group_array[0];
		}
		else if (size <= 8192)
		{
			return _ListIndex(size - 1024, 7) + group_array[1] + group_array[0];
		}
		else if (size <= 65536)
		{
			return _ListIndex(size - 8192, 9) + group_array[2] + group_array[1] + group_array[0];
		}
		assert(false);
		return -1;
	}
};

class Span
{
	
	PAGE_ID _pageid;  //ҳ��
	int _pagesize;   //ҳ������
	FreeList _freeList ;   //������������
	int _usecount;    //�ڴ�����ʹ�ü���

	//size_t objsize;  �����С
	Span* _next;   
	Span* _prev;
};

class SpanList
{
public:
	SpanList()
	{
		_head = new Span;
		_head->next = _head;
		_head->_prev = _head;
	}
	Span* Begin()
	{
		return _head->_next;
	}
	Span* End()
	{
		return _head;
	}
	void PushFront(Span* newspan)
	{
		Insert(_head->next, newspan);
	}
	void PopFront()
	{
		Erase(_head->_next);
	}
	void PushBack(Span* newspan)
	{
		Insert(_head, newspan);
	}
	void Popback()
	{
		Erase(_head->_prev);
	}
	void Insert(Span* pos, Span* newspan)
	{
		Span* prev = pos->_prev;

		//prev newspan pos
		prev->_next = newspan;
		newspan->_next = pos;
		pos->_prev = newspan;
		newspan -> _prev = prev;
	}

	void Erase(Span* pos)
	{
		assert(pos != _head);

		Span* prev = pos->_prev;
		Span*_next = pos->_next;

		prev->_next = next;
		next->_prev = prev;
	}
private:
	Span* _head;
};
 