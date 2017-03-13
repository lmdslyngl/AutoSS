
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
#include "Setting.h"

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")


/*
 * AutoSSアプリケーション
*/
class AutoSSApp : public wxApp {
public:
	virtual bool OnInit();
	
private:
	
	// ScreenShotクラスを作成
	std::unique_ptr<ScreenShot2> CreateSS();
	
	// 現在時刻を文字列で返す
	std::string GetDateString() const;
	
	void OnStart();
	void OnStop();
	void OnChangeConf();
	
private:
	std::unique_ptr<Setting> pSetting;
	std::unique_ptr<ScreenShot2> pSS;
};



wxIMPLEMENT_APP(AutoSSApp);

bool AutoSSApp::OnInit() {
	if( !wxApp::OnInit() ) return false;
	
	pSS = CreateSS();
	if( !pSS ) return false;
	
	AutoSSFrame *pFrame = new AutoSSFrame();
	pFrame->SetOnStartFunc([this]() { this->OnStart(); });
	pFrame->SetOnStopFunc([this]() { this->OnStop(); });
	pFrame->SetOnChangeConfFunc([this]() { this->OnChangeConf(); });
	
	pFrame->Show();
	
	return true;
	
}

std::unique_ptr<ScreenShot2> AutoSSApp::CreateSS() {
	// 設定ファイル読み込み
	pSetting = std::make_unique<Setting>();
	if( !pSetting->Load("AutoSS.ini") ) {
		wxMessageBox(
			"設定ファイルの読み込みに失敗しました",
			"AutoSS",
			wxOK | wxICON_ERROR);
		return nullptr;
	}
	
	// Captureクラス
	std::shared_ptr<CaptureBase> pCap;
	if( pSetting->GetCaptureMethod() == "bitblt" ) {
		// BitBlt
		HINSTANCE hInstance = GetModuleHandle(nullptr);
		auto pCapBitblt = std::make_shared<BitBltCapture>();
		pCapBitblt->Setup(hInstance, nullptr);
		pCap = pCapBitblt;
		
	} else if( pSetting->GetCaptureMethod() == "desktop_dupl_api" ) {
		// Desktop Duplication API
		auto pCapDD = std::make_shared<DesktopDuplCapture>();
		pCapDD->Setup();
		pCap = pCapDD;
		
	} else {
		wxMessageBox(
			"不正なキャプチャ方式です: " + pSetting->GetCaptureMethod(),
			"AutoSS",
			wxOK | wxICON_ERROR);
		return nullptr;
	}
	
	// 画像書き出しクラス
	std::shared_ptr<ImageWriterBase> pImageWriter;
	if( pSetting->GetSaveFormat() == "ppm" ) {
		pImageWriter = std::make_shared<ImageWriterPPM>();
	} else if( pSetting->GetSaveFormat() == "bmp" ) {
		pImageWriter = std::make_shared<ImageWriterBMP>();
	} else {
		wxMessageBox(
			"不正な画像形式です: " + pSetting->GetSaveFormat(),
			"AutoSS",
			wxOK | wxICON_ERROR);
		return nullptr;
	}
	
	// スクリーンショットクラス
	auto pSS = std::make_unique<ScreenShot2>(
		pCap, pImageWriter, "",
		pSetting->GetWait(),
		pSetting->GetTrimmingMode());
	
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
	std::string nameFormat = pSetting->GetSavePath()
		+ "ss_" + GetDateString() + "_%04d." + pSetting->GetSaveFormat();
	pSS->SetSavePathFormat(nameFormat);
	
	pSS->Start();
	
}

void AutoSSApp::OnStop() {
	pSS->Stop();
}

void AutoSSApp::OnChangeConf() {
	
}



