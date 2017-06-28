
#include "Util.h"
#include <Windows.h>
#include <vector>
#include <fstream>

// UTF-8なstd::stringをstd::wstringに変換
std::wstring ConvUtf8ToWStr(const std::string &utf8str) {
	std::vector<wchar_t> buffer;
	buffer.resize(MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, nullptr, 0));
	MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, buffer.data(), buffer.size());
	return buffer.data();
}

// std::wstringをUTF-8なstd::stringに変換
std::string ConvWStrToUtf8(const std::wstring &wstr) {
	std::vector<char> buffer;
	buffer.resize(WideCharToMultiByte(
		CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr));
	WideCharToMultiByte(
		CP_UTF8, 0, wstr.c_str(), -1, buffer.data(), buffer.size(), nullptr, nullptr);
	return buffer.data();
}

// UTF-8なファイルを読み込み
std::wstring ReadUtf8File(const std::wstring &file) {
	std::ifstream ifs(file, std::ios::in);
	if( !ifs ) {
		throw std::exception("Failed to open file");
	}
	std::string content;
	content.resize(ifs.seekg(0, std::ios::end).tellg());
	ifs.seekg(0, std::ios::beg).read(&content[0], content.size());
	ifs.close();
	return ConvUtf8ToWStr(content);
}

// UTF-8なファイルを書き込み
void WriteUtf8File(const std::wstring &file, const std::wstring &content) {
	std::string utf8content = ConvWStrToUtf8(content);
	std::ofstream ofs(file, std::ios::out | std::ios::trunc);
	if( !ofs ) {
		throw std::exception("Failed to open file");
	}
	ofs.write(utf8content.c_str(), utf8content.size());
	ofs.close();
}

// 文字列分割
std::vector<std::wstring> SplitStr(const std::wstring &str, wchar_t ch) {
	std::vector<std::wstring> vecSplitedStr;
	std::wstring splited;
	std::wstring::size_type pos = 0;
	std::wstring::size_type prevPos = 0;
	while( true ) {
		pos = str.find(ch, prevPos);
		if( pos == std::wstring::npos ) break;
		splited = str.substr(prevPos, pos - prevPos);
		vecSplitedStr.push_back(splited);
		prevPos = pos + 1;
	}
	vecSplitedStr.push_back(str.substr(prevPos));
	return vecSplitedStr;
}

