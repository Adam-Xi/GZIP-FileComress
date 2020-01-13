#include "Huffman.hpp"
#include "FileCompressHuffman.h"

int main()
{
	FileCompressHuffman fc;
	fc.CompressFile("1.txt");
	fc.UnCompressFile("1.gzip");

	return 0;
}