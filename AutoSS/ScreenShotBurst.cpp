
#include "ScreenShotBurst.h"
#include <chrono>

ScreenShotBurst::ScreenShotBurst(
	std::shared_ptr<CaptureBase> &pCap,
	std::shared_ptr<ImageWriterBase> &pWriter,
	const std::string &savePathFormat,
	TRIMMING_MODE trimMode,
	unsigned int numCaptureImages)
	: ScreenShotBase()
{
	
	this->SetCapturer(pCap);
	this->SetWriter(pWriter);
	this->SetSavePathFormat(savePathFormat);
	this->SetTrimmingMode(trimMode);
	this->NumCaptureImages = numCaptureImages;
	
}

ScreenShotBurst::~ScreenShotBurst() {
	Stop();
}

// 連写開始
void ScreenShotBurst::Start() {
	Stop();
	TakenCount = 0;
	StopFlag = false;
	TakeThread = std::thread([this]() { this->TakeSSFunc(); });
}

// 連写終了
void ScreenShotBurst::Stop() {
	StopFlag = true;
	if( TakeThread.joinable() ) TakeThread.detach();
}

// スクリーンショット撮影
void ScreenShotBurst::TakeSSFunc() {
	unsigned char *bufcursor = nullptr;
	unsigned char *bufend = nullptr;
	int sswidth, ssheight;
	
	vecImgSize.reserve(NumCaptureImages);
	
	// 撮影
	auto start = std::chrono::system_clock::now();
	for( int i = 0; i < NumCaptureImages; i++ ) {
		if( StopFlag ) break;
		
		HWND hCaptureWindow = GetForegroundWindow();
		RECT windowRect = GetWindowRegion(hCaptureWindow);
		windowRect = ClampOutOfRegion(windowRect);
		
		unsigned int buflen = pCap->CalcNecessaryBufferLength(&windowRect);
		if( bufcursor == nullptr ) {
			// まだバッファが確保されていなかったとき
			vecImgBuffer.resize(buflen * NumCaptureImages);
			bufcursor = vecImgBuffer.data();
			bufend = bufcursor + vecImgBuffer.size();
		}
		if( bufend - bufcursor < buflen ) {
			// 今から書こうとしている画像の分のバッファが足りないとき
			// 今の画像サイズ*残り枚数分だけ追加で確保
			int64_t curidx = bufcursor - vecImgBuffer.data();
			int remainingImg = NumCaptureImages - i;
			vecImgBuffer.resize(vecImgBuffer.size() + buflen * remainingImg);
			bufcursor = vecImgBuffer.data() + curidx;
			bufend = vecImgBuffer.data() + vecImgBuffer.size();
		}
		
		pCap->CaptureRegion(
			&windowRect,
			bufcursor, bufend - bufcursor,
			&sswidth, &ssheight);
		bufcursor += buflen;
		
		vecImgSize.push_back(std::tuple<int, int>(sswidth, ssheight));
		
		TakenCount++;
		
	}
	auto end = std::chrono::system_clock::now();
	
	// 書き出し
	bufcursor = vecImgBuffer.data();
	for( unsigned int i = 0; i < vecImgSize.size(); i++ ) {
		char savename[128];
		sprintf_s(savename, SavePathFormat.c_str(), i);
		
		const auto &size = vecImgSize[i];
		sswidth = std::get<0>(size);
		ssheight = std::get<1>(size);
		
		pWriter->Write(savename, sswidth, ssheight,
			bufcursor, sswidth * ssheight * 3);
		
		bufcursor += sswidth * ssheight * 3;
		
	}
	
	vecImgSize.clear();
	
	// FPS計算
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	CapturedFPS = GetTakenCount() / (elapsed.count() / 1000.0);
	
	if( OnFinishedFunc ) OnFinishedFunc();
	
}

