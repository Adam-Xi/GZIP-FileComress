#include "common.h"

std::string _FileName::GetFilePostFix(const std::string& fileName)
{
	return fileName.substr(fileName.rfind('.'));
}


//获取文件名称
std::string _FileName::GetFileNameOnly(const std::string& fileName)
{
	//return fileName.substr(fileName.rfind('/') + 1, fileName.rfind('.') - 2);
	int pos1 = fileName.find_last_of('/');
	int pos2 = fileName.find_last_of('.');
	return fileName.substr(pos1 + 1, pos2 - pos1 - 1);
}

//自行封装函数实现：一次读取一行
//getline()也可以是实现该功能，不过参数需要传递输入流对象，而不是指针，所以不符合此处需求
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