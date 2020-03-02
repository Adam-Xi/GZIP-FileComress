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
// ���ܣ�����LZ77��ѹ��
//////////////////////////////////////////////////////////////////////
void LZ77::CompressFile(const std::string& strFilePath, std::string& lz_newFileNameOnly)
{
	// ����ļ��ܴ�СС�� MIN_MATCH ���򲻽���ƥ��
	FILE* fIN = fopen(strFilePath.c_str(), "rb");
	if (nullptr == fIN)
	{
		std::cout << "���ļ�ʧ��" << std::endl;
		return;
	}
	// ��ȡ�ļ���С
	fseek(fIN, 0, SEEK_END);
	ULL fileSize = ftell(fIN);

	if (fileSize <= MIN_MATCH)
	{
		std::cout << "�ļ�̫С��������ѹ��" << std::endl;
		return;
	}

	fseek(fIN, 0, SEEK_SET);

	// ��ѹ���ļ��ж�ȡһ�������������ݵ�������
	size_t lookAhead = fread(_pWin, 1, WSIZE * 2, fIN);  // lookAhead��ʾ���л�������ʣ���ַ�����
	
	USH hashAddr = 0;
	// ������ʼhashAddr
	for (USH i = 0; i < MIN_MATCH - 1; i++)
	{
		_hash.HashFunc(hashAddr, _pWin[i]);
	}

	// ѹ��
	lz_newFileNameOnly = _FileName::GetFileNameOnly(strFilePath);
	lz_newFileNameOnly += ".lzp";
	FILE* fOUT = fopen(lz_newFileNameOnly.c_str(), "wb");
	if (nullptr == fOUT)
	{
		std::cout << "���ļ�ʧ��" << std::endl;
		return;
	}

	// ���ļ���׺����д��ѹ���ļ���
	std::string filePostFix = _FileName::GetFilePostFix(strFilePath);
	filePostFix += '\n';
	fwrite(filePostFix.c_str(), strlen(filePostFix.c_str()), 1, fOUT);
	fflush(fOUT);

	// ������ƥ����صı���
	USH matchHead = 0;
	USH start = 0;  // �Ӵ�����������ƥ��
	USH curMatchLength = 0;  // ��ǰ�����ƥ�䳤��
	USH curMatchDistance = 0;  // ��ǰ���ƥ�䳤�ȶ�Ӧ�ľ���

	// ��д������Ϣ��صı���
	UCH chFlag = 0;
	UCH bitCount = 0;
	// bool isLength = false;
	FILE* fOutFlag = fopen("3.txt", "wb");  // �������Ϣд��3.txt��
	assert(fOutFlag);

	while (lookAhead)
	{ 
		// ����ǰ�����ַ�(start, start + 1, start + 2)���뵽��ϣ���У�����ȡƥ��ͷ
		_hash.Insert(matchHead, _pWin[start + 2], start, hashAddr);

		curMatchLength = 0;
		curMatchDistance = 0;

		// ��֤�ڲ��һ��������Ƿ��ҵ�ƥ��
		if (matchHead)
		{
			// ˳��ƥ�������ƥ��
			curMatchLength = LongestMatch(matchHead, curMatchDistance, start);
		}

		// ��֤��ƥ�������Ƿ��ҵ�ƥ��
		if (curMatchLength < MIN_MATCH)
		{
			// �ڲ��һ�������δ���ҵ��ظ��ַ���
			// ��startλ�õ��ַ�д�뵽ѹ���ļ���
			fputc(_pWin[start], fOUT);

			// д��ǰԭ�ַ���Ӧ�ı��
			WriteFlag(fOutFlag, chFlag, bitCount, false);
			start++;
			lookAhead--;
		}
		else
		{
			// �ҵ�ƥ�� 
			// ��<����,����>д�뵽ѹ���ļ���
			// д����
			UCH chLength = curMatchLength - 3;
			fputc(chLength, fOUT);
			// д����
			fwrite(&curMatchDistance, sizeof(curMatchDistance), 1, fOUT);
			// д��ǰԭ�ַ���Ӧ�ı��
			WriteFlag(fOutFlag, chFlag, bitCount, true);

			// �������л�������ʣ���ַ��ĸ���
			lookAhead -= curMatchLength;

			// ���Ѿ�ƥ����ַ�����������һ����뵽��ϣ����
			curMatchLength--;
			while (curMatchLength)
			{
				start++;
				_hash.Insert(matchHead, _pWin[start], start, hashAddr);
				curMatchLength--;
			}
			start++;  // ��ѭ���У�start�ټ���һ��
		}

		// ������л�������ʣ���ַ��ĸ���
		if (lookAhead <= MIN_LOOKAHEAD)
		{
			FillWindow(fIN, lookAhead, start);
		}
	}

	//���λ������8������λ
	if (bitCount > 0 && bitCount < 8)
	{
		chFlag <<= (8 - bitCount);
		fputc(chFlag, fOutFlag);
	}

	fclose(fOutFlag);

	// ��ѹ�������ļ��ͱ����Ϣ�ļ����кϲ�
	MergeFile(fOUT, fileSize);

	fclose(fIN);
	fclose(fOUT);

	// ��������������Ϣ����ʱ�ļ�ɾ����
	remove("./3.txt");
}

void LZ77::FillWindow(FILE* fIN, size_t& lookAhead, USH& start)
{
	// ѹ���Ѿ����е��Ҵ������л�������ʣ�����ݲ���MIN_LOOKAHEAD
	if (start >= WSIZE)
	{
		// ���Ҵ��е����ݰ��Ƶ�����
		memcpy(_pWin, _pWin + WSIZE, WSIZE);
		memset(_pWin + WSIZE, 0, WSIZE);
		start -= WSIZE;

		// ���¹�ϣ��
		_hash.Update();
		
		// ���Ҵ��в���һ�����ڵĴ�ѹ������
		if (!feof(fIN))
		{
			lookAhead += fread(_pWin + WSIZE, 1, WSIZE, fIN);
		}
	}
}


