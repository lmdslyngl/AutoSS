
#include <lz4.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <png.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#pragma comment(lib, "liblz4_static.lib")
#pragma comment(lib, "libpng.lib")
#pragma comment(lib, "zlib.lib")

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

void PNGWriteFunc(png_structp pPng, png_bytep data, png_size_t length) {
	auto pVecBuffer = (std::vector<unsigned char>*)png_get_io_ptr(pPng);
	int start = pVecBuffer->size();
	pVecBuffer->resize(pVecBuffer->size() + length);
	memcpy(pVecBuffer->data() + start, data, length);
}
void PNGFlushFunc(png_structp pPng) { }

void WritePNG(
	std::vector<unsigned char> &vecOut,
	const std::vector<unsigned char> &vecIn,
	int width, int height)
{
	
	png_structp pPng = nullptr;
	pPng = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if( !pPng ) {
		throw std::exception("Failed to png_create_write_struct");
	}
	
	png_infop pInfo = nullptr;
	pInfo = png_create_info_struct(pPng);
	if( !pInfo ) {
		throw std::exception("Failed to png_create_info_struct");
	}
	
	// メモリ書き込みコールナック設定
	png_set_write_fn(pPng, &vecOut, PNGWriteFunc, PNGFlushFunc);
	
	// 画像情報設定
	png_set_IHDR(
		pPng, pInfo,
		width, height, 8,
		PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT);
	png_write_info(pPng, pInfo);
	
	// 画像書き出し
	const unsigned char *cursor = vecIn.data();
	int pitch = width * 3;
	for( int i = 0; i < height; i++ ) {
		png_write_row(pPng, cursor);
		cursor += pitch;
	}
	png_write_end(pPng, pInfo);
	
	png_free_data(pPng, pInfo, PNG_FREE_ALL, -1);
	png_destroy_write_struct(&pPng, nullptr);
	
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
		std::vector<unsigned char> vecData;
		WritePNG(vecData, vecImagePixels, head.Width, head.Height);
		std::ofstream ofs(filename, std::ios::binary);
		ofs.write((const char*)vecData.data(), vecData.size());
		ofs.close();
		
	}
	
	std::cout << std::endl;
	
	return 0;
	
}


