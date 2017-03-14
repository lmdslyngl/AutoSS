
#define WXUSINGDLL
#include <wx/wx.h>

#include <memory>
#include <functional>
#include <time.h>

#include "UI.h"
#include "BitBltCapture.h"
#include "DesktopDuplCapture.h"
#include "ImageWriterBMP.h"
#include "ImageWriterPPM.h"
#include "ScreenShot2.h"
#include "Config.h"

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")


/*
 * AutoSSアプリケーション
*/
class AutoSSApp : public wxApp {
public:
	virtual bool OnInit();
	virtual int OnExit();
	
private:
	
	// ScreenShotクラスを作成
	std::unique_ptr<ScreenShot2> CreateSS(const std::shared_ptr<Config> &pConf);
	
	// 現在時刻を文字列で返す
	std::string GetDateString() const;
	
	void OnStart();
	void OnStop();
	void OnChangeConf(const std::shared_ptr<Config> &pConf);
	
private:
	std::shared_ptr<Config> pConf;
	std::unique_ptr<ScreenShot2> pSS;
	AutoSSFrame *pFrame;
};



wxIMPLEMENT_APP(AutoSSApp);

bool AutoSSApp::OnInit() {
	if( !wxApp::OnInit() ) return false;
	
	// 設定ファイル読み込み
	pConf = std::make_unique<Config>();
	try {
		pConf->Load("AutoSS.ini");
	} catch( std::exception &e ) {
		wxMessageBox(
			"設定ファイルの読み込みに失敗しました\n" + std::string(e.what()),
			"AutoSS",
			wxOK | wxICON_ERROR);
		return nullptr;
	}
	
	// スクリーンショット撮影クラス作成
	pSS = CreateSS(pConf);
	if( !pSS ) return false;
	
	// メインウィンドウ
	pFrame = new AutoSSFrame(pConf);
	pFrame->SetOnStartFunc([this]() { this->OnStart(); });
	pFrame->SetOnStopFunc([this]() { this->OnStop(); });
	pFrame->SetOnChangeConfFunc(
		[this](const std::shared_ptr<Config> &pConf) {
			this->OnChangeConf(pConf);
		}
	);
	
	pFrame->Show();
	
	return true;
	
}

int AutoSSApp::OnExit() {
	pConf->Save("AutoSS.ini");
	return wxApp::OnExit();
}

std::unique_ptr<ScreenShot2> AutoSSApp::CreateSS(const std::shared_ptr<Config> &pConf) {
	// Captureクラス
	std::shared_ptr<CaptureBase> pCap;
	if( pConf->CaptureMethod == CAPTURE_BITBLT ) {
		// BitBlt
		HINSTANCE hInstance = GetModuleHandle(nullptr);
		auto pCapBitblt = std::make_shared<BitBltCapture>();
		pCapBitblt->Setup(hInstance, nullptr);
		pCap = pCapBitblt;
		
	} else if( pConf->CaptureMethod == CAPTURE_DESKTOP_DUPL_API ) {
		// Desktop Duplication API
		auto pCapDD = std::make_shared<DesktopDuplCapture>();
		pCapDD->Setup();
		pCap = pCapDD;
		
	} else {
		wxMessageBox(
			"不正なキャプチャ方式です: " + pConf->CaptureMethod,
			"AutoSS",
			wxOK | wxICON_ERROR);
		return nullptr;
	}
	
	// 画像書き出しクラス
	std::shared_ptr<ImageWriterBase> pImageWriter;
	if( pConf->ImageFormat == IMGFMT_PPM ) {
		pImageWriter = std::make_shared<ImageWriterPPM>();
	} else if( pConf->ImageFormat = IMGFMT_BMP ) {
		pImageWriter = std::make_shared<ImageWriterBMP>();
	} else {
		wxMessageBox(
			"不正な画像形式です: " + pConf->ImageFormat,
			"AutoSS",
			wxOK | wxICON_ERROR);
		return nullptr;
	}
	
	// ウィンドウのトリミング方法
	TRIMMING_MODE trimmode;
	if( pConf->IncludeBorder ) {
		trimmode = TRIMMING_WINDOW_RECT_DWM;
	} else {
		trimmode = TRIMMING_CLIENT_RECT;;
	}
	
	// スクリーンショットクラス
	auto pSS = std::make_unique<ScreenShot2>(
		pCap, pImageWriter, "",
		pConf->WaitTime, trimmode);
	
	return pSS;
	
}

// 現在時刻を文字列で返す
std::string AutoSSApp::GetDateString() const {
	time_t now = time(nullptr);
	tm tmnow;
	localtime_s(&tmnow, &now);
	
	char timestr[16];
	sprintf_s(timestr, "%02d%02d%02d%02d%02d%02d",
		tmnow.tm_year % 100, tmnow.tm_mon + 1, tmnow.tm_mday,
		tmnow.tm_hour, tmnow.tm_min, tmnow.tm_sec
	);
	
	return timestr;
	
}

void AutoSSApp::OnStart() {
	// スクリーンショットのファイル名フォーマット設定
	std::string nameFormat = pConf->SavePath + "\\"
		+ "ss_" + GetDateString() + "_%04d." + pConf->GetFormatExt();
	pSS->SetSavePathFormat(nameFormat);
	
	pSS->Start();
	
}

void AutoSSApp::OnStop() {
	pSS->Stop();
}

void AutoSSApp::OnChangeConf(const std::shared_ptr<Config> &pConf) {
	this->pConf = pConf;
	pSS.reset();
	pSS = CreateSS(pConf);
}



