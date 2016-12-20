
#include "ImageWriterLZ4.h"
#include <fstream>

// 画像を保存
void ImageWriterLZ4::Write(
	const std::string &filename,
	int width, int height,
	const unsigned char *data,
	size_t length)
{
	
	if( this->Filename.empty() ) {
		this->Filename = filename;
	}
	
	if( !pLZ4Img ) {
		// LZ4BatchedImageがまだ無いときは作成
		pLZ4Img = std::make_unique<LZ4BatchedImage>();
		pLZ4Img->Setup(width, height, 20);
	}
	
	int prevWidth, prevHeight;
	pLZ4Img->GetSize(&prevWidth, &prevHeight);
	
	if( prevWidth != width || prevHeight != height ) {
		// 前のフレームと異なるサイズだったときは，強制的に書き出しと初期化
		Flush();
		pLZ4Img->Setup(width, height, 20);
	}
	
	pLZ4Img->Add(data, length);
	
	if( pLZ4Img->IsFull() ) {
		// 画像が溜まったら書き出ししてリセット
		Flush();
		pLZ4Img->Setup(width, height, 20);
	}
	
}

// 連続撮影開始
void ImageWriterLZ4::BeginCapture() { }

// 連続撮影終了
void ImageWriterLZ4::EndCapture() {
	// 連写終了後は強制的に画像を書き出し
	Flush();
}

// 画像を書き出し
void ImageWriterLZ4::Flush() {
	pLZ4Img->CompressedWrite(this->Filename);
	this->Filename.clear();
}

