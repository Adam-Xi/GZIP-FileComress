//����huffman���ļ�ѹ��
#include "FileCompressHuffman.h"

//���캯������ʼ��_fileInfo��Ϣ
FileCompressHuffman::FileCompressHuffman()
{
	_fileInfo.resize(256);
	for (int i = 0; i < 256; i++)
	{
		_fileInfo[i]._ch = i;
		_fileInfo[i]._count = 0;
	}
}

//�ļ�ѹ�����߼�
void FileCompressHuffman::CompressFile(const std::string& path)
{
	FILE* fIn = fopen(path.c_str(), "rb");  //�������ı���ʽ�򿪽���ѹ��

	if (nullptr == fIn)
	{
		assert(false);
		return;
	}
	//1��ͳ��Դ�ļ���ÿ���ַ����ֵĴ���
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

	//2�����ַ����ֵĴ���ΪȨֵ����huffman��
	HuffmanTree<CharInfo> tree(_fileInfo, CharInfo());  //����0�ε���Ч���ַ����������huffman���Ĺ���

	//3����ȡÿ���ַ��ı���
	GenerateHuffmanCode(tree.GetRoot());

	fseek(fIn, 0, SEEK_SET);  // ���¶�λ�ļ�ָ��

	//4���û�ȡ���ı������¸�дԴ�ļ�
	//��ȡѹ������ļ����ƣ���Ϊʹ��gzip�㷨ѹ��������Լ��ѹ������ļ�������".gzip"��β
	// std::string NewFileNameOnly = _FileName::GetFileNameOnly(path);
	// NewFileNameOnly += ".gzip";
	//std::cout << NewFileNameOnly << std::endl;  //����

	std::string newFileName = _FileName::GetFileNameOnly(path);
	newFileName += ".gzip";
	FILE* fOut = fopen(newFileName.c_str(), "wb");  // ��������ļ�(ѹ�����ļ�)
	if (nullptr == fOut)
	{
		assert(false);
		return;
	}

	// дͷ����Ϣ
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
		//�����ֽڵı���Զ�ȡ�������ݽ�����д
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
					fputc(ch, fOut);  //���ļ���һ��д��һ���ֽ�
					bitCount = 0;
					ch = 0;
				}
			}
		}
	}
	//���һ��ch�п��ܲ���8��bitλ
	if (bitCount < 8)
	{
		ch <<= (8 - bitCount);
		fputc(ch, fOut);
	}

	delete[] pReadBuff;
	fclose(fIn);
	fclose(fOut);
}

//����huffman����
void FileCompressHuffman::GenerateHuffmanCode(HuffmanTreeNode<CharInfo>* pRoot)
{
	if (nullptr == pRoot)
	{
		return;
	}
	GenerateHuffmanCode(pRoot->_pLeft);
	GenerateHuffmanCode(pRoot->_pRight);

	if (nullptr == pRoot->_pLeft && nullptr == pRoot->_pRight)
	{//Ҷ�ӽڵ㣬Ҫ������ַ�
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

//дѹ���ļ�ͷ����Ϣ���Դ���Ϊ��ѹ������
void FileCompressHuffman::WriteHead(FILE*  fOut, const std::string& fileName)
{
	assert(fOut);

	//д�ļ���׺
	std::string strHead;
	// strHead += _FileName::GetFilePostFix(fileName);
	// strHead += '\n';  // �������㷨�ϲ����ʱhuffmanѹ��ʱ������Ҫ��¼Դ�ļ����ƣ���Ϊ��LZ77ѹ�����Ѿ���¼����

	//д�ַ���Ϣ�������� �� ������Ϣ
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

//��ѹ�����߼�
void FileCompressHuffman::UnCompressFile(const std::string& path, std::string& huff_newFileName)
{
	FILE* fIn = fopen(path.c_str(), "rb");
	if (nullptr == fIn)
	{
		assert(false);
		return;
	}

	// ��ȡ�ļ���׺----����Ҫ�������ֽ�ѹ���㷨�ϲ�������huffman��ѹ�������LZ77ʱ��ȡ�ļ���׺
	// std::string strFilePostFix;
	// ReadLine(fIn, strFilePostFix);

	// ��ȡ�ַ���Ϣ��������
	std::string strCount;
	_Other::ReadLine(fIn, strCount);
	int lineCount = atoi(strCount.c_str());

	// ��ȡ�ַ���Ϣ
	for (int i = 0; i < lineCount; i++)
	{
		std::string strChCount;
		_Other::ReadLine(fIn, strChCount);
		if (strChCount.empty())
		{  //��ȡ������
			strChCount += '\n';
			_Other::ReadLine(fIn, strChCount);
		}

		// A:1
		_fileInfo[(unsigned char)strChCount[0]]._count = atoi(strChCount.c_str() + 2);
	}

	// ��ȡ��ѹ���ļ�������
	huff_newFileName = _FileName::GetFileNameOnly(path);
	huff_newFileName += "-����";
	huff_newFileName += ".lzp";
	//std::cout << NewFileName << std::endl;  //����

	// ��ԭhuffman��
	HuffmanTree<CharInfo> tree(_fileInfo, CharInfo());

	FILE* fOut = fopen(huff_newFileName.c_str(), "wb");  //�������ı��ļ���ʽ���ļ�����д����ѹ����
	if (nullptr == fOut)
	{
		assert(false);
		return;
	}

	// ��ѹ��
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
			// ֻ�轫һ���ֽ���8������λ��������
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

