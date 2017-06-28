#pragma once

#include <string>
#include <vector>

// UTF-8なstd::stringをstd::wstringに変換
std::wstring ConvUtf8ToWStr(const std::string &utf8str);

// std::wstringをUTF-8なstd::stringに変換
std::string ConvWStrToUtf8(const std::wstring &wstr);

// UTF-8なファイルを読み込み
std::wstring ReadUtf8File(const std::wstring &file);

// UTF-8なファイルを書き込み
void WriteUtf8File(const std::wstring &file, const std::wstring &content);

// 文字列分割
std::vector<std::wstring> SplitStr(const std::wstring &str, wchar_t ch);

