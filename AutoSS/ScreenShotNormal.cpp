
#include "ScreenShotNormal.h"
#include <sstream>
#include <fstream>
#include <dwmapi.h>

ScreenShot2::ScreenShot2(
	std::shared_ptr<CaptureBase> &pCap,
	std::shared_ptr<ImageWriterBase> &pWriter,
	const std::string &savePathFormat,
	int waitTimeMillisec,
	TRIMMING_MODE trimMode)
	: ScreenShotBase()
{
	
	this->SetCapturer(pCap);
	this->SetWriter(pWriter);
	this->SetSavePathFormat(savePathFormat);
	this->SetTrimmingMode(trimMode);
	
	pTimer = std::make_unique<TimerExec>(
		[this](void *ptr) { this->TakeSSFunc(ptr); },
		waitTimeMillisec);
	
}

// 連写開始
void ScreenShot2::Start() {
	TakenCount = 0;
	CapStart = std::chrono::system_clock::now();
	pTimer->Start();
}

// 連写終了
void ScreenShot2::Stop() {
	pTimer->Stop();
	
	// FPS計算
	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - CapStart);
	CapturedFPS = GetTakenCount() / (elapsed.count() / 1000.0);
	
	if( OnFinishedFunc ) OnFinishedFunc();
	
}

// スクリーンショット撮影
void ScreenShot2::TakeSSFunc(void *ptr) {
	HWND hCaptureWindow = GetForegroundWindow();
	RECT windowRect = GetWindowRegion(hCaptureWindow);
	windowRect = ClampOutOfRegion(windowRect);
	
	unsigned int buflen = pCap->CalcNecessaryBufferLength(&windowRect);
	vecImgBuffer.resize(buflen);
	
	int sswidth, ssheight;
	pCap->CaptureRegion(
		&windowRect,
		vecImgBuffer.data(), vecImgBuffer.size(),
		&sswidth, &ssheight);
	
	char savename[128];
	sprintf_s(savename, SavePathFormat.c_str(), TakenCount);
	
	pWriter->Write(savename, sswidth, ssheight,
		vecImgBuffer.data(), vecImgBuffer.size());
	
	TakenCount++;
	
}


