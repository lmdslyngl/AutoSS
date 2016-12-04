#pragma once

#include <Windows.h>

/**
 * ダミーウィンドウクラス
*/
class DummyWindow {
public:
	
	DummyWindow() = default;
	~DummyWindow();
	
	bool Setup(HINSTANCE hInstance, WNDPROC pFuncWndProc);
	
	HWND GetWindowHandle() const {
		return hWindowDummy;
	}
	
protected:
	
	// ダミーウィンドウを作成
	bool CreateDummyWindow(WNDPROC pFuncWndProc);
	
	// デフォルトのウィンドウプロシージャ
	static LRESULT CALLBACK DefaultWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
	
protected:
	HINSTANCE hInstance;
	HWND hWindowDummy;
};




