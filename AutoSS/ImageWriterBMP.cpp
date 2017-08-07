
#include "ImageWriterBMP.h"
#include <Windows.h>
#include <fstream>
#include <vector>

// 画像を保存
void ImageWriterBMP::Write(
	const std::wstring &filename,
	int width, int height,
	const unsigned char *data,
	size_t length)
{
	
	int pitch = 3 * width;
	int numPadding = (4 - (pitch % 4)) % 4;
	
	BITMAPINFOHEADER info;
	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = width;
	info.biHeight = height;
	info.biPlanes = 1;
	info.biBitCount = 24;
	info.biCompression = 0;
	info.biSizeImage = (pitch + numPadding) * height;
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
	
	std::ofstream ofs(filename, std::ios::binary | std::ios::trunc);
	vecStreamBuf.resize(width * height * 4);
	ofs.rdbuf()->pubsetbuf(vecStreamBuf.data(), vecStreamBuf.size());
	
	ofs.write((const char*)&header, sizeof(BITMAPFILEHEADER));
	ofs.write((const char*)&info, sizeof(BITMAPINFOHEADER));
	
	unsigned char zeros[4] = { 0, 0, 0, 0 };
	
	for( int r = height - 1; 0 <= r; r-- ) {
		// 上下反転しながら書き出し
		ofs.write((const char*)data + pitch * r, pitch);
		ofs.write((const char*)zeros, numPadding);
	}
	
	ofs.close();
	
}

// 連続撮影開始
void ImageWriterBMP::BeginCapture() { }

// 連続撮影終了
void ImageWriterBMP::EndCapture() { }

