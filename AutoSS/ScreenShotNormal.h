#pragma once

#include <vector>
#include <chrono>
#include "ScreenShotBase.h"
#include "TimerExec.h"

class ScreenShot2 : public ScreenShotBase {
public:
	
	ScreenShot2(
		std::shared_ptr<CaptureBase> &pCap,
		std::shared_ptr<ImageWriterBase> &pWriter,
		const std::wstring &savePathFormat,
		int waitTimeMillisec,
		TRIMMING_MODE trimMode);
	
	// 連写開始
	virtual void Start();
	
	// 連写終了
	virtual void Stop();
	
	// 連写間隔設定
	void SetWaitTime(int waitMillisec) {
		pTimer->SetWaitTime(waitMillisec);
	}
	
	// 連写間隔取得
	int GetWaitTime() const {
		return pTimer->GetWaitTime();
	}
	
protected:
	
	// スクリーンショット撮影
	void TakeSSFunc(void *ptr);
	
protected:
	std::unique_ptr<TimerExec> pTimer;
	std::vector<unsigned char> vecImgBuffer;
	std::chrono::time_point<std::chrono::system_clock> CapStart;
};

