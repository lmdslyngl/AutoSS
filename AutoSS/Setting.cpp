
#include "Setting.h"
#include <fstream>
#include "ScreenShotNormal.h"

bool Setting::Load(const std::wstring &settingfile) {
	std::wifstream ifs(settingfile);
	if( !ifs ) return false;
	
	std::wstring line;
	while( std::getline(ifs, line) ) {
		// 行頭が#ならコメント
		if( line[0] == L'#' ) continue;
		
		std::wstring::size_type splitpos = line.find(L'=');
		if( splitpos != std::wstring::npos ) {
			std::wstring key = line.substr(0, splitpos);
			std::wstring val = line.substr(splitpos + 1);
			mapKeyValues[key] = val;
		}
		
	}
	
	ifs.close();
	
	return true;
	
}

TRIMMING_MODE Setting::GetTrimmingMode() const {
	const std::wstring &mode = GetValueOrDefault(L"TrimmingMode", L"window");
	if( mode == L"window" ) {
		return TRIMMING_WINDOW_RECT;
	} else if( mode == L"window_dwm" ) {
		return TRIMMING_WINDOW_RECT_DWM;
	} else if( mode == L"client" ) {
		return TRIMMING_CLIENT_RECT;
	}
	return TRIMMING_WINDOW_RECT;
}


