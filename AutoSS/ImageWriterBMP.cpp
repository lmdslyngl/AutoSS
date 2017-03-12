
#include "ImageWriterBMP.h"
#include <Windows.h>
#include <fstream>
#include <vector>

// 画像を保存
void ImageWriterBMP::Write(
	const std::string &filename,
	int width, int height,
	const unsigned char *data,
	size_t length)
{
	
	BITMAPINFOHEADER info;
	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = width;
	info.biHeight = height;
	info.biPlanes = 1;
	info.biBitCount = 24;
	info.biCompression = 0;
	info.biSizeImage = 0;
	info.biXPelsPerMeter = 0;
	info.biYPelsPerMeter = 0;
	info.biClrUsed = 0;
	info.biClrImportant = 0;
	
	BITMAPFILEHEADER header;
	memcpy(&header.bfType, "BM", 2);
	header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + info.biSizeImage;
	header.bfReserved1 = 0;
	header.bfReserved2 = 0;
	header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	
	std::ofstream ofs(filename, std::ios::binary);
	ofs.write((const char*)&header, sizeof(BITMAPFILEHEADER));
	ofs.write((const char*)&info, sizeof(BITMAPINFOHEADER));
	
	int pitch = 3 * width;
	int paddedPitch = pitch + ((4 - (pitch % 4)) % 4);	// パディングを含んだピッチ
	std::vector<unsigned char> vecRowBuffer(paddedPitch);
	std::fill(std::begin(vecRowBuffer), std::end(vecRowBuffer), 0);
	const unsigned char *cursor = data;
	unsigned char *rowcursor = nullptr;
	
	for( int r = height - 1; 0 <= r; r-- ) {
		// 上下反転，RGB -> BGRに並べ替えながらコピー
		rowcursor = vecRowBuffer.data();
		cursor = data + pitch * r;
		for( int c = 0; c < width; c++ ) {
			rowcursor[2] = cursor[0];
			rowcursor[1] = cursor[1];
			rowcursor[0] = cursor[2];
			rowcursor += 3;
			cursor += 3;
		}
		ofs.write((const char*)vecRowBuffer.data(), vecRowBuffer.size());
	}
	
	ofs.close();
	
}

// 連続撮影開始
void ImageWriterBMP::BeginCapture() { }

// 連続撮影終了
void ImageWriterBMP::EndCapture() { }

