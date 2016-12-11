
#include <lz4.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#pragma comment(lib, "liblz4_static.lib")

constexpr int BLOCK_SIZE = 1024 * 64;

struct BATCHED_IMAGE_HEADER {
	int BatchSize;	// 最大何枚の画像が入るか
	int Stride;		// 1枚の画像のバイト数
	int Count;		// このファイルに何枚画像が入っているか
	int Width;		// 画像の幅
	int Height;		// 画像の高さ
};

void ReadAndDecompress(
	BATCHED_IMAGE_HEADER *pOutHeader,
	std::vector<unsigned char> &vecOut,
	std::istream &is)
{
	
	LZ4_streamDecode_t stream;
	LZ4_setStreamDecode(&stream, nullptr, 0);
	
	char doubleBuf[2][BLOCK_SIZE];
	int bufidx = 0;
	
	// 画像の情報を取得
	BATCHED_IMAGE_HEADER head;
	is.read((char*)&head, sizeof(BATCHED_IMAGE_HEADER));
	memcpy(pOutHeader, &head, sizeof(BATCHED_IMAGE_HEADER));
	
	// 展開済みのデータを入れるバッファ
	vecOut.resize(head.BatchSize * head.Stride);
	unsigned char *decompData = vecOut.data();
	
	while( true ) {
		char *buf = doubleBuf[bufidx];
		char compressedBuf[LZ4_COMPRESSBOUND(BLOCK_SIZE)];
		int compressedBytes = 0;
		
		is.read((char*)&compressedBytes, sizeof(int));
		if( compressedBytes == 0 ) break;	// 終端のゼロだったら終了
		
		memset(compressedBuf, 0, sizeof(compressedBuf));
		is.read(compressedBuf, compressedBytes);
		
		int decompBytes = LZ4_decompress_safe_continue(
			&stream, compressedBuf, buf,
			compressedBytes, BLOCK_SIZE);
		
		memcpy(decompData, buf, decompBytes);
		decompData += decompBytes;
		
	}
	
}

int main(int argc, char *argv[]) {
	
	if( argc < 2 ) {
		std::cerr << "Too less arguments\n"
			<< "    Usage: LZ4BatchedImageExtractor.exe <batchedimage.lz4>";
		return 1;
	}
	std::string batchname = argv[1];
	
	// ファイル読み込み&展開
	std::ifstream ifs(batchname, std::ios::binary);
	BATCHED_IMAGE_HEADER head;
	std::vector<unsigned char> vecDecompressedData;
	ReadAndDecompress(&head, vecDecompressedData, ifs);
	ifs.close();
	
	std::vector<unsigned char> vecImagePixels(head.Stride);
	for( int i = 0; i < head.Count; i++ ) {
		std::string filename =
			batchname.substr(0, batchname.rfind('.'))
			+ '_' + std::to_string(i) + ".png";
		std::cout << "\rwriting: " << filename;
		
		unsigned char *dst = vecImagePixels.data();
		const unsigned char *src = vecDecompressedData.data() + i;
		for( int j = 0; j < head.Stride; j++ ) {
			*dst = *src;
			dst++;
			src += head.BatchSize;
		}
		
		// PNGで書き出し
		stbi_write_png(filename.c_str(),
			head.Width, head.Height, 3,
			vecImagePixels.data(), 0);
		
	}
	
	std::cout << std::endl;
	
	return 0;
	
}


