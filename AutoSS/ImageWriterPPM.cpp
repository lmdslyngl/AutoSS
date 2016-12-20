
#include "ImageWriterPPM.h"
#include <fstream>

// 画像を保存
void ImageWriterPPM::Write(
	const std::string &filename,
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
	ofs.write((const char*)data, length);
	
	ofs.close();
	
}

// 連続撮影開始
void ImageWriterPPM::BeginCapture() { }

// 連続撮影終了
void ImageWriterPPM::EndCapture() { }

