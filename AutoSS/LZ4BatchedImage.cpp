
#include "LZ4BatchedImage.h"
#include <lz4.h>
#include <fstream>
#include <algorithm>

void LZ4BatchedImage::Setup(int width, int height, int batchSize) {
	this->Width = width;
	this->Height = height;
	Count = 0;
	this->BatchSize = batchSize;
	Stride = width * height * 3;
	vecBuffer.resize(Stride * BatchSize);
}

void LZ4BatchedImage::Add(const unsigned char *data, int length) {
	// 既にバッファがいっぱいだったときは，何もしない
	if( IsFull() ) return;
	
	unsigned char *dst = vecBuffer.data() + Count;
	for( int i = 0; i < length; i++ ) {
		*dst = *data;
		data++;
		dst += BatchSize;
	}
	Count++;
	
}

void LZ4BatchedImage::CompressedWrite(const std::string &filename) {
	LZ4_stream_t stream;
	LZ4_resetStream(&stream);
	
	constexpr int BLOCK_SIZE = 64 * 1024;
	char doubleBuf[2][BLOCK_SIZE];	// 圧縮で使うダブルバッファ
	int bufidx = 0;
	
	unsigned char *src = vecBuffer.data();
	unsigned char *srcEnd = src + vecBuffer.size();
	
	std::ofstream ofs(filename, std::ios::binary);
	
	// 画像のデータを書き出し
	ofs.write((const char*)&BatchSize, sizeof(int));
	ofs.write((const char*)&Stride, sizeof(int));
	ofs.write((const char*)&Count, sizeof(int));
	ofs.write((const char*)&Width, sizeof(int));
	ofs.write((const char*)&Height, sizeof(int));
	
	while( true ) {
		char *buf = doubleBuf[bufidx];
		memset(buf, 0, BLOCK_SIZE);
		
		int available = srcEnd - src;
		int compressSize = std::min(available, BLOCK_SIZE);
		memcpy(buf, src, compressSize);
		src += compressSize;
		
		char compressedBuffer[LZ4_COMPRESSBOUND(BLOCK_SIZE)];
		int compressedBytes = LZ4_compress_fast_continue(
			&stream, buf, compressedBuffer,
			compressSize, sizeof(compressedBuffer), 1);
		
		// 圧縮後のサイズ，圧縮データを書き込む
		ofs.write((const char*)&compressedBytes, sizeof(int));
		ofs.write(compressedBuffer, compressedBytes);
		
		bufidx = (bufidx + 1) % 2;
		if( src == srcEnd ) break;
		
	}
	
	// 終端のゼロを書く
	int zero = 0;
	ofs.write((const char*)&zero, sizeof(int));
	
	ofs.close();
	
}


