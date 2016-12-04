#pragma once

#include <string>
#include <map>

enum TRIMMING_MODE;

class Setting {
public:
	
	Setting() = default;
	~Setting() = default;
	
	bool Load(const std::string &settingfile);
	
	const std::string &GetSavePath() const {
		return GetValueOrDefault("SavePath");
	}
	
	const std::string &GetSaveFormat() const {
		return GetValueOrDefault("SaveFormat", "ppm");
	}
	
	int GetWait() const {
		return std::stoi(GetValueOrDefault("Wait", "100"));
	}
	
	const std::string &GetCaptureMethod() const {
		return GetValueOrDefault("CaptureMethod", "bitblt");
	}
	
	TRIMMING_MODE GetTrimmingMode() const;
	
protected:
	
	// マップに値が無ければデフォルトの値を取得する
	const std::string &GetValueOrDefault(
		const std::string &key,
		const std::string &def = std::string()) const
	{
		auto it = mapKeyValues.find(key);
		if( it == mapKeyValues.end() ) {
			return def;
		} else {
			return it->second;
		}
	}
	
protected:
	std::map<std::string, std::string> mapKeyValues;
};
