#pragma once

#include <string.h>

typedef unsigned char UCH;
typedef unsigned short USH;
typedef unsigned long long ULL;

const USH MIN_MATCH = 3;  // ���ƥ�䳤��
const USH MAX_MATCH = 258;  // ���ƥ�䳤�ȣ�255 + 3
const USH WSIZE = 32 * 1024;  // ���ڴ�С��32K

const USH HASH_BITS = 15;  // һ����ϣ��ַ��15������λ�����ڼ����ϣ��ַ
const USH HASH_SIZE = (1 << HASH_BITS);  // ��ϣ��Ĵ�С
const USH HASH_MASK = HASH_SIZE - 1;  // ���룬�����

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
		// ���ݵ�ǰ�ַ������ϣ��ַ
		HashFunc(hashAddr, ch);
		// �ҵ�ǰ�����ַ��ڲ��һ������������һ������ƥ������ͷ
		matchHead = _head[hashAddr];
		// ��ͷ�е�ֵ������һ���ڵ���±����_prev���У����������
		_prev[pos & HASH_MASK] = _head[hashAddr];
		// ����ƥ������ͷ
		_head[hashAddr] = pos;
	}

	// hashAddr: ��һ���ַ���������Ĺ�ϣ��ַ
	// ch����ǰ�ַ�
	// ���εĹ�ϣ��ַ����ǰһ�ι�ϣ��ַ�����ϣ��ٽ�ϵ�ǰ�ַ�ch���������
	// HASH_MASKΪWSIZE-1��&��������Ҫ��Ϊ�˷�ֹ��ϣ��ַԽ��
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
			// ����head
			if (_head[i] >= WSIZE)
			{
				_head[i] -= WSIZE;
			}
			else
			{
				_head[i] = 0;
			}

			// ����prev
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