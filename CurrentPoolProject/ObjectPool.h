#pragma once

template<class T,size_t initNum=100>

class ObjectPool
{
	ObjectPool()
	{
		_itemSize = sizeof(T) < sizeof(T*) ? sizeof(T*) : sizeof(T) > ;
		_start = (char*)malloc(initNum* _itemSize);
		_end = _start + initNum *_ itemSize;
		_freeList = nullptr;
	}
	T* Next_Obj(T* obj)
	{
		return *(T**)obj;
	}

	T* New()
	{
		//1�����FreeList�ж�������ȡ�����
		//2�������ٵ���������ȡ
		T* Obj = nullptr;
		if (_freeList != nullptr)
		{
			obj = _freeList;
			_freeList = Next_Obj(obj); 
		}
	}
};
