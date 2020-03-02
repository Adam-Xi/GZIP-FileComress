#include "LZ77.h"

LZ77::LZ77()
	: _pWin(new UCH[WSIZE * 2])
	, _hash(WSIZE)
{}

LZ77::~LZ77()
{
	delete[] _pWin;
	_pWin = nullptr;
}

//////////////////////////////////////////////////////////////////////
// 功能：基于LZ77的压缩
//////////////////////////////////////////////////////////////////////
void LZ77::CompressFile(const std::string& strFilePath, std::string& lz_newFileNameOnly)
{
	// 如果文件总大小小于 MIN_MATCH ，则不进行匹配
	FILE* fIN = fopen(strFilePath.c_str(), "rb");
	if (nullptr == fIN)
	{
		std::cout << "打开文件失败" << std::endl;
		return;
	}
	// 获取文件大小
	fseek(fIN, 0, SEEK_END);
	ULL fileSize = ftell(fIN);

	if (fileSize <= MIN_MATCH)
	{
		std::cout << "文件太小，不进行压缩" << std::endl;
		return;
	}

	fseek(fIN, 0, SEEK_SET);

	// 从压缩文件中读取一个缓冲区的数据到窗口中
	size_t lookAhead = fread(_pWin, 1, WSIZE * 2, fIN);  // lookAhead表示先行缓冲区中剩余字符个数
	
	USH hashAddr = 0;
	// 设置起始hashAddr
	for (USH i = 0; i < MIN_MATCH - 1; i++)
	{
		_hash.HashFunc(hashAddr, _pWin[i]);
	}

	// 压缩
	lz_newFileNameOnly = _FileName::GetFileNameOnly(strFilePath);
	lz_newFileNameOnly += ".lzp";
	FILE* fOUT = fopen(lz_newFileNameOnly.c_str(), "wb");
	if (nullptr == fOUT)
	{
		std::cout << "打开文件失败" << std::endl;
		return;
	}

	// 将文件后缀首先写入压缩文件中
	std::string filePostFix = _FileName::GetFilePostFix(strFilePath);
	filePostFix += '\n';
	fwrite(filePostFix.c_str(), strlen(filePostFix.c_str()), 1, fOUT);
	fflush(fOUT);

	// 与查找最长匹配相关的变量
	USH matchHead = 0;
	USH start = 0;  // 从窗口最左侧进行匹配
	USH curMatchLength = 0;  // 当前的最大匹配长度
	USH curMatchDistance = 0;  // 当前最大匹配长度对应的距离

	// 与写入标记信息相关的变量
	UCH chFlag = 0;
	UCH bitCount = 0;
	// bool isLength = false;
	FILE* fOutFlag = fopen("3.txt", "wb");  // 将标记信息写入3.txt中
	assert(fOutFlag);

	while (lookAhead)
	{ 
		// 将当前三个字符(start, start + 1, start + 2)插入到哈希表中，并获取匹配头
		_hash.Insert(matchHead, _pWin[start + 2], start, hashAddr);

		curMatchLength = 0;
		curMatchDistance = 0;

		// 验证在查找缓冲区中是否找到匹配
		if (matchHead)
		{
			// 顺着匹配链找最长匹配
			curMatchLength = LongestMatch(matchHead, curMatchDistance, start);
		}

		// 验证在匹配链中是否找到匹配
		if (curMatchLength < MIN_MATCH)
		{
			// 在查找缓冲区中未查找到重复字符串
			// 将start位置的字符写入到压缩文件中
			fputc(_pWin[start], fOUT);

			// 写当前原字符对应的标记
			WriteFlag(fOutFlag, chFlag, bitCount, false);
			start++;
			lookAhead--;
		}
		else
		{
			// 找到匹配 
			// 将<长度,距离>写入到压缩文件中
			// 写长度
			UCH chLength = curMatchLength - 3;
			fputc(chLength, fOUT);
			// 写距离
			fwrite(&curMatchDistance, sizeof(curMatchDistance), 1, fOUT);
			// 写当前原字符对应的标记
			WriteFlag(fOutFlag, chFlag, bitCount, true);

			// 更新先行缓冲区中剩余字符的个数
			lookAhead -= curMatchLength;

			// 将已经匹配的字符串按照三个一组插入到哈希表中
			curMatchLength--;
			while (curMatchLength)
			{
				start++;
				_hash.Insert(matchHead, _pWin[start], start, hashAddr);
				curMatchLength--;
			}
			start++;  // 在循环中，start少加了一次
		}

		// 检测先行缓冲区中剩余字符的个数
		if (lookAhead <= MIN_LOOKAHEAD)
		{
			FillWindow(fIN, lookAhead, start);
		}
	}

	//标记位数不够8个比特位
	if (bitCount > 0 && bitCount < 8)
	{
		chFlag <<= (8 - bitCount);
		fputc(chFlag, fOutFlag);
	}

	fclose(fOutFlag);

	// 将压缩数据文件和标记信息文件进行合并
	MergeFile(fOUT, fileSize);

	fclose(fIN);
	fclose(fOUT);

	// 将用来保存标记信息的临时文件删除掉
	remove("./3.txt");
}

