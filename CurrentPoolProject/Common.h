#pragma once
#include <iostream>
#include <assert.h>
using namespace std;

const size_t MAX_SIZE = 64 * 1024;
const size_t NFREE_LIST = MAX_SIZE / 8;

inline void*& NextObj(void* obj)    //取下一个对象，可能四个字节或者八个字节
{
	return *((void**)obj);   //调用函数是有消耗的，并且这个函数时频繁调用的
}

class FreeList
{
public:
	void Push(void *obj)
	{
		//头插
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
		//头删
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

	//控制在[1%,10%]左右的内存对齐数字
	//[1,128]  8byte对齐   freelist[0,16]
	//[129,1024]  16byte对齐，freelist[16,72]
	//[1025,8*1024] 128byte对齐，freelist[72,128]
	//[8*1024+1,64*1024] 1024byte对齐，freelist[128,184] 
	static size_t _RoundUp(size_t size,int alignment)   //向上对齐，如果你请求5，我就给你8
	{//第二个参数为对齐数
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

		//每个区间有多少个链
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
	
	PAGE_ID _pageid;  //页号
	int _pagesize;   //页得数量
	FreeList _freeList ;   //对象自由链表
	int _usecount;    //内存块对象使用计数

	//size_t objsize;  对象大小
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
 