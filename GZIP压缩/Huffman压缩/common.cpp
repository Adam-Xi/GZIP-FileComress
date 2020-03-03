#include "common.h"

std::string _FileName::GetFilePostFix(const std::string& fileName)
{
	return fileName.substr(fileName.rfind('.'));
}


//��ȡ�ļ�����
std::string _FileName::GetFileNameOnly(const std::string& fileName)
{
	//return fileName.substr(fileName.rfind('/') + 1, fileName.rfind('.') - 2);
	int pos1 = fileName.find_last_of('/');
	int pos2 = fileName.find_last_of('.');
	return fileName.substr(pos1 + 1, pos2 - pos1 - 1);
}

//���з�װ����ʵ�֣�һ�ζ�ȡһ��
//getline()Ҳ������ʵ�ָù��ܣ�����������Ҫ�������������󣬶�����ָ�룬���Բ����ϴ˴�����
void _Other::ReadLine(FILE* fIn, std::string& strInfo)
{
	assert(fIn);

	while (!feof(fIn))
	{
		char ch = fgetc(fIn);
		if (ch == '\n')
		{
			break;
		}

		strInfo += ch;
	}
}