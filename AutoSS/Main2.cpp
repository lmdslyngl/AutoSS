
#define WXUSINGDLL
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
#include "ImageWriterPPM.h"
#include "ScreenShot2.h"
#include "ScreenShotBurst.h"
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
	std::unique_ptr<ScreenShotBase> CreateSS(
		const std::shared_ptr<Config> &pConf,
		bool burstMode);
	
	// 現在時刻を文字列で返す
	std::string GetDateString() const;
	
	void OnStart();
	void OnStop();
	void OnChangeConf(const std::shared_ptr<Config> &pConf);
	
private:
	std::shared_ptr<Config> pConf;
	std::unique_ptr<ScreenShotBase> pSS;
	AutoSSFrame *pFrame;
	std::chrono::time_point<std::chrono::system_clock> StartTime;
	std::unique_ptr<wxSingleInstanceChecker> pSingleChecker;
	bool IsBurstModeFlag;
};



wxIMPLEMENT_APP(AutoSSApp);

bool AutoSSApp::OnInit() {
	if( !wxApp::OnInit() ) return false;
	
	// 多重起動かどうかをチェック
	pSingleChecker = std::make_unique<wxSingleInstanceChecker>();
	if( pSingleChecker->IsAnotherRunning() ) {
		wxMessageBox(
			"AutoSSは既に起動してます",
			"AutoSS",
			wxOK | wxICON_ERROR);
		return false;
	}
	
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
	pConf->Save("AutoSS.ini");
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
				"BitBltCaptureの初期化に失敗しました",
				"AutoSS",
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
					"DesktopDuplCaptureの初期化に失敗しました\n"
					"設定からキャプチャ方式をBitBltに変更してみてください\n"),
				"AutoSS",
				wxOK | wxICON_ERROR);
			return nullptr;
		}
		
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
		trimmode = TRIMMING_WINDOW_RECT;
	} else {
		trimmode = TRIMMING_CLIENT_RECT;;
	}
	
	// スクリーンショットクラス
	std::unique_ptr<ScreenShotBase> pSS;
	if( burstMode ) {
		pSS = std::make_unique<ScreenShotBurst>(
			pCap, pImageWriter, "",
			trimmode, 100);
	} else {
		pSS = std::make_unique<ScreenShot2>(
			pCap, pImageWriter, "",
			pConf->WaitTime, trimmode);
	}
	
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
	std::string savepath = pConf->SavePath;
	if( pConf->SavePath.empty() ) {
		// 保存ディレクトリが空の場合は，ピクチャフォルダに保存
		savepath = wxStandardPaths::Get().GetUserDir(wxStandardPaths::Dir_Pictures);
	}
	
	std::stringstream nameFormat;
	nameFormat << savepath << "\\"
		<< "ss_" << GetDateString() << "_%04d." << pConf->GetFormatExt();
	pSS->SetSavePathFormat(nameFormat.str());
	
	StartTime = std::chrono::system_clock::now();
	
	pSS->Start();
	
}

void AutoSSApp::OnStop() {
	pSS->Stop();
	
	auto endTime = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - StartTime);
	double fps = (double)pSS->GetTakenCount() / (duration.count() / 1000.0);
	
	std::stringstream statusText;
	statusText << "Stopped: " << pSS->GetTakenCount() << " images taken "
		<< "(fps: " << std::fixed << std::setprecision(2) << fps << ")";
	pFrame->SetStatusText(statusText.str());
	
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



