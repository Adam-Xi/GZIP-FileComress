//基于huffman的文件压缩
#include "FileCompressHuffman.h"

//构造函数，初始化_fileInfo信息
FileCompressHuffman::FileCompressHuffman()
{
	_fileInfo.resize(256);
	for (int i = 0; i < 256; i++)
	{
		_fileInfo[i]._ch = i;
		_fileInfo[i]._count = 0;
	}
}

//文件压缩主逻辑
void FileCompressHuffman::CompressFile(const std::string& path)
{
	FILE* fIn = fopen(path.c_str(), "rb");  //必须以文本形式打开进行压缩

	if (nullptr == fIn)
	{
		assert(false);
		return;
	}
	//1、统计源文件中每个字符出现的次数
	unsigned char* pReadBuff = new unsigned char[1024];
	size_t readSize = 0;
	while (true)
	{
		readSize = fread(pReadBuff, 1, 1024, fIn);
		if (0 == readSize)
		{
			break;
		}
		for (size_t i = 0; i < readSize; i++)
		{
			_fileInfo[pReadBuff[i]]._count++;
		}
	}
	//==============================================

	//2、以字符出现的次数为权值创建huffman树
	HuffmanTree<CharInfo> tree(_fileInfo, CharInfo());  //出现0次的无效的字符将不会参与huffman树的构造

	//3、获取每个字符的编码
	GenerateHuffmanCode(tree.GetRoot());

	fseek(fIn, 0, SEEK_SET);  // 重新定位文件指针

	//4、用获取到的编码重新改写源文件
	//获取压缩后的文件名称，因为使用gzip算法压缩，所以约定压缩后的文件名称以".gzip"结尾
	// std::string NewFileNameOnly = _FileName::GetFileNameOnly(path);
	// NewFileNameOnly += ".gzip";
	//std::cout << NewFileNameOnly << std::endl;  //测试

	std::string newFileName = _FileName::GetFileNameOnly(path);
	newFileName += ".gzip";
	FILE* fOut = fopen(newFileName.c_str(), "wb");  // 最终输出文件(压缩后文件)
	if (nullptr == fOut)
	{
		assert(false);
		return;
	}

	// 写头部信息
	WriteHead(fOut, path);

	unsigned char ch = 0;
	int bitCount = 0;
	while (true)
	{
		readSize = fread(pReadBuff, 1, 1024, fIn);
		if (0 == readSize)
		{
			break;
		}
		//根据字节的编码对读取到的内容进行重写
		for (size_t i = 0; i < readSize; i++)
		{
			std::string strCode = _fileInfo[pReadBuff[i]]._strCode;
			for (size_t j = 0; j < strCode.size(); j++)
			{
				ch <<= 1;
				if ('1' == strCode[j])
				{
					ch |= 1;
				}
				bitCount++;
				if (8 == bitCount)
				{
					fputc(ch, fOut);  //往文件中一次写入一个字节
					bitCount = 0;
					ch = 0;
				}
			}
		}
	}
	//最后一次ch中可能不够8个bit位
	if (bitCount < 8)
	{
		ch <<= (8 - bitCount);
		fputc(ch, fOut);
	}

	delete[] pReadBuff;
	fclose(fIn);
	fclose(fOut);
}

//生成huffman编码
void FileCompressHuffman::GenerateHuffmanCode(HuffmanTreeNode<CharInfo>* pRoot)
{
	if (nullptr == pRoot)
	{
		return;
	}
	GenerateHuffmanCode(pRoot->_pLeft);
	GenerateHuffmanCode(pRoot->_pRight);

	if (nullptr == pRoot->_pLeft && nullptr == pRoot->_pRight)
	{//叶子节点，要编码的字符
		std::string strCode;
		HuffmanTreeNode<CharInfo>* pCur = pRoot;
		HuffmanTreeNode<CharInfo>* pParent = pCur->_pParent;

		while (pParent)
		{
			if (pCur == pParent->_pLeft)
			{
				strCode += '0';
			}
			else
			{
				strCode += '1';
			}
			pCur = pParent;
			pParent = pCur->_pParent;
		}
		reverse(strCode.begin(), strCode.end());

		_fileInfo[pRoot->_Weight._ch]._strCode = strCode;
	}
}

