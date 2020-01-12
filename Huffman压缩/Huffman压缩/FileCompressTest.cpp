#include "Huffman.hpp"
#include "FileCompressHuffman.h"

int main()
{
	FileCompressHuffman fc;
	fc.CompressFile("../测试用例/南京中山陵.jpg");
	fc.UnCompressFile("南京中山陵.gzip");

	return 0;
}