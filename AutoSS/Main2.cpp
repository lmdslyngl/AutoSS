
#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/snglinst.h>

#include <memory>
#include <functional>
#include <time.h>
#include <sstream>
#include <iomanip>
#include <chrono>

#include "UI.h"
#include "BitBltCapture.h"
#include "DesktopDuplCapture.h"
#include "ImageWriterBMP.h"
#include "ScreenShotNormal.h"
#include "ScreenShotBurst.h"
#include "CaptureRegion.h"
#include "Config.h"
#include "Logger.h"
#include "DPIUtil.h"

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
	std::unique_ptr<ScreenShotBase> CreateSS(
		const std::shared_ptr<Config> &pConf,
		bool burstMode);
	
	// 現在時刻を文字列で返す
	std::string GetDateString() const;
	
	void OnStart();
	void OnStop();
	void OnChangeConf(const std::shared_ptr<Config> &pConf);
	void OnCaptureFinished();
	
private:
	std::shared_ptr<Config> pConf;
	std::unique_ptr<ScreenShotBase> pSS;
	AutoSSFrame *pFrame;
	std::unique_ptr<wxSingleInstanceChecker> pSingleChecker;
	bool IsBurstModeFlag;
};



wxIMPLEMENT_APP(AutoSSApp);

bool AutoSSApp::OnInit() {
	if( !wxApp::OnInit() ) return false;
	
	// GlobalDpiUtilを初期化
	SetupGlobalDpiUtil();
	
	// Loggerを初期化
	GlbLog::SetupGlobalLogger(L"AutoSS_dbg.log");
	
	// 多重起動かどうかをチェック
	pSingleChecker = std::make_unique<wxSingleInstanceChecker>();
	if( pSingleChecker->IsAnotherRunning() ) {
		wxMessageBox(
			L"AutoSSは既に起動してます",
			L"AutoSS",
			wxOK | wxICON_ERROR);
		return false;
	}
	
	// 設定ファイル読み込み
	pConf = std::make_unique<Config>();
	try {
		pConf->Load(L"AutoSS.ini");
	} catch( std::exception &e ) {
		wxMessageBox(
			L"設定ファイルの読み込みに失敗しました\n" + std::string(e.what()),
			L"AutoSS",
			wxOK | wxICON_ERROR);
		return nullptr;
	}
	
	// スクリーンショット撮影クラス作成
	IsBurstModeFlag = false;
	pSS = CreateSS(pConf, IsBurstModeFlag);
	
	// メインウィンドウ
	pFrame = new AutoSSFrame(pConf);
	pFrame->SetOnStartFunc([this]() { this->OnStart(); });
	pFrame->SetOnStopFunc([this]() { this->OnStop(); });
	pFrame->SetOnChangeConfFunc(
		[this](const std::shared_ptr<Config> &pConf) {
			this->OnChangeConf(pConf);
		}
	);
	pFrame->SetOnModeChangeFunc(
		[this](bool mode) {
			pSS.reset();
			pSS = CreateSS(pConf, mode);
			IsBurstModeFlag = mode;
		}
	);
	
	// スクリーンショットの初期化に失敗した場合は，開始ボタンを無効にする
	if( !pSS ) pFrame->DisableCapture();
	
	pFrame->Show();
	
	return true;
	
}

int AutoSSApp::OnExit() {
	pConf->Save(L"AutoSS.ini");
	return wxApp::OnExit();
}

