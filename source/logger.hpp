#pragma once

#include <format>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

enum class LogLevel {
  Debug,
  Info,
  Warning,
  Error,
};

inline std::string logLevelToString(LogLevel level) {
#define ADD_LEVEL(X)                                                           \
  case LogLevel::X:                                                            \
    return #X
  switch (level) {
    ADD_LEVEL(Debug);
    ADD_LEVEL(Info);
    ADD_LEVEL(Warning);
    ADD_LEVEL(Error);
  }
#undef ADD_LEVEL
}

using LoggerWriteFn = std::function<void(const std::string &msg)>;

class Logger {
protected:
  virtual std::string name();

public:
  static void setWriteFunction(LoggerWriteFn writeFn);
  virtual std::string combineMessage(LogLevel level,
                                     const std::string &message);
  virtual void writeMessage(const std::string &logMessage);

  template <class... Args>
  void log(LogLevel level, std::string_view fmt, Args &&...args) {
    writeMessage(combineMessage(
        level, std::vformat(fmt, std::make_format_args(args...))));
  }
};