void LZ77::FillWindow(FILE* fIN, size_t& lookAhead, USH& start)
{
	// 压缩已经进行到右窗，先行缓冲区中剩余数据不够MIN_LOOKAHEAD
	if (start >= WSIZE)
	{
		// 将右窗中的数据搬移到左窗中
		memcpy(_pWin, _pWin + WSIZE, WSIZE);
		memset(_pWin + WSIZE, 0, WSIZE);
		start -= WSIZE;

		// 更新哈希表
		_hash.Update();
		
		// 向右窗中补充一个窗口的待压缩数据
		if (!feof(fIN))
		{
			lookAhead += fread(_pWin + WSIZE, 1, WSIZE, fIN);
		}
	}
}


// 功能：合并压缩数据文件和标记信息文件
void LZ77::MergeFile(FILE* fOUT, ULL fileSize)
{
	// 读取标记信息文件中的内容，然后将结果写入到压缩文件中
	FILE* fInFlag = fopen("3.txt", "rb");
	size_t flagSize = 0;
	UCH* pReadBuff = new UCH[1024];
	while (true)
	{
		size_t readSize = fread(pReadBuff, 1, 1024, fInFlag);
		if (0 == readSize)
		{
			break;
		}
		fwrite(pReadBuff, 1, readSize, fOUT);
		flagSize += readSize;
	}  

	//  写标记文件大小
	fwrite(&flagSize, sizeof(flagSize), 1, fOUT);
	// 写原始文件大小
	fwrite(&fileSize, sizeof(fileSize), 1, fOUT);

	fclose(fInFlag);
	delete[] pReadBuff;
}

// 功能：写入标记信息
// 参数：
//         chFlag:该字节中的每个比特位用来区分当前字节是原字符还是长度：0->原字符；1->长度
//         bitCount：该字节中的多少个比特位已经被设置
//         isLength：代表该字节是原字符还是长度
// 返回值：void
void LZ77::WriteFlag(FILE* fOutFlag, UCH& chFlag, UCH& bitCount, bool isLength)
{
	chFlag <<= 1;
	if (isLength)
	{
		chFlag |= 1;
	}

	bitCount++;
	if (8 == bitCount)
	{
		// 将该标记写入压缩文件中
		fputc(chFlag, fOutFlag);
		chFlag = 0;
		bitCount = 0;
	}
}

