#pragma once
#pragma warning(disable:4996)

#include <vector>
#include <queue>

template<class T>
struct HuffmanTreeNode
{
	HuffmanTreeNode(const T& weight = T())
		: _pLeft(nullptr)
		, _pRight(nullptr)
		, _pParent(nullptr)
		, _Weight(weight)
	{}
	HuffmanTreeNode<T>* _pLeft;
	HuffmanTreeNode<T>* _pRight;
	HuffmanTreeNode<T>* _pParent;
	T _Weight;  //权值
};

template<class T>
class Less
{
	typedef HuffmanTreeNode<T> Node;
	
public:
	bool operator()(const Node* pLeft, const Node* pRight)
	{
		return pLeft->_Weight > pRight->_Weight;  //根据大于的方式构造出小根堆
	}

};

template<class T>
class HuffmanTree
{
	typedef HuffmanTreeNode<T> Node;

public:
	HuffmanTree()
		: _pRoot(nullptr)
	{}

	HuffmanTree(const std::vector<T> vWeight, const T& invalid_weight)
		: _pRoot(nullptr)
	{
		CreateHuffmanTree(vWeight, invalid_weight);
	}

	~HuffmanTree()
	{
		Destory(_pRoot);
	}

	Node* GetRoot()
	{
		return _pRoot;
	}

private:
	void CreateHuffmanTree(const std::vector<T> vWeight, const T& invalid_weight)
	{
		//1、构造森林
		std::priority_queue<Node*, std::vector<Node*>, Less<T>> queue;
		for (auto e : vWeight)
		{
			if (e == invalid_weight)
			{//无效的字符，即未出现的字符，_count == 0
				continue;
			}
			queue.push(new Node(e));
		}

		//2、将森林中的树不断合并，构造haffman树
		while (queue.size() > 1)
		{
			Node* pLeft = queue.top();
			queue.pop();

			Node* pRight = queue.top();
			queue.pop();

			Node* pParent = new Node(pLeft->_Weight + pRight->_Weight);
			pParent->_pLeft = pLeft;
			pParent->_pRight = pRight;
			pLeft->_pParent = pParent;
			pRight->_pParent = pParent;

			queue.push(pParent);
		}

		_pRoot = queue.top();
	}

	void Destory(Node*& pRoot)
	{
		if (pRoot)
		{
			Destory(pRoot->_pLeft);
			Destory(pRoot->_pRight);
			delete pRoot;
			pRoot = nullptr;
		}
	}
private:
	Node* _pRoot;
};