std::unique_ptr<ScreenShotBase> AutoSSApp::CreateSS(
	const std::shared_ptr<Config> &pConf,
	bool burstMode)
{
	
	// Captureクラス
	std::shared_ptr<CaptureBase> pCap;
	if( pConf->CaptureMethod == CAPTURE_BITBLT ) {
		// BitBlt
		try {
			HINSTANCE hInstance = GetModuleHandle(nullptr);
			auto pCapBitblt = std::make_shared<BitBltCapture>();
			pCapBitblt->Setup(hInstance, nullptr);
			pCap = pCapBitblt;
		} catch( std::exception & ) {
			wxMessageBox(
				L"BitBltCaptureの初期化に失敗しました",
				L"AutoSS",
				wxOK | wxICON_ERROR);
			return nullptr;
		}
		
	} else if( pConf->CaptureMethod == CAPTURE_DESKTOP_DUPL_API ) {
		// Desktop Duplication API
		try {
			auto pCapDD = std::make_shared<DesktopDuplCapture>();
			pCapDD->Setup();
			pCap = pCapDD;
		} catch( std::exception & ) {
			wxMessageBox(
				wxString(
					L"DesktopDuplCaptureの初期化に失敗しました\n"
					L"設定からキャプチャ方式をBitBltに変更してみてください\n"),
				L"AutoSS",
				wxOK | wxICON_ERROR);
			return nullptr;
		}
		
	} else {
		wxMessageBox(
			L"不正なキャプチャ方式です: " + pConf->CaptureMethod,
			L"AutoSS",
			wxOK | wxICON_ERROR);
		return nullptr;
	}
	
	// 画像書き出しクラス
	std::shared_ptr<ImageWriterBase> pImageWriter;
	if( pConf->ImageFormat == IMGFMT_BMP ) {
		pImageWriter = std::make_shared<ImageWriterBMP>();
	} else {
		wxMessageBox(
			L"不正な画像形式です: " + pConf->ImageFormat,
			L"AutoSS",
			wxOK | wxICON_ERROR);
		return nullptr;
	}
	
	// 撮影範囲
	std::shared_ptr<CaptureRegionBase> pCapRegion;
	if( pConf->RegionMode == CAPTURE_REGION_ACTIVE_WINDOW ) {
		pCapRegion = std::make_shared<CaptureRegionActiveWindow>(
			pConf->IncludeBorder);
	} else if( pConf->RegionMode == CAPTURE_REGION_SELECTED_REGION ) {
		pCapRegion = std::make_shared<CaptureRegionSelectedRegion>(
			pConf->RegionX, pConf->RegionY,
			pConf->RegionWidth, pConf->RegionHeight);
	} else if( pConf->RegionMode == CAPTURE_REGION_FULLSCREEN ) {
		pCapRegion = std::make_shared<CaptureRegionFullscreen>();
	}
	
	// スクリーンショットクラス
	std::unique_ptr<ScreenShotBase> pSS;
	if( burstMode ) {
		pSS = std::make_unique<ScreenShotBurst>(
			pCap, pImageWriter, L"",
			pCapRegion, 100);
	} else {
		pSS = std::make_unique<ScreenShot2>(
			pCap, pImageWriter, L"",
			pConf->WaitTime, pCapRegion,
			pConf->MaxCaptureCount);
	}
	pSS->SetOnFinishedFunc([this]() { this->OnCaptureFinished(); });
	
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
	std::wstring savepath = pConf->SavePath;
	if( pConf->SavePath.empty() ) {
		// 保存ディレクトリが空の場合は，ピクチャフォルダに保存
		savepath = wxStandardPaths::Get().GetUserDir(wxStandardPaths::Dir_Pictures);
	}
	
	std::wstringstream nameFormat;
	nameFormat << savepath << L"\\"
		<< L"ss_" << GetDateString() << L"_%04d." << pConf->GetFormatExt();
	pSS->SetSavePathFormat(nameFormat.str());
	
	pFrame->Start();
	pSS->Start();
	
}

void AutoSSApp::OnStop() {
	pFrame->DisableCapture();
	pSS->Stop();
}

void AutoSSApp::OnChangeConf(const std::shared_ptr<Config> &pConf) {
	*this->pConf = *pConf;
	pSS.reset();
	pSS = CreateSS(pConf, IsBurstModeFlag);
	if( pSS ) {
		pFrame->EnableCapture();
	} else {
		pFrame->DisableCapture();
	}
}

void AutoSSApp::OnCaptureFinished() {
	pFrame->Stop();
	pFrame->EnableCapture();
	
	std::wstringstream statusText;
	statusText << L"Stopped: " << pSS->GetTakenCount() << L" images taken "
		<< L"(fps: " << std::fixed << std::setprecision(2) << pSS->GetCapturedFPS() << L")";
	pFrame->SetStatusText(statusText.str());
	
}


