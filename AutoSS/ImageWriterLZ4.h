#pragma once

#include <memory>
#include <string>
#include "ImageWriterBase.h"
#include "LZ4BatchedImage.h"

class ImageWriterLZ4 : public ImageWriterBase {
public:
	
	ImageWriterLZ4() = default;
	virtual ~ImageWriterLZ4() = default;
	
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
	
private:
	
	// 画像を書き出し
	void Flush();
	
private:
	std::unique_ptr<LZ4BatchedImage> pLZ4Img;
	std::string Filename;
};
