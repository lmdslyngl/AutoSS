#pragma once

#include "ImageWriterBase.h"
#include <vector>

/*
 * BMP形式で保存
*/
class ImageWriterBMP : public ImageWriterBase {
public:
	
	ImageWriterBMP() = default;
	virtual ~ImageWriterBMP() = default;
	
	// 画像を保存
	virtual void Write(
		const std::wstring &filename,
		int width, int height,
		const unsigned char *data,
		size_t length);
	
	// 連続撮影開始
	virtual void BeginCapture();
	
	// 連続撮影終了
	virtual void EndCapture();
	
private:
	std::vector<char> vecStreamBuf;
};

