
#include "Setting.h"
#include <fstream>
#include "ScreenShot.h"

bool Setting::Load(const std::string &settingfile) {
	std::ifstream ifs(settingfile);
	if( !ifs ) return false;
	
	std::string line;
	while( std::getline(ifs, line) ) {
		// 行頭が#ならコメント
		if( line[0] == '#' ) continue;
		
		std::string::size_type splitpos = line.find('=');
		if( splitpos != std::string::npos ) {
			std::string key = line.substr(0, splitpos);
			std::string val = line.substr(splitpos + 1);
			mapKeyValues[key] = val;
		}
		
	}
	
	ifs.close();
	
	return true;
	
}

TRIMMING_MODE Setting::GetTrimmingMode() const {
	const std::string &mode = GetValueOrDefault("TrimmingMode", "window");
	if( mode == "window" ) {
		return TRIMMING_WINDOW_RECT;
	} else if( mode == "window_dwm" ) {
		return TRIMMING_WINDOW_RECT_DWM;
	} else if( mode == "client" ) {
		return TRIMMING_CLIENT_RECT;
	}
	return TRIMMING_WINDOW_RECT;
}


