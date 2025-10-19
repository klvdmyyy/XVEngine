#include "logger.hpp"

#include <iostream>
#include <memory>

static LoggerWriteFn gWriteFn = [](const std::string &msg) {
  std::cout << msg << std::endl;
};

std::string Logger::name() {
  std::string typeName = typeid(*this).name();
  if (typeName.front() == 'N' && typeName.back() == 'E') {
    typeName.erase(typeName.begin());
    typeName.erase(typeName.size() - 1);
  }

  std::string result;
  bool hasNums = false;
  for (int i = 0; i < typeName.size(); i++) {
    char c = typeName[i];
    if (std::isalpha(c)) {
      if (hasNums) {
        hasNums = false;
        result.append("::");
      }
      result.insert(result.end(), c);
    } else if (std::isalnum(c) && !result.empty()) {
      hasNums = true;
    }
  }
  return result;
}

std::string Logger::combineMessage(LogLevel level, const std::string &message) {
  return std::format("[{}][{}]: {}", logLevelToString(level), name(), message);
}

void Logger::writeMessage(const std::string &logMessage) {
  gWriteFn(logMessage);
}

void Logger::setWriteFunction(LoggerWriteFn writeFn) { gWriteFn = writeFn; }
