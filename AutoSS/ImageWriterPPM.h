#pragma once

#include "ImageWriterBase.h"

/*
 * PPM形式で保存
*/
class ImageWriterPPM : public ImageWriterBase {
public:
	
	ImageWriterPPM() = default;
	virtual ~ImageWriterPPM() = default;
	
	// 画像を保存
	virtual void Write(
		const std::string &filename,
		int width, int height,
		const unsigned char *data,
		size_t length);
	
	// 連続撮影開始
	virtual void BeginCapture();
	
	// 連続撮影終了
	virtual void EndCapture();
	
};

