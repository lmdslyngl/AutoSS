#pragma once

#include <string>
#include <map>

enum TRIMMING_MODE;

class Setting {
public:
	
	Setting() = default;
	~Setting() = default;
	
	bool Load(const std::wstring &settingfile);
	
	const std::wstring &GetSavePath() const {
		return GetValueOrDefault(L"SavePath");
	}
	
	const std::wstring &GetSaveFormat() const {
		return GetValueOrDefault(L"SaveFormat", L"ppm");
	}
	
	int GetWait() const {
		return std::stoi(GetValueOrDefault(L"Wait", L"100"));
	}
	
	const std::wstring &GetCaptureMethod() const {
		return GetValueOrDefault(L"CaptureMethod", L"bitblt");
	}
	
	TRIMMING_MODE GetTrimmingMode() const;
	
protected:
	
	// マップに値が無ければデフォルトの値を取得する
	const std::wstring &GetValueOrDefault(
		const std::wstring &key,
		const std::wstring &def = std::wstring()) const
	{
		auto it = mapKeyValues.find(key);
		if( it == mapKeyValues.end() ) {
			return def;
		} else {
			return it->second;
		}
	}
	
protected:
	std::map<std::wstring, std::wstring> mapKeyValues;
};
