
#include <time.h>
#include <ShObjIdl.h>
#include "Main.h"
#include "ScreenShot2.h"
#include "Setting.h"
#include "BitBltCapture.h"
#include "DesktopDuplCapture.h"
#include "ImageWriterPPM.h"
#include <iostream>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d11.lib")

const int HOTKEY_TOGGLE = 0;
const int HOTKEY_EXIT = 1;
const int TIMER_SS = 0;

bool ScreenShotting = false;
int ctr = 0;
std::string savePrefix;
std::unique_ptr<ScreenShot2> pSS;
std::shared_ptr<ImageWriterBase> pImageWriter;
std::unique_ptr<Setting> pSetting;

ITaskbarList3 *pTaskbar = nullptr;
HWND hConWindow = nullptr;

int main(int argc, char *argv[]) {
	
	CoInitialize(nullptr);
	
	// 設定
	pSetting = std::make_unique<Setting>();
	if( !pSetting->Load("AutoSS.ini") ) return 1;
	PrintSetting();
	
	// ダミーウィンドウ
	auto pWindow = std::make_shared<DummyWindow>();
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	if( !pWindow->Setup(hInstance, WndProc) ) return 2;
	
	// Captureクラス
	std::shared_ptr<CaptureBase> pCap;
	if( pSetting->GetCaptureMethod() == "bitblt" ) {
		auto pCapBitblt = std::make_shared<BitBltCapture>();
		pCapBitblt->Setup(hInstance, pWindow);
		pCap = pCapBitblt;
	} else if( pSetting->GetCaptureMethod() == "desktop_dupl_api" ) {
		auto pCapDD = std::make_shared<DesktopDuplCapture>();
		pCapDD->Setup();
		pCap = pCapDD;
	}
	
	// スクリーンショットクラス
	pSS = std::make_unique<ScreenShot2>(pCap);
	pSS->SetTrimmingMode(pSetting->GetTrimmingMode());
	
	// 画像書き出しクラス
	if( pSetting->GetSaveFormat() == "ppm" ) {
		pImageWriter = std::make_shared<ImageWriterPPM>();
	}
	
	// スクリーンショット開始/終了ホットキー
	RegisterHotKey(
		pWindow->GetWindowHandle(), HOTKEY_TOGGLE,
		MOD_CONTROL | MOD_SHIFT, VK_F1);
	
	// アプリケーション終了ホットキー
	RegisterHotKey(
		pWindow->GetWindowHandle(), HOTKEY_EXIT,
		MOD_CONTROL | MOD_SHIFT, VK_F2);
	
	// タスクバーリスト
	HRESULT hr = CoCreateInstance(
		CLSID_TaskbarList,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_ITaskbarList3,
		(void**)&pTaskbar);
	if( FAILED(hr) ) return 4;
	
	hConWindow = GetConsoleHWND();
	SetConsoleTitle("AutoSS");
	
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	while( GetMessage(&msg, nullptr, 0, 0) ) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	UnregisterHotKey(pWindow->GetWindowHandle(), HOTKEY_TOGGLE);
	UnregisterHotKey(pWindow->GetWindowHandle(), HOTKEY_EXIT);
	
	if( ScreenShotting ) {
		KillTimer(pWindow->GetWindowHandle(), TIMER_SS);
	}
	
	pTaskbar->Release();
	pTaskbar = nullptr;
	
	CoUninitialize();
	
	return 0;
	
}

// スクリーンショット撮影&保存
void TakeSS() {
	HWND hCaptureWindow = GetForegroundWindow();
	if( hCaptureWindow ) {
		pSS->TakeScreenShot(hCaptureWindow);
		
		char name[128];
		sprintf_s(name, "%sss_%s_%04d.%s",
			pSetting->GetSavePath().c_str(), savePrefix.c_str(), ctr,
			pSetting->GetSaveFormat().c_str());
		
		pSS->WriteImage(name, pImageWriter);
		
		ctr++;
		
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch( msg ) {
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
		
	case WM_HOTKEY:
		if( wp == HOTKEY_TOGGLE ) {
			// スクリーンショット開始/終了
			ScreenShotting = !ScreenShotting;
			
			if( ScreenShotting ) {
				std::cout << "Capture started" << std::endl;
				
				pImageWriter->BeginCapture();
				
				savePrefix = GetNowDate();
				ctr = 0;
				
				SetTimer(hwnd, TIMER_SS, pSetting->GetWait(), nullptr);
				pTaskbar->SetProgressState(hConWindow, TBPF_NORMAL);
				pTaskbar->SetProgressValue(hConWindow, 1, 1);
				
			} else {
				pImageWriter->EndCapture();
				
				KillTimer(hwnd, TIMER_SS);
				pTaskbar->SetProgressState(hConWindow, TBPF_NOPROGRESS);
				
				std::cout << "\nCapture stopped: "
					<< ctr << " files captured" << std::endl;
				
			}
			
		} else if( wp == HOTKEY_EXIT ) {
			// アプリケーション終了
			PostQuitMessage(0);
			
		}
		break;
		
	case WM_TIMER:
		// スクリーンショット撮影
		std::cout << "\rCapturing " << ctr << std::flush;
		TakeSS();
		break;
		
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

// 現在の日時を取得
std::string GetNowDate() {
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

// コマンドプロンプトのウィンドウハンドルを取得
HWND GetConsoleHWND() {
	char title[1024];
	GetConsoleTitle(title, 1024);
	return FindWindow(nullptr, title);
}

// 設定を出力
void PrintSetting() {
	std::cout << "Settings\n"
		<< "  SavePath: " << pSetting->GetSavePath() << "\n"
		<< "  Wait: " << pSetting->GetWait() << "\n"
		<< "  SaveFormat: " << pSetting->GetSaveFormat() << "\n"
		<< "  TrimmingMode: " << pSetting->GetTrimmingMode() << "\n"
		<< "  CaptureMethod: " << pSetting->GetCaptureMethod() << std::endl;
}


