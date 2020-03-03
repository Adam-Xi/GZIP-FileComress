#pragma once

#include <string.h>

typedef unsigned char UCH;
typedef unsigned short USH;
typedef unsigned long long ULL;

const USH MIN_MATCH = 3;  // 最短匹配长度
const USH MAX_MATCH = 258;  // 最大匹配长度：255 + 3
const USH WSIZE = 32 * 1024;  // 窗口大小：32K

const USH HASH_BITS = 15;  // 一个哈希地址有15个比特位，用于计算哈希地址
const USH HASH_SIZE = (1 << HASH_BITS);  // 哈希表的大小
const USH HASH_MASK = HASH_SIZE - 1;  // 掩码，防溢出

const USH MIN_LOOKAHEAD = MAX_MATCH + MIN_MATCH + 1;
const USH MAX_DIST = WSIZE - MIN_LOOKAHEAD;

class HashTable
{
public:
	HashTable(USH size)
		: _prev(new USH[size * 2])
		, _head(_prev + size)
	{
		memset(_prev, 0, size * 2 * sizeof(USH));
	}

	~HashTable()
	{
		delete[] _prev;
		_prev = nullptr;
		_head = nullptr;
	}

	void Insert(USH& matchHead, UCH ch, USH pos, USH& hashAddr)
	{
		// 根据当前字符计算哈希地址
		HashFunc(hashAddr, ch);
		// 找当前三个字符在查找缓冲区中最近的一个，即匹配链的头
		matchHead = _head[hashAddr];
		// 将头中的值，即下一个节点的下标放入_prev表中，即插入操作
		_prev[pos & HASH_MASK] = _head[hashAddr];
		// 更新匹配链的头
		_head[hashAddr] = pos;
	}

	// hashAddr: 上一个字符串计算出的哈希地址
	// ch：当前字符
	// 本次的哈希地址是在前一次哈希地址基础上，再结合当前字符ch计算出来的
	// HASH_MASK为WSIZE-1，&上掩码主要是为了防止哈希地址越界
	void HashFunc(USH& hashAddr, UCH ch)
	{
		hashAddr = (((hashAddr) << H_SHIFT()) ^ (ch)) & HASH_MASK;
	}

	USH GetNextHashAddr(USH matchHead)
	{
		return _prev[matchHead & HASH_MASK];
	}

	void Update()
	{
		for (USH i = 0; i < WSIZE; i++)
		{
			// 更新head
			if (_head[i] >= WSIZE)
			{
				_head[i] -= WSIZE;
			}
			else
			{
				_head[i] = 0;
			}

			// 更新prev
			if (_prev[i] >= WSIZE)
			{
				_prev[i] -= WSIZE;
			}
			else
			{
				_prev[i] = 0;
			}
		}
	}
private: 
	USH H_SHIFT()
	{
		return (HASH_BITS + MIN_MATCH - 1) / MIN_MATCH;
	}
private:
	USH* _prev;
	USH* _head;
};