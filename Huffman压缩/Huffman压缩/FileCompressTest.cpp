#include "Huffman.hpp"
#include "FileCompressHuffman.h"

int main()
{
	FileCompressHuffman fc;
	fc.CompressFile("../��������/����.mp3");
	fc.UnCompressFile("����.gzip");

	return 0;
}