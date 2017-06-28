#pragma once

#include <string>

class ImageWriterBase {
public:
	
	// 画像を保存
	virtual void Write(
		const std::wstring &filename,
		int width, int height,
		const unsigned char *data,
		size_t length) = 0;
	
	// 連続撮影開始
	virtual void BeginCapture() = 0;
	
	// 連続撮影終了
	virtual void EndCapture() = 0;
	
};

