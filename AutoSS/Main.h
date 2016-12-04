#pragma once

#include <Windows.h>
#include <vector>
#include <string>

// スクリーンショット撮影&保存
void TakeSS();

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

// 現在の日時を取得
std::string GetNowDate();

// コマンドプロンプトのウィンドウハンドルを取得
HWND GetConsoleHWND();

// 設定を出力
void PrintSetting();


