#pragma once

#include "HashTable.hpp"
#include "common.h"

class LZ77
{
public:
	LZ77();
	~LZ77();
	void CompressFile(const std::string& strFilePath, std::string& lz_newFileNameOnly);
	void UnCompressFile(const std::string& strFilePath);
private:
	USH LongestMatch(USH matchHead, USH& MatchDistance, USH start);
	void WriteFlag(FILE* fOUT, UCH& chFlag, UCH& bitCount, bool isLength);
	void MergeFile(FILE* fOUT, ULL fileSize);
	void FillWindow(FILE* fIN, size_t& lookAhead, USH& start);

private:
	UCH* _pWin;  // 用来保存待压缩数据的缓冲区
	HashTable _hash;
};
