#pragma once

#include <vector>
#include <tuple>
#include <thread>
#include "ScreenShotBase.h"

class ScreenShotBurst : public ScreenShotBase {
public:
	
	ScreenShotBurst(
		std::shared_ptr<CaptureBase> &pCap,
		std::shared_ptr<ImageWriterBase> &pWriter,
		const std::wstring &savePathFormat,
		TRIMMING_MODE trimMode,
		unsigned int numCaptureImages);
	
	virtual ~ScreenShotBurst();
	
	// 連写開始
	virtual void Start();
	
	// 連写終了
	virtual void Stop();
	
protected:
	
	// スクリーンショット撮影
	void TakeSSFunc();
	
protected:
	std::thread TakeThread;
	std::vector<unsigned char> vecImgBuffer;
	std::vector<std::tuple<int, int>> vecImgSize;
	unsigned int NumCaptureImages;
	bool StopFlag;
};

