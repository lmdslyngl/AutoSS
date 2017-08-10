#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <string.h>

// ログ書き出しクラス
class Logger {
public:
	
	// ファイルにログを出力しないLoggerを作成
	Logger();
	
	// 出力ファイルを指定してLoggerを作成
	Logger(const std::wstring &logfile);
	
	~Logger();
	
	// ログ書き出し
	// 先頭に現在時刻が自動的に追加される
	void Write(const wchar_t *log);
	
	// ログ書き出し
	// 先頭に現在時刻が自動的に追加される
	void Write(const std::wstring &log);
	
	//　フォーマットを指定してログ書き出し
	template <class ... Args> void Write(const wchar_t *format, Args ... args) {
		if( QuietFlag ) return;
		wchar_t buf[2048];
		swprintf_s(buf, format, args...);
		Write(buf);
	}
	
private:
	
	// 現在時刻を表す文字列を取得
	std::wstring GetDateStr();
	
private:
	std::ofstream ofs;
	std::mutex mtx;
	bool QuietFlag;
};


// グローバルLogger
class GlbLog {
public:
	
	// ファイルにログを出力しないLoggerを作成
	static void SetupGlobalQuietLogger() {
		pLogger = std::make_unique<Logger>();
	}
	
	// ログファイルを指定してLoggerを作成
	static void SetupGlobalLogger(const std::wstring &filename) {
		pLogger = std::make_unique<Logger>(filename);
	}
	
	static Logger &GetLogger() {
		return *pLogger;
	}
	
private:
	static std::unique_ptr<Logger> pLogger;
};

