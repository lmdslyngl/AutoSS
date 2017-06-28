
#include "Config.h"
#include <fstream>
#include <sstream>
#include "Util.h"

/*
 * 設定クラス
*/

void Config::Load(const std::wstring &conffile) {
	auto mapKeyValues = ParseConfFile(conffile);
	
	SavePath = mapKeyValues[L"SavePath"];
	WaitTime = std::stoi(mapKeyValues[L"WaitTime"]);
	CaptureMethod = (CAPTURE_METHOD)std::stoi(mapKeyValues[L"CaptureMethod"]);
	IncludeBorder = std::stoi(mapKeyValues[L"IncludeBorder"]) ? true : false;
	HotkeyMod = std::stoi(mapKeyValues[L"HotkeyMod"]);
	HotkeyCode = std::stoi(mapKeyValues[L"HotkeyCode"]);
	HotkeyCodeRaw = std::stoi(mapKeyValues[L"HotkeyCodeRaw"]);
	ImageFormat = (IMAGE_FORMAT)std::stoi(mapKeyValues[L"ImageFormat"]);
	
}

void Config::Save(const std::wstring &conffile) {
	std::wstringstream ss;
	ss << L"SavePath=" << SavePath << L"\n";
	ss << L"WaitTime=" << WaitTime << L"\n";
	ss << L"CaptureMethod=" << std::to_wstring(CaptureMethod) << L"\n";
	ss << L"IncludeBorder=" << (IncludeBorder ? 1 : 0) << L"\n";
	ss << L"HotkeyMod=" << std::to_wstring(HotkeyMod) << L"\n";
	ss << L"HotkeyCode=" << std::to_wstring(HotkeyCode) << L"\n";
	ss << L"HotkeyCodeRaw=" << std::to_wstring(HotkeyCodeRaw) << L"\n";
	ss << L"ImageFormat=" << std::to_wstring(ImageFormat) << L"\n";
	WriteUtf8File(conffile, ss.str());
}


// 設定ファイルをキーと値に切り分ける
std::map<std::wstring, std::wstring> Config::ParseConfFile(const std::wstring &conffile) {
	std::wstring confFileContent = ReadUtf8File(conffile);
	std::map<std::wstring, std::wstring> mapKeyValues;
	
	std::vector<std::wstring> vecLines = SplitStr(confFileContent, L'\n');
	for( const std::wstring &line : vecLines ) {
		// 先頭が#ならコメント
		if( line[0] == L'#' ) continue;
		
		std::wstring::size_type splitpos = line.find(L'=');
		if( splitpos != std::wstring::npos ) {
			std::wstring key = line.substr(0, splitpos);
			std::wstring val = line.substr(splitpos + 1);
			mapKeyValues[key] = val;
		}
		
	}
	
	return mapKeyValues;
	
}

