#include "Huffman.hpp"
#include "FileCompressHuffman.h"

int main()
{
	FileCompressHuffman fc;
	fc.CompressFile("../²âÊÔÓÃÀı/Ëû‚ƒ.mp3");
	fc.UnCompressFile("Ëû‚ƒ.gzip");

	return 0;
}