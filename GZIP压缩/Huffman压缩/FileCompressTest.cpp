#include "common.h"
#include "FileCompressHuffman.h"
#include "LZ77.h"

int main()
{
	FileCompressHuffman huff;
	LZ77 lz;

# if 0
	std::string path("./1.txt");	std::string file;
	// —πÀı
	lz.CompressFile(path, file);
	huff.CompressFile(path);
	// remove(file.c_str());
# else
	std::string path("./1.gzip");
	std::string file;
	// Ω‚—πÀı
	huff.UnCompressFile(path, file);
	lz.UnCompressFile("./1-∏±±æ.lzp");
	remove(file.c_str());
# endif

	return 0;
}