// ���ܣ��ϲ�ѹ�������ļ��ͱ����Ϣ�ļ�
void LZ77::MergeFile(FILE* fOUT, ULL fileSize)
{
	// ��ȡ�����Ϣ�ļ��е����ݣ�Ȼ�󽫽��д�뵽ѹ���ļ���
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

	//  д����ļ���С
	fwrite(&flagSize, sizeof(flagSize), 1, fOUT);
	// дԭʼ�ļ���С
	fwrite(&fileSize, sizeof(fileSize), 1, fOUT);

	fclose(fInFlag);
	delete[] pReadBuff;
}

// ���ܣ�д������Ϣ
// ������
//         chFlag:���ֽ��е�ÿ������λ�������ֵ�ǰ�ֽ���ԭ�ַ����ǳ��ȣ�0->ԭ�ַ���1->����
//         bitCount�����ֽ��еĶ��ٸ�����λ�Ѿ�������
//         isLength��������ֽ���ԭ�ַ����ǳ���
// ����ֵ��void
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
		// ���ñ��д��ѹ���ļ���
		fputc(chFlag, fOutFlag);
		chFlag = 0;
		bitCount = 0;
	}
}

// ���ܣ��ڵ�ǰƥ���������ƥ��
// ������  hashAddr��ƥ��������ʼλ��
//         matchStart���ƥ�䴮�ڻ��������е���ʼλ��
// ����ֵ���ƥ�䴮�ĳ���
// ע�⣺���ܻ�������״������----������������ƥ��������
//           ƥ������MAX_DIST��Χ�ڽ���ƥ��ģ�̫Զ���򲻽���ƥ��
USH LZ77::LongestMatch(USH matchHead, USH& MatchDistance, USH start)
{
	UCH curMatchLength = 0;  // һ��ƥ��ĳ���
	UCH maxMatchLength = 0;
	UCH maxMatchCount = 255;  // ����ƥ�� �����������״��
	USH curMatchStart = 0;  // ��ǰƥ���ڲ��һ������е���ʼλ��

	// �����л������в���ƥ��ʱ������̫Զ �����ܳ���MAX_DIST
	USH limit = start > MAX_DIST ? start - MAX_DIST : 0;

	do
	{
		// ƥ�䷶Χ
		UCH* pstart = _pWin + start;
		UCH* pend = pstart + MAX_MATCH;

		// ���һ�����ƥ�䴮����ʼ
		UCH* pMatchStart = _pWin + matchHead;

		curMatchLength = 0;

		// ���Խ���ƥ��
		while (pstart < pend && *pstart == *pMatchStart)
		{
			curMatchLength++;
			pstart++;
			pMatchStart++;
		}

		// һ��ƥ�����
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
// ���ܣ�����LZ77�Ľ�ѹ��
//////////////////////////////////////////////////////////////////////
void  LZ77::UnCompressFile(const std::string& strFilePath)
{
	// ����ѹ���ļ� "ԭʼ����" ��ָ��
	FILE* fIN = fopen(strFilePath.c_str(), "rb");
	if (nullptr == fIN)
	{
		std::cout << "ѹ���ļ���ʧ��" << std::endl;
		return;
	}

	// ����ѹ���ļ� "�������" ��ָ��
	FILE* fInFlag = fopen(strFilePath.c_str(), "rb");
	if (nullptr == fInFlag)
	{
		std::cout << "ѹ���ļ���ʧ��" << std::endl;
		return;
	}

	// ��ȡԭʼ�ļ��Ĵ�С
	ULL fileSize = 0;
	fseek(fInFlag, 0 - sizeof(fileSize), SEEK_END);
	fread(&fileSize, sizeof(fileSize), 1, fInFlag);

	// ��ȡ�����Ϣ�Ĵ�С
	size_t flagSize = 0;
	fseek(fInFlag, 0 - sizeof(fileSize) - sizeof(flagSize), SEEK_END);
	fread(&flagSize, sizeof(fileSize), 1, fInFlag);

	// �������Ϣ�ļ����ļ�ָ���ƶ������������ݵ���ʼλ��
	fseek(fInFlag, 0 - sizeof(fileSize) - sizeof(flagSize) - flagSize, SEEK_END);
	
	std::string fileName = _FileName::GetFileNameOnly(strFilePath);
	std::string filePostFix;
	_Other::ReadLine(fIN, filePostFix);
	fileName += filePostFix;
	
	// ��ѹ����ʼ
	FILE* fOUT = fopen(fileName.c_str(), "wb");
	assert(fOUT);

	// ���Ѿ���ѹ�õ���������ǰ��ȡ�ظ����ֽ�
	FILE* fOutIn = fopen(fileName.c_str(), "rb");

	UCH bitCount = 0;
	UCH chFlag = 0;
	ULL encodeCount = 0;  // �Ѿ�����ĳ���
	while (encodeCount < fileSize)
	{
		if (0 == bitCount)
		{
			chFlag = fgetc(fInFlag);
			bitCount = 8;
		}

		if (chFlag & 0x80)
		{
			// ���볤�ȶ�
			USH matchLength = fgetc(fIN) + 3;
			USH matchDistance = 0;
			fread(&matchDistance, sizeof(matchDistance), 1, fIN);
			
			// ��ջ�������ϵͳ�Ὣ�������е�����д�뵽�ļ���
			fflush(fOUT);

			// �����Ѿ������ֽ�����С
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
			// ԭ�ַ�
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