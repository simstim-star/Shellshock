#pragma once

#include <string>
#include <vector>

namespace TGW {
enum class LogType { INFO, NUM_LOG_TYPES };

struct LogEntry {
	std::string message;
	LogType type;

	const char *GetTypeString() const
	{
		switch (type) {
		case LogType::INFO:
			return "[INFO] ";
		default:
			return "[UNKNOWN] ";
		}
	}

	inline const std::string AsString() const { return GetTypeString() + message; }
};

class Logger {
  public:
	inline static void LogInfo(const std::string &log) { Log(log, LogType::INFO); }
	inline static void Clear() { Get()._entries.clear(); }
	inline static const std::vector<LogEntry> &GetAll() { return Get()._entries; }

  private:
	inline static void Log(const std::string &log, LogType type) { Get()._entries.push_back({log, type}); }

	inline static Logger &Get()
	{
		static Logger instance;
		return instance;
	}
	std::vector<LogEntry> _entries;
};
} // namespace TGW