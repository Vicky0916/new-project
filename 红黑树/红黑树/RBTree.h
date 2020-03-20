#pragma once

enum Colour
{
	BLACK,
	RED,
};

template<class T>
struct RBTreeNode
{
	RBTreeNode<T>* _left;
	RBTreeNode<T>* _right;
	RBTreeNode<T>* _parent;

	T _data;
	Colour _col;

	RBTreeNode
};

template<class K, class V>
class RBTree
{
	typedef RBTreeNode<pair<K, V>> Node;
public:
	RBTree()
		:_root(nullptr)
	{

	}

	bool Insert(const pair<K, V>& kv)
	{
		if (_root == nullptr)
		{
			_root->_col = BLACK;
			_root = new Node(kv);
			return make_pair(_root, true);
		}
		Node* parent = nullptr;
		Node* cur = _root;
			while (cur)
			{
				if (cur->_data.first > kv.first)
				{
					parent = cur;
					cur = cur->_left;
				}
				else if (cur->_data.first < kv.first)
				{
					parent = cur;
					cur = cur->_right;
				}
				else
					return make_pair(cur, false);
			}
			cur = new Node(kv);
			cur->_col = RED;
			if (cur->_data.first > parent->_data.first)
			{
				 parent->_right=cur;
				 cur->_parent = parent;
			}
			else (cur->_data.first < parent->_data.first)
			{
				 parent->_left=cur;
				 cur->_parent = parent;
			}

			//
			while(parent&&parent->_col == RED)
			{
				Node* grandfather = parent->_parent;
				if (parent == grandfather->_left)
				{
					Node* uncle = grandfather->_right;
					//情况一：如果叔叔存在并且叔叔的颜色为红色
					//那叔叔和父亲变为黑色，祖父变为红色
					//再继续向上调整
					if (uncle&&uncle->_col == RED)
					{
						parent->_col = uncle->_col = BLACK;
						grandfather->_col = RED;

						cur = grandfather;
						parent = cur->_parent;
					}
					//情况二/三：祖父不存在或者存在但是颜色为黑色
					//需要旋转处理
					else
					{
						//旋转--单旋
						if (cur == parent->_left)
						{
							//右单旋
							//  g
							//p
						  //c
							RotateR(parent);
							parent->_col = BLACK;
							grandfather->_col = RED;
						}

						else
						{
							//左右单旋
							//  g
							//p
						  //    c
							RotateLR(grandfather);
							grandfather->_col = RED;
							cur->_col = BLACK;
						}
						break;
					}
				}
				else
				{
					Node* uncle = grandfather->_left;
					//  g
				   //      p
				  //         c
					if (uncle&&uncle->_col == RED)
					{
						parent->_col = uncle->_col = BLACK;
						grandfather->_col = RED;

						cur = grandfather;
						parent = cur->_parent;
					}
					else
					{
						if (cur == parent->_right)
						{
							//单旋
							RotateL(grandfather);
							parent->_col = BLACK;
							grandfather->_col = RED;
						}
						else
						{  //   g
						  //       p
						 //     c    
							RotateRL(grandfather);
							cur->_col = BLACK;
							grandfather->_col = RED;
						}
						break;
					}
				}
			}
			_root->_col = BLACK;

			return make_pair(cur, true);
	}
private:
	Node* _root;
};