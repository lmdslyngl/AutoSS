#pragma once

#include <vector>

class LZ4BatchedImage {
public:
	
	LZ4BatchedImage() = default;
	~LZ4BatchedImage() = default;
	
	void Setup(int width, int height, int batchSize);
	void Add(const unsigned char *data, int length);
	
	void CompressedWrite(const std::string &filename);
	
	bool IsFull() const {
		return BatchSize <= Count;
	}
	
	void GetSize(int *pOutWidth, int *pOutHeight) {
		if( pOutWidth ) *pOutWidth = Width;
		if( pOutHeight ) *pOutHeight = Height;
	}
	
protected:
	int BatchSize;
	int Count;
	int Stride;
	int Width, Height;
	std::vector<unsigned char> vecBuffer;
};



