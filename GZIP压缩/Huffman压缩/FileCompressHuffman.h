//����huffman��ѹ��
#pragma once

#include <vector>
#include "Huffman.hpp"
#include "common.h"

//��Ȩֵ�ýṹ���ʾ,������Ҫ��huffman�еĲ�������������
struct CharInfo
{
	unsigned char _ch;  //�����ַ�
	int _count;  //�ַ����ִ���
	std::string _strCode;  //�ַ�����

	CharInfo(int count = 0)
		: _count(count)
	{}

	CharInfo operator+(const CharInfo& ch) const
	{
		return CharInfo(_count + ch._count);
	}

	bool operator>(const CharInfo& ch) const
	{
		return _count > ch._count;
	}

	bool operator==(const CharInfo& ch) const
	{
		return _count == ch._count;
	}
};

class FileCompressHuffman
{
public:
	FileCompressHuffman();
	void CompressFile(const std::string& path);
	void UnCompressFile(const std::string& path, std::string& huff_newFileName);

private:
	void GenerateHuffmanCode(HuffmanTreeNode<CharInfo>* pRoot);  //����huffman����
	void WriteHead(FILE*  fOut, const std::string& filePostFix);

private:
	std::vector<CharInfo> _fileInfo;
};