// 功能：在当前匹配链中找最长匹配
// 参数：  hashAddr：匹配链的起始位置
//         matchStart：最长匹配串在滑动窗口中的起始位置
// 返回值：最长匹配串的长度
// 注意：可能会遇到环状链问题----可以设置祖达的匹配次数解决
//           匹配是在MAX_DIST范围内进行匹配的，太远的则不进行匹配
USH LZ77::LongestMatch(USH matchHead, USH& MatchDistance, USH start)
{
	UCH curMatchLength = 0;  // 一次匹配的长度
	UCH maxMatchLength = 0;
	UCH maxMatchCount = 255;  // 最大的匹配 次数，解决环状链
	USH curMatchStart = 0;  // 当前匹配在查找缓冲区中的起始位置

	// 在先行缓冲区中查找匹配时，不能太远 即不能超过MAX_DIST
	USH limit = start > MAX_DIST ? start - MAX_DIST : 0;

	do
	{
		// 匹配范围
		UCH* pstart = _pWin + start;
		UCH* pend = pstart + MAX_MATCH;

		// 查找缓冲区匹配串的起始
		UCH* pMatchStart = _pWin + matchHead;

		curMatchLength = 0;

		// 可以进行匹配
		while (pstart < pend && *pstart == *pMatchStart)
		{
			curMatchLength++;
			pstart++;
			pMatchStart++;
		}

		// 一次匹配结束
		if (curMatchLength > maxMatchLength)
		{
			maxMatchLength = curMatchLength;
			curMatchStart = matchHead;
		}
	} while ((matchHead = _hash.GetNextHashAddr(matchHead)) > limit && maxMatchCount--);
	
	MatchDistance = start - curMatchStart;
	return maxMatchLength;
}

//////////////////////////////////////////////////////////////////////
// 功能：基于LZ77的解压缩
//////////////////////////////////////////////////////////////////////
void  LZ77::UnCompressFile(const std::string& strFilePath)
{
	// 操作压缩文件 "原始数据" 的指针
	FILE* fIN = fopen(strFilePath.c_str(), "rb");
	if (nullptr == fIN)
	{
		std::cout << "压缩文件打开失败" << std::endl;
		return;
	}

	// 操作压缩文件 "标记数据" 的指针
	FILE* fInFlag = fopen(strFilePath.c_str(), "rb");
	if (nullptr == fInFlag)
	{
		std::cout << "压缩文件打开失败" << std::endl;
		return;
	}

	// 获取原始文件的大小
	ULL fileSize = 0;
	fseek(fInFlag, 0 - sizeof(fileSize), SEEK_END);
	fread(&fileSize, sizeof(fileSize), 1, fInFlag);

	// 获取标记信息的大小
	size_t flagSize = 0;
	fseek(fInFlag, 0 - sizeof(fileSize) - sizeof(flagSize), SEEK_END);
	fread(&flagSize, sizeof(fileSize), 1, fInFlag);

	// 将标记信息文件的文件指针移动到保存标记数据的起始位置
	fseek(fInFlag, 0 - sizeof(fileSize) - sizeof(flagSize) - flagSize, SEEK_END);
	
	std::string fileName = _FileName::GetFileNameOnly(strFilePath);
	std::string filePostFix;
	_Other::ReadLine(fIN, filePostFix);
	fileName += filePostFix;
	
	// 解压缩开始
	FILE* fOUT = fopen(fileName.c_str(), "wb");
	assert(fOUT);

	// 从已经解压好的数据中向前读取重复的字节
	FILE* fOutIn = fopen(fileName.c_str(), "rb");

	UCH bitCount = 0;
	UCH chFlag = 0;
	ULL encodeCount = 0;  // 已经解码的长度
	while (encodeCount < fileSize)
	{
		if (0 == bitCount)
		{
			chFlag = fgetc(fInFlag);
			bitCount = 8;
		}

		if (chFlag & 0x80)
		{
			// 距离长度对
			USH matchLength = fgetc(fIN) + 3;
			USH matchDistance = 0;
			fread(&matchDistance, sizeof(matchDistance), 1, fIN);
			
			// 清空缓冲区：系统会将缓冲区中的数据写入到文件中
			fflush(fOUT);

			// 更新已经解码字节数大小
			encodeCount += matchLength;

			UCH ch;
			fseek(fOutIn, 0 - matchDistance, SEEK_END);
			while (matchLength)
			{
				ch = fgetc(fOutIn);
				fputc(ch, fOUT);
				matchLength--;
				fflush(fOUT);
			}
		}
		else
		{
			// 原字符
			UCH ch = fgetc(fIN);
			fputc(ch, fOUT);
			encodeCount++;
		}

		chFlag <<= 1;
		bitCount--;
	}

	fclose(fIN);
	fclose(fInFlag);
	fclose(fOUT);
	fclose(fOutIn);
}