﻿
#include "DummyWindow.h"

/**
 * ダミーウィンドウクラス
*/
DummyWindow::~DummyWindow() {
	DestroyWindow(hWindowDummy);
	UnregisterClass(L"DUMMYWINDOW", hInstance);
}

bool DummyWindow::Setup(HINSTANCE hInstance, WNDPROC pFuncWndProc) {
	this->hInstance = hInstance;
	
	if( pFuncWndProc ) {
		if( !CreateDummyWindow(pFuncWndProc) ) return false;
	} else {
		if( !CreateDummyWindow(DefaultWndProc) ) return false;
	}
	
	return true;
	
}

// ダミーウィンドウを作成
bool DummyWindow::CreateDummyWindow(WNDPROC pFuncWndProc) {
	WNDCLASSEX winc;
	winc.cbSize			= sizeof(WNDCLASSEX);
	winc.style			= CS_HREDRAW | CS_VREDRAW;
	winc.lpfnWndProc	= pFuncWndProc;
	winc.cbClsExtra		= 0;
	winc.cbWndExtra		= 0;
	winc.hInstance		= hInstance;
	winc.hIcon			= nullptr;
	winc.hIconSm		= 0;
	winc.hCursor		= LoadCursor(hInstance, IDC_ARROW);
	winc.hbrBackground	= GetSysColorBrush(COLOR_BTNFACE);
	winc.lpszMenuName	= NULL;
	winc.lpszClassName	= L"DUMMYWINDOW";
	
	if( GetClassInfoEx(hInstance, L"DUMMYWINDOW", &winc) == 0 ) {
		// DUMMYWINDOWクラスが登録されていなかったときのみ登録する
		if( !RegisterClassEx(&winc) ) return false;
	}
	
	hWindowDummy = CreateWindow(
		L"DUMMYWINDOW", L"DummyWindow", 0,
		0, 0, 100, 100,
		nullptr, nullptr, hInstance, nullptr
	);
	
	if( hWindowDummy == nullptr ) return false;
	
	return true;
	
}

// デフォルトのウィンドウプロシージャ
LRESULT CALLBACK DummyWindow::DefaultWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	return DefWindowProc(hwnd, msg, wp, lp);
}



