#pragma once
#pragma warning(disable:4996)

#include <assert.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>

namespace _FileName
{
	//获取文件名称后缀
	std::string GetFilePostFix(const std::string& fileName);

	//获取文件名称
	std::string GetFileNameOnly(const std::string& fileName);
}

namespace _Other
{
	void ReadLine(FILE* fIn, std::string& strInfo);
}