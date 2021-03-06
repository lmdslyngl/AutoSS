﻿
#include "ScreenShotNormal.h"
#include <sstream>
#include <fstream>
#include <dwmapi.h>

ScreenShot2::ScreenShot2(
	std::shared_ptr<CaptureBase> &pCap,
	std::shared_ptr<ImageWriterBase> &pWriter,
	const std::wstring &savePathFormat,
	int waitTimeMillisec,
	const std::shared_ptr<CaptureRegionBase> &pRegion,
	int maxCaptureCount)
	: ScreenShotBase()
{
	
	this->SetCapturer(pCap);
	this->SetWriter(pWriter);
	this->SetSavePathFormat(savePathFormat);
	this->SetCaptureRegion(pRegion);
	this->MaxCaptureCount = maxCaptureCount;
	
	pTimer = std::make_unique<TimerExec>(
		[this](void *ptr) { this->TakeSSFunc(ptr); },
		waitTimeMillisec);
	
}

ScreenShot2::~ScreenShot2() {
	pTimer->Stop();
}

// 連写開始
void ScreenShot2::Start() {
	pTimer->Stop();
	TakenCount = 0;
	CapStart = std::chrono::system_clock::now();
	pTimer->Start();
}

// 連写終了
void ScreenShot2::Stop() {
	pTimer->StopWithoutJoin();
	
	// FPS計算
	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - CapStart);
	CapturedFPS = GetTakenCount() / (elapsed.count() / 1000.0);
	
	if( OnFinishedFunc ) OnFinishedFunc();
	
}

// スクリーンショット撮影
void ScreenShot2::TakeSSFunc(void *ptr) {
	RECT windowRect;
	pCapRegion->GetCaptureRegionRect(&windowRect);
	
	unsigned int buflen = pCap->CalcNecessaryBufferLength(&windowRect);
	vecImgBuffer.resize(buflen);
	
	int sswidth, ssheight;
	pCap->CaptureRegion(
		&windowRect,
		vecImgBuffer.data(), vecImgBuffer.size(),
		&sswidth, &ssheight);
	
	wchar_t savename[128];
	swprintf_s(savename, SavePathFormat.c_str(), TakenCount);
	
	pWriter->Write(savename, sswidth, ssheight,
		vecImgBuffer.data(), vecImgBuffer.size());
	
	if( 0 < MaxCaptureCount && MaxCaptureCount - 1 <= TakenCount ) {
		// 最大撮影枚数に到達したときは終了
		Stop();
		
	} else {
		// 最大撮影枚数が設定されていない，または到達していない場合はそのまま続行
		TakenCount++;
	}
	
}


