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
	IMGFMT_BMP
};

/*
 * 撮影範囲
*/
enum CAPTURE_REGION {
	CAPTURE_REGION_ACTIVE_WINDOW,
	CAPTURE_REGION_SELECTED_REGION,
	CAPTURE_REGION_FULLSCREEN
};

/*
 * 設定クラス
*/
class Config {
public:
	
	Config() = default;
	~Config() = default;
	
	void Load(const std::wstring &conffile);
	void Save(const std::wstring &conffile);
	
	// 画像書き出しフォーマットの拡張子を取得
	std::wstring GetFormatExt() const {
		if( ImageFormat == IMGFMT_BMP ) return L"bmp";
		return L"";
	}
	
public:
	std::wstring SavePath;
	unsigned int WaitTime;
	CAPTURE_METHOD CaptureMethod;
	bool IncludeBorder;
	int HotkeyMod;
	int HotkeyCode;
	int HotkeyCodeRaw;
	IMAGE_FORMAT ImageFormat;
	CAPTURE_REGION RegionMode;
	int RegionX, RegionY;
	int RegionWidth, RegionHeight;
	int MaxCaptureCount;
	
private:
	// 設定ファイルをキーと値に切り分ける
	std::map<std::wstring, std::wstring> ParseConfFile(const std::wstring &conffile);
	
};

