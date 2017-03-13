
#include "Config.h"
#include <fstream>

/*
 * 設定クラス
*/

void Config::Load(const std::string &conffile) {
	auto mapKeyValues = ParseConfFile(conffile);
	
	SavePath = mapKeyValues["SavePath"];
	WaitTime = std::stoi(mapKeyValues["WaitTime"]);
	CaptureMethod = (CAPTURE_METHOD)std::stoi(mapKeyValues["CaptureMethod"]);
	IncludeBorder = std::stoi(mapKeyValues["IncludeBorder"]) ? true : false;
	HotkeyMod = std::stoi(mapKeyValues["HotkeyMod"]);
	HotkeyCode = std::stoi(mapKeyValues["HotkeyCode"]);
	ImageFormat = (IMAGE_FORMAT)std::stoi(mapKeyValues["ImageFormat"]);
	
}

void Config::Save(const std::string &conffile) {
	std::ofstream ofs(conffile);
	if( !ofs ) {
		throw std::exception("Failed to open conffile");
	}
	
	ofs << "SavePath=" << SavePath << "\n";
	ofs << "WaitTime=" << WaitTime << "\n";
	ofs << "CaptureMethod=" << std::to_string(CaptureMethod) << "\n";
	ofs << "IncludeBorder=" << (IncludeBorder ? 1 : 0) << "\n";
	ofs << "HotkeyMod=" << std::to_string(HotkeyMod) << "\n";
	ofs << "HotkeyCode=" << std::to_string(HotkeyCode) << "\n";
	ofs << "ImageFormat=" << std::to_string(ImageFormat) << "\n";
	
	ofs.close();
	
}


// 設定ファイルをキーと値に切り分ける
std::map<std::string, std::string> Config::ParseConfFile(const std::string &conffile) {
	std::ifstream ifs(conffile);
	if( !ifs ) {
		throw std::exception("Failed to open conffile");
	}
	
	std::map<std::string, std::string> mapKeyValues;
	
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
	
	return mapKeyValues;
	
}

