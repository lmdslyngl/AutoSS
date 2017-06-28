
#include "ImageWriterPPM.h"
#include <fstream>
#include <vector>

// 画像を保存
void ImageWriterPPM::Write(
	const std::wstring &filename,
	int width, int height,
	const unsigned char *data,
	size_t length)
{
	
	std::ofstream ofs(filename, std::ios::binary);
	
	// PPMヘッダ
	char header[32];
	std::fill(std::begin(header), std::end(header), 0);
	sprintf_s(header, "P6\n%d %d\n%d\n", width, height, 255);
	ofs.write(header, strlen(header));
	
	// 画像データ
	std::vector<unsigned char> vecRowBuffer(width * 3);
	std::fill(std::begin(vecRowBuffer), std::end(vecRowBuffer), 0);
	unsigned char *rowdst = nullptr;
	const unsigned char *src = data;
	
	// BGRをRGBに変換しながら書き出し
	for( int r = 0; r < height; r++ ) {
		rowdst = vecRowBuffer.data();
		for( int c = 0; c < width; c++ ) {
			rowdst[0] = src[2];
			rowdst[1] = src[1];
			rowdst[2] = src[0];
			rowdst += 3;
			src += 3;
		}
		ofs.write((const char*)vecRowBuffer.data(), vecRowBuffer.size());
	}
	
	ofs.close();
	
}

// 連続撮影開始
void ImageWriterPPM::BeginCapture() { }

// 連続撮影終了
void ImageWriterPPM::EndCapture() { }

