#pragma once

#include <vector>
#include <memory>
#include <string>
#include <thread>
#include <atomic>

class CompressedImageWriter {
public:
	
	CompressedImageWriter(int batchSize, int stride, int width, int height) {
		this->BatchSize = batchSize;
		this->Stride = stride;
		this->Width = width;
		this->Height = height;
		vecBuffer.resize(BatchSize * stride);
	}
	
	~CompressedImageWriter() = default;
	
	unsigned char *GetBufferPtr() {
		return vecBuffer.data();
	}
	size_t GetBufferSize() const {
		return vecBuffer.size();
	}
	
	void SetCount(int count) {
		this->Count = count;
	}
	
	void Write(const std::string &filename);
	
private:
	std::vector<unsigned char> vecBuffer;
	int BatchSize;
	int Stride;
	int Count;
	int Width, Height;
};


class LZ4BatchedImage {
public:
	
	LZ4BatchedImage() = default;
	~LZ4BatchedImage();
	
	void Setup(int width, int height, int batchSize);
	void Add(const unsigned char *data, size_t length);
	
	void CompressedWrite(const std::string &filename);
	
	bool IsFull() const {
		return BatchSize <= Count;
	}
	
	void GetSize(int *pOutWidth, int *pOutHeight) {
		if( pOutWidth ) *pOutWidth = Width;
		if( pOutHeight ) *pOutHeight = Height;
	}
	
protected:
	std::shared_ptr<CompressedImageWriter> pWriter;
	int BatchSize;
	int Stride;
	int Count;
	int Width, Height;
	
	std::thread WritingThread;
	std::atomic_bool ThreadRunningFlag;
	
};



