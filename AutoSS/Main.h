#pragma once

#include <Windows.h>
#include <vector>
#include <string>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

// 現在の日時を取得
std::string GetNowDate();

// コマンドプロンプトのウィンドウハンドルを取得
HWND GetConsoleHWND();

// 設定を出力
void PrintSetting();


