#pragma once

#include <string>
#include <map>

/*
 * キャプチャ方法
*/
enum CAPTURE_METHOD {
	CAPTURE_BITBLT,
	CAPTURE_DESKTOP_DUPL_API
};

/*
 * 保存形式
*/
enum IMAGE_FORMAT {
	IMGFMT_PPM,
	IMGFMT_BMP
};

/*
 * 設定クラス
*/
class Config {
public:
	
	Config() = default;
	~Config() = default;
	
	void Load(const std::string &conffile);
	void Save(const std::string &conffile);
	
	// 画像書き出しフォーマットの拡張子を取得
	std::string GetFormatExt() const {
		if( ImageFormat == IMGFMT_BMP ) return "bmp";
		else if( ImageFormat == IMGFMT_PPM ) return "ppm";
		return "";
	}
	
public:
	std::string SavePath;
	unsigned int WaitTime;
	CAPTURE_METHOD CaptureMethod;
	bool IncludeBorder;
	int HotkeyMod;
	int HotkeyCode;
	IMAGE_FORMAT ImageFormat;
	
private:
	// 設定ファイルをキーと値に切り分ける
	std::map<std::string, std::string> ParseConfFile(const std::string &conffile);
	
};