//写压缩文件头部信息，以此作为解压的依据
void FileCompressHuffman::WriteHead(FILE*  fOut, const std::string& fileName)
{
	assert(fOut);

	//写文件后缀
	std::string strHead;
	// strHead += _FileName::GetFilePostFix(fileName);
	// strHead += '\n';  // 将两个算法合并后此时huffman压缩时并不需要记录源文件名称，因为在LZ77压缩中已经记录过了

	//写字符信息的总行数 及 次数信息
	size_t lineCount = 0;
	std::string strChCount;
	char szValue[32] = { 0 };
	for (int i = 0; i < 256; i++)
	{
		if (_fileInfo[i]._count)
		{
			lineCount++;
			strChCount += _fileInfo[i]._ch;
			strChCount += ':';
			//itoa(_fileInfo[i]._count, szValue, 10);
			strChCount += std::to_string(_fileInfo[i]._count);
			strChCount += '\n';
		}
	}

	itoa(lineCount, szValue, 10);
	strHead += szValue;
	strHead += '\n';

	strHead += strChCount;

	fwrite(strHead.c_str(), 1, strHead.size(), fOut);
}

//解压缩主逻辑
void FileCompressHuffman::UnCompressFile(const std::string& path, std::string& huff_newFileName)
{
	FILE* fIn = fopen(path.c_str(), "rb");
	if (nullptr == fIn)
	{
		assert(false);
		return;
	}

	// 获取文件后缀----不需要，将两种解压缩算法合并后，先用huffman解压缩，最后LZ77时获取文件后缀
	// std::string strFilePostFix;
	// ReadLine(fIn, strFilePostFix);

	// 获取字符信息的总行数
	std::string strCount;
	_Other::ReadLine(fIn, strCount);
	int lineCount = atoi(strCount.c_str());

	// 获取字符信息
	for (int i = 0; i < lineCount; i++)
	{
		std::string strChCount;
		_Other::ReadLine(fIn, strChCount);
		if (strChCount.empty())
		{  //读取到空行
			strChCount += '\n';
			_Other::ReadLine(fIn, strChCount);
		}

		// A:1
		_fileInfo[(unsigned char)strChCount[0]]._count = atoi(strChCount.c_str() + 2);
	}

	// 获取解压后文件的名称
	huff_newFileName = _FileName::GetFileNameOnly(path);
	huff_newFileName += "-副本";
	huff_newFileName += ".lzp";
	//std::cout << NewFileName << std::endl;  //测试

	// 还原huffman树
	HuffmanTree<CharInfo> tree(_fileInfo, CharInfo());

	FILE* fOut = fopen(huff_newFileName.c_str(), "wb");  //必须以文本文件形式打开文件进行写（解压缩）
	if (nullptr == fOut)
	{
		assert(false);
		return;
	}

	// 解压缩
	char* pReadBuff = new char[1024];
	char ch = 0;
	HuffmanTreeNode<CharInfo>* pCur = tree.GetRoot();
	long long fileSize = pCur->_Weight._count;
	size_t unCount = 0;
	while (true)
	{
		size_t readSize = fread(pReadBuff, 1, 1024, fIn);
		if (0 == readSize)
		{
			break;
		}

		for (size_t i = 0; i < readSize; i++)
		{
			// 只需将一个字节中8个比特位单独处理
			ch = pReadBuff[i];
			for (int pos = 0; pos < 8; pos++)
			{
				if (nullptr == pCur->_pLeft && nullptr == pCur->_pRight)
				{
					//unCount++;
					fputc(pCur->_Weight._ch, fOut);
					if (unCount == fileSize)
					{
						break;
					}
					pCur = tree.GetRoot();
				}
				if (ch & 0x80)
				{
					pCur = pCur->_pRight;
				}
				else
				{
					pCur = pCur->_pLeft;
				}
				ch <<= 1;
				if (nullptr == pCur->_pLeft && nullptr == pCur->_pRight)
				{
					unCount++;
					fputc(pCur->_Weight._ch, fOut);
					if (unCount == fileSize)
					{
						break;
					}
					pCur = tree.GetRoot();
				}
			}  // end for pos
		}  // end for i
	}  // end while

	delete[] pReadBuff;
	fclose(fIn);
	fclose(fOut);
}

