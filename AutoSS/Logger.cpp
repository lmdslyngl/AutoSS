
#include "Logger.h"
#include <time.h>
#include <sstream>
#include "Util.h"

// ログ書き出しクラス
Logger::Logger(const std::wstring &logfile) {
	ofs.open(logfile, std::ios::app | std::ios::binary);
}
Logger::~Logger() {
	ofs.close();
}

// メッセージ書き出し
void Logger::Write(const wchar_t *log) {
	std::lock_guard<std::mutex> mtxLock(mtx);
	
	std::wstringstream ss;
	ss << L"[" << GetDateStr() << L"] " << log << L"\n";
	std::string utf8line = ConvWStrToUtf8(ss.str());
	ofs.write(utf8line.c_str(), utf8line.size());
	
}

// メッセージ書き出し
void Logger::Write(const std::wstring &log) {
	Write(log.c_str());
}

// 現在時刻を表す文字列を取得
std::wstring Logger::GetDateStr() {
	time_t t = time(nullptr);
	tm timedata;
	localtime_s(&timedata, &t);
	
	std::wstringstream ss;
	ss << timedata.tm_year + 1900 << L"/"
		<< timedata.tm_mon + 1 << L"/"
		<< timedata.tm_mday << L" "
		<< timedata.tm_hour << L":"
		<< timedata.tm_min << L":"
		<< timedata.tm_sec;
	
	return ss.str();
	
}


// グローバルLogger
// logger実態
std::unique_ptr<Logger> GlbLog::pLogger = nullptr;

