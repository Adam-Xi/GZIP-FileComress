#include "Huffman.hpp"
#include "FileCompressHuffman.h"

int main()
{
	FileCompressHuffman fc;
	fc.CompressFile("../��������/�Ͼ���ɽ��.jpg");
	fc.UnCompressFile("�Ͼ���ɽ��.gzip");

	return 0;
}